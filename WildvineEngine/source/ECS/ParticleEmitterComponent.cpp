#include "ECS/ParticleEmitterComponent.h"
#include "Device.h"
#include "DeviceContext.h"
#include "EngineUtilities/Utilities/Camera.h"
#include "EngineUtilities/Utilities/EditorViewportPass.h"
#include <algorithm>
#include <cmath>

namespace {

constexpr float kPi = 3.14159265358979323846f;

EU::Vector3
normalizeOr(const EU::Vector3 &value, const EU::Vector3 &fallback) {
  const float lengthSquared = value.x * value.x + value.y * value.y + value.z * value.z;
  if (lengthSquared <= 0.000001f)
    return fallback;
  const float inverseLength = 1.0f / std::sqrt(lengthSquared);
  return EU::Vector3(
      value.x * inverseLength, value.y * inverseLength, value.z * inverseLength);
}

EU::Vector3
cross(const EU::Vector3 &left, const EU::Vector3 &right) {
  return EU::Vector3(left.y * right.z - left.z * right.y,
                     left.z * right.x - left.x * right.z,
                     left.x * right.y - left.y * right.x);
}

} // namespace

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
  blend.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
  hr = device.m_device->CreateBlendState(&blend, &m_alphaBlend);
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
ParticleEmitterComponent::sampleEmitter(EU::Vector3 &position, EU::Vector3 &direction) {
  const EU::Vector3 up = normalizeOr(m_settings.direction, EU::Vector3(0.0f, 0.0f, 1.0f));
  position = EU::Vector3(0.0f, 0.0f, 0.0f);
  direction = up;

  if (m_settings.shape == ParticleEmitterShape::Box) {
    position = EU::Vector3((random01() * 2.0f - 1.0f) * m_settings.boxExtents.x,
                           (random01() * 2.0f - 1.0f) * m_settings.boxExtents.y,
                           (random01() * 2.0f - 1.0f) * m_settings.boxExtents.z);
  } else if (m_settings.shape == ParticleEmitterShape::Sphere) {
    EU::Vector3 radial;
    do {
      radial = EU::Vector3(
          random01() * 2.0f - 1.0f, random01() * 2.0f - 1.0f, random01() * 2.0f - 1.0f);
    } while (radial.x * radial.x + radial.y * radial.y + radial.z * radial.z > 1.0f);
    direction = normalizeOr(radial, up);
    const float radius = (std::max)(0.0f, m_settings.sphereRadius);
    const float distance = radius * std::cbrt(random01());
    position = EU::Vector3(
        direction.x * distance, direction.y * distance, direction.z * distance);
  }

  if (m_settings.shape == ParticleEmitterShape::Cone) {
    const EU::Vector3 reference = std::fabs(up.z) < 0.99f ? EU::Vector3(0.0f, 0.0f, 1.0f)
                                                          : EU::Vector3(0.0f, 1.0f, 0.0f);
    const EU::Vector3 tangent =
        normalizeOr(cross(reference, up), EU::Vector3(1.0f, 0.0f, 0.0f));
    const EU::Vector3 bitangent = cross(up, tangent);
    const float angle = random01() * 2.0f * kPi;
    const float coneRadius =
        std::tan((std::max)(0.0f, m_settings.coneAngleDegrees) * kPi / 180.0f);
    const float offset = std::sqrt(random01()) * coneRadius;
    direction = normalizeOr(
        EU::Vector3(
            up.x + offset * (std::cos(angle) * tangent.x + std::sin(angle) * bitangent.x),
            up.y + offset * (std::cos(angle) * tangent.y + std::sin(angle) * bitangent.y),
            up.z +
                offset * (std::cos(angle) * tangent.z + std::sin(angle) * bitangent.z)),
        up);
  } else if (m_settings.shape != ParticleEmitterShape::Sphere) {
    const float spread = (std::max)(0.0f, m_settings.spread);
    direction = normalizeOr(EU::Vector3(up.x + (random01() * 2.0f - 1.0f) * spread,
                                        up.y + (random01() * 2.0f - 1.0f) * spread,
                                        up.z + (random01() * 2.0f - 1.0f) * spread),
                            up);
  }
}

void
ParticleEmitterComponent::spawnParticle() {
  if (m_particles.size() >= MaxParticles)
    return;
  Particle particle{};
  particle.lifetime = (std::max)(0.1f, m_settings.lifetime);
  EU::Vector3 direction;
  sampleEmitter(particle.position, direction);
  const float minimumSpeed = (std::max)(0.0f, m_settings.minSpeed);
  const float maximumSpeed = (std::max)(minimumSpeed, m_settings.maxSpeed);
  const float speed = minimumSpeed + random01() * (maximumSpeed - minimumSpeed);
  particle.velocity =
      EU::Vector3(direction.x * speed, direction.y * speed, direction.z * speed);
  m_particles.push_back(particle);
}

void
ParticleEmitterComponent::update(float dt) {
  if (!m_settings.enabled)
    return;
  if (m_settings.emissionMode == ParticleEmissionMode::Continuous) {
    m_spawnAccumulator += dt * (std::max)(0.0f, m_settings.emissionRate);
    while (m_spawnAccumulator >= 1.0f) {
      spawnParticle();
      m_spawnAccumulator -= 1.0f;
    }
  } else if (m_burstPending) {
    const uint32_t count = (std::min)(m_settings.burstCount, MaxParticles);
    for (uint32_t index = 0; index < count; ++index)
      spawnParticle();
    m_burstPending = false;
  }
  for (Particle &particle : m_particles) {
    particle.age += dt;
    particle.velocity.x += m_settings.gravity.x * dt;
    particle.velocity.y += m_settings.gravity.y * dt;
    particle.velocity.z += m_settings.gravity.z * dt;
    particle.position.x += particle.velocity.x * dt;
    particle.position.y += particle.velocity.y * dt;
    particle.position.z += particle.velocity.z * dt;
  }
  m_particles.erase(std::remove_if(m_particles.begin(),
                                   m_particles.end(),
                                   [](const Particle &particle) {
                                     return particle.age >= particle.lifetime;
                                   }),
                    m_particles.end());
}

void
ParticleEmitterComponent::renderParticles(DeviceContext &context,
                                          const Camera &camera,
                                          EditorViewportPass &viewport) {
  if (!m_ready || !m_settings.enabled || m_particles.empty() || !m_transform)
    return;
  std::vector<Vertex> vertices;
  vertices.reserve(m_particles.size() * 6);
  EU::Vector3 right = camera.GetRight(), up = camera.GetUp(),
              origin = m_transform->getPosition();
  const int corners[6][2] = {{-1, -1}, {-1, 1}, {1, 1}, {-1, -1}, {1, 1}, {1, -1}};
  for (const Particle &p : m_particles) {
    const float t = (std::min)(1.0f, p.age / p.lifetime);
    const float size =
        m_settings.startSize + (m_settings.endSize - m_settings.startSize) * t;
    const float r =
        m_settings.startColor.x + (m_settings.endColor.x - m_settings.startColor.x) * t;
    const float g =
        m_settings.startColor.y + (m_settings.endColor.y - m_settings.startColor.y) * t;
    const float b =
        m_settings.startColor.z + (m_settings.endColor.z - m_settings.startColor.z) * t;
    const float a = 1.0f - t;
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
  ID3D11BlendState *blendState = m_settings.blendMode == ParticleBlendMode::Additive
                                     ? m_additiveBlend
                                     : m_alphaBlend;
  context.m_deviceContext->OMSetBlendState(blendState, blendFactor, 0xffffffff);
  context.m_deviceContext->RSSetState(m_rasterizer);
  context.m_deviceContext->OMSetDepthStencilState(m_depthRead, 0);
  context.m_deviceContext->Draw((UINT)vertices.size(), 0);
  ++context.m_drawCallCount;
  context.m_deviceContext->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
  context.m_deviceContext->OMSetDepthStencilState(nullptr, 0);
}

void
ParticleEmitterComponent::applyPreset(ParticlePreset preset) {
  ParticleEmitterSettings settings;
  if (preset == ParticlePreset::Smoke) {
    settings.shape = ParticleEmitterShape::Sphere;
    settings.blendMode = ParticleBlendMode::Alpha;
    settings.emissionRate = 18.0f;
    settings.lifetime = 4.5f;
    settings.minSpeed = 0.15f;
    settings.maxSpeed = 0.35f;
    settings.gravity = EU::Vector3(0.0f, 0.0f, 0.08f);
    settings.startSize = 0.15f;
    settings.endSize = 0.65f;
    settings.startColor = EU::Vector3(0.35f, 0.35f, 0.35f);
    settings.endColor = EU::Vector3(0.08f, 0.08f, 0.08f);
  } else if (preset == ParticlePreset::Sparks) {
    settings.shape = ParticleEmitterShape::Cone;
    settings.emissionMode = ParticleEmissionMode::Burst;
    settings.coneAngleDegrees = 50.0f;
    settings.burstCount = 96;
    settings.lifetime = 1.2f;
    settings.minSpeed = 2.0f;
    settings.maxSpeed = 4.5f;
    settings.gravity = EU::Vector3(0.0f, 0.0f, -4.5f);
    settings.startSize = 0.06f;
    settings.endSize = 0.01f;
    settings.startColor = EU::Vector3(1.0f, 0.75f, 0.15f);
    settings.endColor = EU::Vector3(0.8f, 0.05f, 0.0f);
  } else if (preset == ParticlePreset::Rain) {
    settings.shape = ParticleEmitterShape::Box;
    settings.blendMode = ParticleBlendMode::Alpha;
    settings.boxExtents = EU::Vector3(4.0f, 4.0f, 0.1f);
    settings.direction = EU::Vector3(0.0f, 0.0f, -1.0f);
    settings.gravity = EU::Vector3(0.0f, 0.0f, -1.5f);
    settings.spread = 0.03f;
    settings.emissionRate = 120.0f;
    settings.lifetime = 2.5f;
    settings.minSpeed = 4.0f;
    settings.maxSpeed = 6.0f;
    settings.startSize = 0.035f;
    settings.endSize = 0.02f;
    settings.startColor = EU::Vector3(0.55f, 0.72f, 1.0f);
    settings.endColor = EU::Vector3(0.25f, 0.42f, 0.75f);
  }
  m_settings = settings;
  clearParticles();
  if (m_settings.emissionMode == ParticleEmissionMode::Burst)
    triggerBurst();
}

void
ParticleEmitterComponent::triggerBurst() {
  m_burstPending = true;
}

void
ParticleEmitterComponent::clearParticles() {
  m_particles.clear();
  m_spawnAccumulator = 0.0f;
}

void
ParticleEmitterComponent::destroy() {
  SAFE_RELEASE(m_rasterizer);
  SAFE_RELEASE(m_depthRead);
  SAFE_RELEASE(m_alphaBlend);
  SAFE_RELEASE(m_additiveBlend);
  SAFE_RELEASE(m_inputLayout);
  SAFE_RELEASE(m_pixelShader);
  SAFE_RELEASE(m_vertexShader);
  SAFE_RELEASE(m_constantBuffer);
  SAFE_RELEASE(m_vertexBuffer);
  m_particles.clear();
  m_ready = false;
}
