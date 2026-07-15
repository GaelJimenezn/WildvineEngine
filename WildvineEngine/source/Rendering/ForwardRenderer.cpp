#include "Rendering/ForwardRenderer.h"
#include <algorithm>
#include <cmath>
#include "Device.h"
#include "DeviceContext.h"
#include "Rendering/Material.h"
#include "Rendering/MaterialInstance.h"
#include "Rendering/Mesh.h"
#include "SamplerState.h"
#include "EngineUtilities/Utilities/Camera.h"
#include "EngineUtilities/Utilities/EditorViewportPass.h"
#include "EngineUtilities/Utilities/LayoutBuilder.h"
#include "EngineUtilities/Utilities/Skybox.h"


// Inicializa todos los recursos necesarios del renderer (buffers, sombras, estados, etc.)
HRESULT
ForwardRenderer::init(Device& device) {
  HRESULT hr = m_perFrameBuffer.init(device, sizeof(CBPerFrame));
  if (FAILED(hr)) {
    return hr;
  }

  hr = m_perObjectBuffer.init(device, sizeof(CBPerObject));
  if (FAILED(hr)) {
    return hr;
  }

  hr = m_perMaterialBuffer.init(device, sizeof(CBPerMaterial));
  if (FAILED(hr)) {
    return hr;
  }

  hr = m_transparentDepthStencil.init(device,
    true,
    D3D11_DEPTH_WRITE_MASK_ZERO,
    D3D11_COMPARISON_LESS_EQUAL);
  if (FAILED(hr)) {
    return hr;
  }

  hr = createShadowResources(device);
  if (FAILED(hr)) {
    return hr;
  }

  hr = m_preShadowDebugPass.init(device, 1280, 720);
  if (FAILED(hr)) {
    return hr;
  }

  hr = createBlendStates(device);
  if (FAILED(hr)) {
    return hr;
  }
  return S_OK;
}


// Ajusta el tamaño del viewport interno (principalmente usado para debug pre-shadow)
void
ForwardRenderer::resize(Device& device, unsigned int width, unsigned int height) {
  m_preShadowDebugPass.resize(device, width, height);
}


// Actualiza el constant buffer por frame con datos de cámara y luz
void
ForwardRenderer::updatePerFrame(const Camera& camera,
  const RenderScene& scene,
  DeviceContext& deviceContext) {
  updateLightMatrices(camera, scene);
  XMStoreFloat4x4(&m_cbPerFrame.View, XMMatrixTranspose(camera.getView()));
  XMStoreFloat4x4(&m_cbPerFrame.Projection, XMMatrixTranspose(camera.getProj()));
  m_cbPerFrame.CameraPos = camera.getPosition();
  m_cbPerFrame.LightDir = EU::Vector3(0.0f, 0.0f, -1.0f);
  m_cbPerFrame.LightColor = EU::Vector3(1.0f, 1.0f, 1.0f);

  if (!scene.directionalLights.empty()) {
    const LightData& mainLight = scene.directionalLights.front();
    m_cbPerFrame.LightDir = mainLight.direction;
    m_cbPerFrame.LightColor = mainLight.color * mainLight.intensity;
  }

  m_perFrameBuffer.update(deviceContext, nullptr, 0, nullptr, &m_cbPerFrame, 0, 0);
}


// Ejecuta todo el pipeline de render: colas, sombras, passes principales
void
ForwardRenderer::render(DeviceContext& deviceContext,
  const Camera& camera,
  RenderScene& scene,
  EditorViewportPass& viewportPass) {
  const float viewportClear[4] = { 0.10f, 0.10f, 0.10f, 1.0f };

  buildQueues(scene, camera);
  updatePerFrame(camera, scene, deviceContext);
  renderPreShadowDebugPass(deviceContext, scene);
  renderShadowPass(deviceContext);
  viewportPass.begin(deviceContext, viewportClear);
  viewportPass.setViewport(deviceContext);
  viewportPass.clearDepth(deviceContext);
  renderSkyboxPass(deviceContext, scene);
  renderOpaquePass(deviceContext);
  renderTransparentPass(deviceContext);

}

void
ForwardRenderer::destroy() {
  m_opaqueQueue.clear();
  m_transparentQueue.clear();
  SAFE_RELEASE(m_alphaBlendState);
  SAFE_RELEASE(m_opaqueBlendState);
  SAFE_RELEASE(m_additiveBlendState);
  SAFE_RELEASE(m_premultipliedBlendState);
  m_transparentDepthStencil.destroy();
  m_perMaterialBuffer.destroy();
  m_perObjectBuffer.destroy();
  m_perFrameBuffer.destroy();
  m_shadowRasterizer.destroy();
  m_shadowShader.destroy();
  m_shadowDSV.destroy();
  m_shadowDepthSRV.destroy();
  m_shadowDepthTexture.destroy();
  m_preShadowDebugPass.destroy();
}

// Renderiza todos los objetos opacos usando el shadow map si está disponible
void
ForwardRenderer::renderOpaquePass(DeviceContext& deviceContext) {
  m_perFrameBuffer.render(deviceContext, 0, 1, true);
  if (m_applyShadows && m_shadowDepthSRV.m_textureFromImg) {
    deviceContext.PSSetShaderResources(6, 1, &m_shadowDepthSRV.m_textureFromImg);
  }
  else {
    ID3D11ShaderResourceView* nullShadowSRV[1] = { nullptr };
    deviceContext.PSSetShaderResources(6, 1, nullShadowSRV);
  }
  deviceContext.OMSetBlendState(m_opaqueBlendState, m_blendFactor, 0xffffffff);

  for (const RenderObject* object : m_opaqueQueue) {
    if (!object) {
      continue;
    }
    renderObject(deviceContext, *object, RenderPassType::Opaque);
  }
}


// Renderiza objetos transparentes con blending adecuado y ordenados por distancia
void
ForwardRenderer::renderTransparentPass(DeviceContext& deviceContext) {
  m_perFrameBuffer.render(deviceContext, 0, 1, true);
  if (m_applyShadows && m_shadowDepthSRV.m_textureFromImg) {
    deviceContext.PSSetShaderResources(6, 1, &m_shadowDepthSRV.m_textureFromImg);
  }
  else {
    ID3D11ShaderResourceView* nullShadowSRV[1] = { nullptr };
    deviceContext.PSSetShaderResources(6, 1, nullShadowSRV);
  }


  for (const RenderObject* object : m_transparentQueue) {
    if (!object) {
      continue;
    }
    Material* material = object->materialInstance ? object->materialInstance->getMaterial() : nullptr;
    deviceContext.OMSetBlendState(resolveBlendState(material), m_blendFactor, 0xffffffff);
    renderObject(deviceContext, *object, RenderPassType::Transparent);
  }
  deviceContext.OMSetBlendState(m_opaqueBlendState, m_blendFactor, 0xffffffff);
}


// Genera el shadow map renderizando la escena desde la perspectiva de la luz
void
ForwardRenderer::renderShadowPass(DeviceContext& deviceContext) {
  if (!m_shadowDSV.m_depthStencilView || !m_shadowShader.m_VertexShader) {
    return;
  }
  ID3D11ShaderResourceView* nullShadowSRV[1] = { nullptr };
  deviceContext.PSSetShaderResources(6, 1, nullShadowSRV);
  deviceContext.OMSetRenderTargets(0, nullptr, m_shadowDSV.m_depthStencilView);
  deviceContext.ClearDepthStencilView(m_shadowDSV.m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

  D3D11_VIEWPORT shadowViewport{};
  shadowViewport.TopLeftX = 0.0f;
  shadowViewport.TopLeftY = 0.0f;
  shadowViewport.Width = static_cast<float>(m_shadowMapSize);
  shadowViewport.Height = static_cast<float>(m_shadowMapSize);
  shadowViewport.MinDepth = 0.0f;
  shadowViewport.MaxDepth = 1.0f;
  deviceContext.RSSetViewports(1, &shadowViewport);

  m_shadowRasterizer.render(deviceContext);
  m_perFrameBuffer.render(deviceContext, 0, 1, false);

  for (const RenderObject* object : m_opaqueQueue) {
    if (!object || !object->castShadow) {
      continue;
    }
    renderShadowObject(deviceContext, *object);
  }
}

void
ForwardRenderer::renderPreShadowDebugPass(DeviceContext& deviceContext, RenderScene& scene) {
  if (!m_preShadowDebugPass.isValid()) {
    return;
  }

  const float clearColor[4] = { 0.10f, 0.10f, 0.10f, 1.0f };
  ID3D11ShaderResourceView* nullShadowSRV[1] = { nullptr };
  deviceContext.PSSetShaderResources(6, 1, nullShadowSRV);
  m_applyShadows = false;

  m_preShadowDebugPass.begin(deviceContext, clearColor);
  m_preShadowDebugPass.setViewport(deviceContext);
  m_preShadowDebugPass.clearDepth(deviceContext);
  renderSkyboxPass(deviceContext, scene);
  renderOpaquePass(deviceContext);
  renderTransparentPass(deviceContext);

  m_applyShadows = true;
}


// Construye y ordena las colas de render (opacos y transparentes)
void
ForwardRenderer::buildQueues(RenderScene& scene, const Camera& camera) {
  (void)camera;
  m_opaqueQueue.clear();
  m_transparentQueue.clear();

  for (auto& object : scene.opaqueObjects) {
    m_opaqueQueue.push_back(&object);
  }

  for (auto& object : scene.transparentObjects) {
    m_transparentQueue.push_back(&object);
  }

  std::sort(m_opaqueQueue.begin(), m_opaqueQueue.end(),
    [](const RenderObject* lhs, const RenderObject* rhs) {
      if (lhs->materialInstance != rhs->materialInstance) {
        return lhs->materialInstance < rhs->materialInstance;
      }
      return lhs->distanceToCamera < rhs->distanceToCamera;
    });



  std::sort(m_transparentQueue.begin(), m_transparentQueue.end(),
    [](const RenderObject* lhs, const RenderObject* rhs) {
      return lhs->distanceToCamera > rhs->distanceToCamera;
    });
}


// Renderiza el skybox si existe en la escena
void
ForwardRenderer::renderSkyboxPass(DeviceContext& deviceContext, RenderScene& scene) {
  if (!scene.skybox) {
    return;
  }
  scene.skybox->render(deviceContext);
}

void
ForwardRenderer::renderObject(DeviceContext& deviceContext,
  const RenderObject& object,
  RenderPassType passType) {
  if (!object.mesh || (!object.materialInstance && object.materialInstances.empty())) {
    return;
  }

  deviceContext.IASetPrimitiveTopology(object.topology);

  std::vector<Submesh>& submeshes = object.mesh->getSubmeshes();
  for (Submesh& submesh : submeshes) {
    MaterialInstance* materialInstance = object.materialInstance;
    if (submesh.materialSlot < object.materialInstances.size() &&
      object.materialInstances[submesh.materialSlot]) {
      materialInstance = object.materialInstances[submesh.materialSlot];
    }

    if (!materialInstance) {
      continue;
    }

    Material* material = materialInstance->getMaterial();
    if (!material) {
      continue;
    }

    if (material->getRasterizerState()) {
      material->getRasterizerState()->render(deviceContext);
    }

    if (passType == RenderPassType::Transparent) {
      m_transparentDepthStencil.render(deviceContext, 0, false);
    }
    else if (material->getDepthStencilState()) {
      material->getDepthStencilState()->render(deviceContext, 0, false);
    }

    if (material->getShader()) {
      material->getShader()->render(deviceContext);
    }

    if (material->getSamplerState()) {
      material->getSamplerState()->render(deviceContext, 0, 1);
    }

    XMMATRIX submeshWorld = XMLoadFloat4x4(&submesh.localTransform) * object.world;
    XMStoreFloat4x4(&m_cbPerObject.World, XMMatrixTranspose(submeshWorld));
    m_perObjectBuffer.update(deviceContext, nullptr, 0, nullptr, &m_cbPerObject, 0, 0);
    m_perObjectBuffer.render(deviceContext, 1, 1, true);

    materialInstance->bindTextures(deviceContext);

    const MaterialParams& params = materialInstance->getParams();
    m_cbPerMaterial.BaseColor = params.baseColor;
    m_cbPerMaterial.Metallic = params.metallic;
    m_cbPerMaterial.Roughness = params.roughness;
    m_cbPerMaterial.AO = params.ao;
    m_cbPerMaterial.NormalScale = params.normalScale;
    m_cbPerMaterial.EmissiveStrength = params.emissiveStrength;
    m_cbPerMaterial.AlphaCutoff = 0.0f;
    if (material->getDomain() == MaterialDomain::Masked) {
      m_cbPerMaterial.AlphaCutoff = params.alphaCutoff;
    }
    m_perMaterialBuffer.update(deviceContext, nullptr, 0, nullptr, &m_cbPerMaterial, 0, 0);
    m_perMaterialBuffer.render(deviceContext, 2, 1, true);

    submesh.vertexBuffer.render(deviceContext, 0, 1);
    submesh.indexBuffer.render(deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);
    deviceContext.DrawIndexed(submesh.indexCount, submesh.startIndex, 0);
  }
}

void
ForwardRenderer::renderShadowObject(DeviceContext& deviceContext, const RenderObject& object) {
  if (!object.mesh) {
    return;
  }

  m_shadowShader.render(deviceContext);
  deviceContext.m_deviceContext->PSSetShader(nullptr, nullptr, 0);
  deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  std::vector<Submesh>& submeshes = object.mesh->getSubmeshes();
  for (Submesh& submesh : submeshes) {
    XMMATRIX submeshWorld = XMLoadFloat4x4(&submesh.localTransform) * object.world;
    XMStoreFloat4x4(&m_cbPerObject.World, XMMatrixTranspose(submeshWorld));
    m_perObjectBuffer.update(deviceContext, nullptr, 0, nullptr, &m_cbPerObject, 0, 0);
    m_perObjectBuffer.render(deviceContext, 1, 1, false);

    submesh.vertexBuffer.render(deviceContext, 0, 1);
    submesh.indexBuffer.render(deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);
    deviceContext.DrawIndexed(submesh.indexCount, submesh.startIndex, 0);
  }
}

// Calcula las matrices de vista y proyección desde la perspectiva de la luz
void
ForwardRenderer::updateLightMatrices(const Camera& camera, const RenderScene& scene) {
  EU::Vector3 lightDir = EU::Vector3(0.0f, 0.0f, -1.0f);
  if (!scene.directionalLights.empty()) {
    lightDir = scene.directionalLights.front().direction;
  }

  XMVECTOR lightDirVec = XMVector3Normalize(XMVectorSet(lightDir.x, lightDir.y, lightDir.z, 0.0f));
  XMVECTOR cameraPos = XMVectorSet(camera.getPosition().x, camera.getPosition().y, camera.getPosition().z, 1.0f);
  XMVECTOR lightTarget = cameraPos;
  XMVECTOR lightEye = XMVectorSubtract(lightTarget, XMVectorScale(lightDirVec, 35.0f));
  XMVECTOR worldUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
  if (fabsf(XMVectorGetX(XMVector3Dot(lightDirVec, worldUp))) > 0.98f) {
    worldUp = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
  }

  XMMATRIX lightView = XMMatrixLookAtLH(lightEye, lightTarget, worldUp);
  XMMATRIX lightProjection = XMMatrixOrthographicLH(40.0f, 40.0f, 1.0f, 80.0f);
  XMStoreFloat4x4(&m_cbPerFrame.LightViewProjection, XMMatrixTranspose(lightView * lightProjection));
}


// Crea todos los recursos necesarios para el sistema de sombras (texturas, shaders, rasterizer)
HRESULT
ForwardRenderer::createShadowResources(Device& device) {
  HRESULT hr = m_shadowDepthTexture.init(
    device,
    m_shadowMapSize,
    m_shadowMapSize,
    DXGI_FORMAT_R24G8_TYPELESS,
    D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
  if (FAILED(hr)) {
    return hr;
  }

  hr = m_shadowDepthSRV.init(device, m_shadowDepthTexture, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
  if (FAILED(hr)) {
    return hr;
  }

  hr = m_shadowDSV.init(device, m_shadowDepthTexture, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_DSV_DIMENSION_TEXTURE2D);
  if (FAILED(hr)) {
    return hr;

  }

  LayoutBuilder builder;
  builder.Add("POSITION", DXGI_FORMAT_R32G32B32_FLOAT)
          .Add("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT)
          .Add("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
          .Add("BITANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
          .Add("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);



  hr = m_shadowShader.init(device, "ShadowMap.hlsl", builder);
  if (FAILED(hr)) {
    return hr;
  }

  hr = m_shadowRasterizer.init(device, D3D11_FILL_SOLID, D3D11_CULL_BACK, false, true);
  if (FAILED(hr)) {
    return hr;
  }
  return S_OK;
}


// Crea los distintos estados de blending utilizados en el renderer
HRESULT
ForwardRenderer::createBlendStates(Device& device) {
  if (!device.m_device) {
    return E_POINTER;
  }

  D3D11_BLEND_DESC blendDesc{};
  blendDesc.AlphaToCoverageEnable = FALSE;
  blendDesc.IndependentBlendEnable = FALSE;

  D3D11_RENDER_TARGET_BLEND_DESC& renderTarget = blendDesc.RenderTarget[0];
  renderTarget.BlendEnable = FALSE;
  renderTarget.SrcBlend = D3D11_BLEND_ONE;
  renderTarget.DestBlend = D3D11_BLEND_ZERO;
  renderTarget.BlendOp = D3D11_BLEND_OP_ADD;
  renderTarget.SrcBlendAlpha = D3D11_BLEND_ONE;
  renderTarget.DestBlendAlpha = D3D11_BLEND_ZERO;
  renderTarget.BlendOpAlpha = D3D11_BLEND_OP_ADD;
  renderTarget.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

  HRESULT hr = device.m_device->CreateBlendState(&blendDesc, &m_opaqueBlendState);
  if (FAILED(hr)) {
    return hr;
  }

  renderTarget.BlendEnable = TRUE;
  renderTarget.SrcBlend = D3D11_BLEND_SRC_ALPHA;
  renderTarget.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  renderTarget.BlendOp = D3D11_BLEND_OP_ADD;
  renderTarget.SrcBlendAlpha = D3D11_BLEND_ONE;
  renderTarget.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
  renderTarget.BlendOpAlpha = D3D11_BLEND_OP_ADD;

  hr = device.m_device->CreateBlendState(&blendDesc, &m_alphaBlendState);
  if (FAILED(hr)) {
    return hr;
  }

  renderTarget.BlendEnable = TRUE;
  renderTarget.SrcBlend = D3D11_BLEND_SRC_ALPHA;
  renderTarget.DestBlend = D3D11_BLEND_ONE;
  renderTarget.BlendOp = D3D11_BLEND_OP_ADD;
  renderTarget.SrcBlendAlpha = D3D11_BLEND_ONE;
  renderTarget.DestBlendAlpha = D3D11_BLEND_ONE;
  renderTarget.BlendOpAlpha = D3D11_BLEND_OP_ADD;


  hr = device.m_device->CreateBlendState(&blendDesc, &m_additiveBlendState);
  if (FAILED(hr)) {
    return hr;
  }

  renderTarget.BlendEnable = TRUE;
  renderTarget.SrcBlend = D3D11_BLEND_ONE;
  renderTarget.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  renderTarget.BlendOp = D3D11_BLEND_OP_ADD;
  renderTarget.SrcBlendAlpha = D3D11_BLEND_ONE;
  renderTarget.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
  renderTarget.BlendOpAlpha = D3D11_BLEND_OP_ADD;

  return device.m_device->CreateBlendState(&blendDesc, &m_premultipliedBlendState);
}


// Determina qué estado de blending usar según el material
ID3D11BlendState*
ForwardRenderer::resolveBlendState(const Material* material) const {
  if (!material) {
    return m_opaqueBlendState;
  }

  if (material->getDomain() != MaterialDomain::Transparent) {
    return m_opaqueBlendState;
  }

  switch (material->getBlendMode()) {
  case BlendMode::Additive:
    return m_additiveBlendState ? m_additiveBlendState : m_alphaBlendState;
  case BlendMode::PremultipliedAlpha:
    return m_premultipliedBlendState ? m_premultipliedBlendState : m_alphaBlendState;
  case BlendMode::Alpha:
  case BlendMode::Opaque:
  default:
    return m_alphaBlendState ? m_alphaBlendState : m_opaqueBlendState;
  }
}