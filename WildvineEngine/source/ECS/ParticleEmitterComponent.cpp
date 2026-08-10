#include "ECS/ParticleEmitterComponent.h"
#include "Device.h"
#include "DeviceContext.h"
#include "EngineUtilities/Utilities/Camera.h"
#include "EngineUtilities/Utilities/EditorViewportPass.h"
#include <algorithm>
#include <cmath>

ParticleEmitterComponent::ParticleEmitterComponent(Transform *transform)
    : Component(ComponentType::NONE)
    , m_transform(transform) {
  m_particles.reserve(MaxParticles);
}

HRESULT
ParticleEmitterComponent::compileShaders(Device &device) {
  ID3DBlob *vsBlob = nullptr;
  ID3DBlob *psBlob = nullptr;
  ID3DBlob *errors = nullptr;
  UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
  flags |= D3DCOMPILE_DEBUG;
#endif
  HRESULT hr = D3DX11CompileFromFile("Particle.hlsl",
                                     nullptr,
                                     nullptr,
                                     "VS",
                                     "vs_5_0",
                                     flags,
                                     0,
                                     nullptr,
                                     &vsBlob,
                                     &errors,
                                     nullptr);
  if (errors) {
    OutputDebugStringA((const char *)errors->GetBufferPointer());
    errors->Release();
    errors = nullptr;
  }
  if (FAILED(hr))
    return hr;
  hr = D3DX11CompileFromFile("Particle.hlsl",
                             nullptr,
                             nullptr,
                             "PS",
                             "ps_5_0",
                             flags,
                             0,
                             nullptr,
                             &psBlob,
                             &errors,
                             nullptr);
  if (errors) {
    OutputDebugStringA((const char *)errors->GetBufferPointer());
    errors->Release();
  }
  if (FAILED(hr)) {
    vsBlob->Release();
    return hr;
  }
  hr = device.m_device->CreateVertexShader(
      vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);
  if (SUCCEEDED(hr))
    hr = device.m_device->CreatePixelShader(
        psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);
  D3D11_INPUT_ELEMENT_DESC elements[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"COLOR",
       0,
       DXGI_FORMAT_R32G32B32A32_FLOAT,
       0,
       12,
       D3D11_INPUT_PER_VERTEX_DATA,
       0}};
  if (SUCCEEDED(hr))
    hr = device.m_device->CreateInputLayout(
        elements, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);
  vsBlob->Release();
  psBlob->Release();
  return hr;
}

HRESULT
ParticleEmitterComponent::init(Device &device) {
  destroy();
  HRESULT hr = compileShaders(device);
  if (FAILED(hr))
    return hr;
  D3D11_BUFFER_DESC vb{};
  vb.ByteWidth = sizeof(Vertex) * MaxParticles * 6;
  vb.Usage = D3D11_USAGE_DYNAMIC;
  vb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  vb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  hr = device.m_device->CreateBuffer(&vb, nullptr, &m_vertexBuffer);
  if (FAILED(hr))
    return hr;
  D3D11_BUFFER_DESC cb{};
  cb.ByteWidth = sizeof(Constants);
  cb.Usage = D3D11_USAGE_DYNAMIC;
  cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  hr = device.m_device->CreateBuffer(&cb, nullptr, &m_constantBuffer);
  if (FAILED(hr))
    return hr;
  D3D11_BLEND_DESC blend{};
  blend.RenderTarget[0].BlendEnable = TRUE;
  blend.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
  blend.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
  blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
  blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
  blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
  blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
  hr = device.m_device->CreateBlendState(&blend, &m_additiveBlend);
  if (FAILED(hr))
    return hr;
  D3D11_DEPTH_STENCIL_DESC depth{};
  // Editor particles are overlays: the grid must not hide a newly created emitter.
  depth.DepthEnable = FALSE;
  depth.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
  depth.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
  hr = device.m_device->CreateDepthStencilState(&depth, &m_depthRead);
  if (FAILED(hr))
    return hr;
  D3D11_RASTERIZER_DESC rasterizer{};
  rasterizer.FillMode = D3D11_FILL_SOLID;
  rasterizer.CullMode = D3D11_CULL_NONE;
  rasterizer.DepthClipEnable = TRUE;
  hr = device.m_device->CreateRasterizerState(&rasterizer, &m_rasterizer);
  m_ready = SUCCEEDED(hr);
  return hr;
}

float
ParticleEmitterComponent::random01() {
  m_randomState = 1664525u * m_randomState + 1013904223u;
  return float(m_randomState & 0x00FFFFFFu) / float(0x01000000u);
}

void
ParticleEmitterComponent::spawnParticle() {
  if (m_particles.size() >= MaxParticles)
    return;
  Particle p{};
  p.lifetime = (std::max)(0.1f, m_lifetime);
  p.position = EU::Vector3((random01() - 0.5f) * 0.2f, (random01() - 0.5f) * 0.2f, 0.0f);
  p.velocity = EU::Vector3((random01() - 0.5f) * 0.35f,
                           (random01() - 0.5f) * 0.35f,
                           0.65f + random01() * 0.65f);
  m_particles.push_back(p);
}

void
ParticleEmitterComponent::update(float dt) {
  if (!m_enabled)
    return;
  m_spawnAccumulator += dt * (std::max)(0.0f, m_emissionRate);
  while (m_spawnAccumulator >= 1.0f) {
    spawnParticle();
    m_spawnAccumulator -= 1.0f;
  }
  for (Particle &p : m_particles) {
    p.age += dt;
    p.velocity.z += 0.15f * dt;
    p.position.x += p.velocity.x * dt;
    p.position.y += p.velocity.y * dt;
    p.position.z += p.velocity.z * dt;
  }
  m_particles.erase(std::remove_if(m_particles.begin(),
                                   m_particles.end(),
                                   [](const Particle &p) { return p.age >= p.lifetime; }),
                    m_particles.end());
}

void
ParticleEmitterComponent::renderParticles(DeviceContext &context,
                                          const Camera &camera,
                                          EditorViewportPass &viewport) {
  if (!m_ready || !m_enabled || m_particles.empty() || !m_transform)
    return;
  std::vector<Vertex> vertices;
  vertices.reserve(m_particles.size() * 6);
  EU::Vector3 right = camera.GetRight(), up = camera.GetUp(),
              origin = m_transform->getPosition();
  const int corners[6][2] = {{-1, -1}, {-1, 1}, {1, 1}, {-1, -1}, {1, 1}, {1, -1}};
  for (const Particle &p : m_particles) {
    float t = (std::min)(1.0f, p.age / p.lifetime),
          size = m_startSize + (m_endSize - m_startSize) * t;
    float r = m_startColor.x + (m_endColor.x - m_startColor.x) * t,
          g = m_startColor.y + (m_endColor.y - m_startColor.y) * t;
    float b = m_startColor.z + (m_endColor.z - m_startColor.z) * t, a = 1.0f - t;
    for (const auto &c : corners) {
      Vertex v{};
      v.position[0] = origin.x + p.position.x + (right.x * c[0] + up.x * c[1]) * size;
      v.position[1] = origin.y + p.position.y + (right.y * c[0] + up.y * c[1]) * size;
      v.position[2] = origin.z + p.position.z + (right.z * c[0] + up.z * c[1]) * size;
      v.color[0] = r;
      v.color[1] = g;
      v.color[2] = b;
      v.color[3] = a;
      vertices.push_back(v);
    }
  }
  D3D11_MAPPED_SUBRESOURCE mapped{};
  if (FAILED(context.m_deviceContext->Map(
          m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    return;
  memcpy(mapped.pData, vertices.data(), vertices.size() * sizeof(Vertex));
  context.m_deviceContext->Unmap(m_vertexBuffer, 0);
  Constants constants{XMMatrixTranspose(camera.getView() * camera.getProj())};
  if (SUCCEEDED(context.m_deviceContext->Map(
          m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
    memcpy(mapped.pData, &constants, sizeof(constants));
    context.m_deviceContext->Unmap(m_constantBuffer, 0);
  }
  viewport.bind(context);
  viewport.setViewport(context);
  UINT stride = sizeof(Vertex), offset = 0;
  float blendFactor[4] = {0, 0, 0, 0};
  context.m_deviceContext->IASetInputLayout(m_inputLayout);
  context.m_deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
  context.m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context.m_deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
  context.m_deviceContext->VSSetConstantBuffers(0, 1, &m_constantBuffer);
  context.m_deviceContext->PSSetShader(m_pixelShader, nullptr, 0);
  context.m_deviceContext->OMSetBlendState(m_additiveBlend, blendFactor, 0xffffffff);
  context.m_deviceContext->RSSetState(m_rasterizer);
  context.m_deviceContext->OMSetDepthStencilState(m_depthRead, 0);
  context.m_deviceContext->Draw((UINT)vertices.size(), 0);
  ++context.m_drawCallCount;
  context.m_deviceContext->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
  context.m_deviceContext->OMSetDepthStencilState(nullptr, 0);
}

void
ParticleEmitterComponent::destroy() {
  SAFE_RELEASE(m_rasterizer);
  SAFE_RELEASE(m_depthRead);
  SAFE_RELEASE(m_additiveBlend);
  SAFE_RELEASE(m_inputLayout);
  SAFE_RELEASE(m_pixelShader);
  SAFE_RELEASE(m_vertexShader);
  SAFE_RELEASE(m_constantBuffer);
  SAFE_RELEASE(m_vertexBuffer);
  m_particles.clear();
  m_ready = false;
}
