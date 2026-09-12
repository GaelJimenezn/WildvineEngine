#include "ECS/Actor.h"
#include "MeshComponent.h"
#include "Device.h"
#include "DeviceContext.h"

Actor::Actor(Device &device) {
  // Setup Default Components
  EU::TSharedPointer<Transform> transform = EU::MakeShared<Transform>();
  addComponent(transform);
  EU::TSharedPointer<MeshComponent> meshComponent = EU::MakeShared<MeshComponent>();
  addComponent(meshComponent);

  HRESULT hr;
  std::string classNameType = "Actor -> " + m_name;
  hr = m_modelBuffer.init(device, sizeof(CBChangesEveryFrame));
  if (FAILED(hr)) {
    ERROR("Actor", classNameType.c_str(), "Failed to create new CBChangesEveryFrame");
  }

  // Awake
  awake();

  hr = m_sampler.init(device);
  if (FAILED(hr)) {
    ERROR("Actor", classNameType.c_str(), "Failed to create new SamplerState");
  }
}

void
Actor::update(float deltaTime, DeviceContext &deviceContext) {
  // Update all components
  for (auto &component : m_components) {
    if (component) {
      component->update(deltaTime);
    }
  }

  // Update the model buffer
  m_model.mWorld = XMMatrixTranspose(getComponent<Transform>()->matrix);
  m_model.vMeshColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
  // Update the constant buffer
  m_modelBuffer.update(deviceContext, nullptr, 0, nullptr, &m_model, 0, 0);
}

void
Actor::render(DeviceContext &deviceContext) {

  m_sampler.render(deviceContext, 0, 1);

  deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  // Update buffer and render all components
  for (unsigned int i = 0; i < m_meshes.size(); i++) {
    m_vertexBuffers[i].render(deviceContext, 0, 1);
    m_indexBuffers[i].render(deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);
    m_modelBuffer.render(deviceContext, 1, 1, true);

    // Limpieza por mesh (evita herencias)
    ID3D11ShaderResourceView *nullSRV[1] = {nullptr};
    deviceContext.m_deviceContext->PSSetShaderResources(0, 1, nullSRV);

    // Bind correcto por mesh
    if (i < m_textures.size()) {
      for (int k = 0; k < m_textures.size(); k++) {
        m_textures[k].render(deviceContext, k, 1);
      }
    }
    // else: se queda null
    deviceContext.DrawIndexed(m_meshes[i].m_numIndex, 0, 0);
  }
}

void
Actor::renderForSkybox(DeviceContext &deviceContext) {
  deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  // Update buffer and render all components
  for (unsigned int i = 0; i < m_meshes.size(); i++) {
    m_vertexBuffers[i].render(deviceContext, 0, 1);
    m_indexBuffers[i].render(deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);

    deviceContext.DrawIndexed(m_meshes[i].m_numIndex, 0, 0);
  }
}

void
Actor::destroy() {
  for (auto &vertexBuffer : m_vertexBuffers) {
    vertexBuffer.destroy();
  }

  for (auto &indexBuffer : m_indexBuffers) {
    indexBuffer.destroy();
  }

  for (auto &tex : m_textures) {
    tex.destroy();
  }
  m_modelBuffer.destroy();

  // m_rasterizer.destroy();
  // m_blendstate.destroy();
  m_sampler.destroy();
}

void
Actor::setMesh(Device &device, std::vector<MeshComponent> meshes) {
  m_meshes = meshes;
  HRESULT hr;
  for (auto &mesh : m_meshes) {
    // Crear vertex buffer
    Buffer vertexBuffer;
    hr = vertexBuffer.init(device, mesh, D3D11_BIND_VERTEX_BUFFER);
    if (FAILED(hr)) {
      ERROR("Actor", "setMesh", "Failed to create new vertexBuffer");
    } else {
      m_vertexBuffers.push_back(vertexBuffer);
    }

    // Crear index buffer
    Buffer indexBuffer;
    hr = indexBuffer.init(device, mesh, D3D11_BIND_INDEX_BUFFER);
    if (FAILED(hr)) {
      ERROR("Actor", "setMesh", "Failed to create new indexBuffer");
    } else {
      m_indexBuffers.push_back(indexBuffer);
    }
  }
}