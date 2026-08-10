/**
 * @file ParticleEmitterComponent.h
 * @brief Emisor CPU de partÃ­culas billboard con render aditivo.
 * @ingroup ecs
 *
 * Simula posiciÃ³n, velocidad, edad y vida en CPU. DespuÃ©s genera quads
 * orientados hacia la cÃ¡mara sin modificar el render Deferred original.
 */
#pragma once

#include "ECS/Component.h"
#include "ECS/Transform.h"
#include <vector>

class Device;
class Camera;
class EditorViewportPass;

/** @brief Emite, simula y renderiza partÃ­culas billboard configurables. */
class ParticleEmitterComponent : public Component {
public:
  /** @brief Asocia el emisor con el transform que define su origen. */
  explicit ParticleEmitterComponent(Transform *transform);

  /** @brief InicializaciÃ³n ECS sin recursos adicionales. */
  void
  init() override {
  }

  /** @brief Crea recursos GPU. @return Resultado HRESULT de la operaciÃ³n. */
  HRESULT init(Device &device);

  /** @brief Avanza emisiÃ³n, movimiento y tiempo de vida. */
  void update(float deltaTime) override;

  /** @brief ImplementaciÃ³n base; el dibujo requiere cÃ¡mara y viewport. */
  void
  render(DeviceContext &context) override {
    (void)context;
  }

  /**
   * @brief Dibuja los billboards sobre el viewport existente.
   * @param context Contexto inmediato de Direct3D 11.
   * @param camera CÃ¡mara usada para orientar los quads.
   * @param viewport Destino que conserva color y profundidad de la escena.
   */
  void renderParticles(DeviceContext &context,
                       const Camera &camera,
                       EditorViewportPass &viewport);

  /** @brief Libera recursos GPU y partÃ­culas vivas. */
  void destroy() override;

  /** @brief Activa o pausa la emisiÃ³n y el dibujo. */
  void
  setEnabled(bool enabled) {
    m_enabled = enabled;
  }
  /** @brief Indica si el emisor estÃ¡ activo. */
  bool
  isEnabled() const {
    return m_enabled;
  }
  /** @brief Devuelve el número actual de partículas vivas. */
  unsigned int
  getLiveParticleCount() const {
    return static_cast<unsigned int>(m_particles.size());
  }
  /** @brief Devuelve la tasa editable en partÃ­culas por segundo. */
  float &
  emissionRate() {
    return m_emissionRate;
  }
  /** @brief Devuelve la vida editable en segundos. */
  float &
  lifetime() {
    return m_lifetime;
  }
  /** @brief Devuelve el tamaÃ±o inicial editable. */
  float &
  startSize() {
    return m_startSize;
  }
  /** @brief Devuelve el tamaÃ±o final editable. */
  float &
  endSize() {
    return m_endSize;
  }
  /** @brief Devuelve el color RGB inicial editable. */
  EU::Vector3 &
  startColor() {
    return m_startColor;
  }
  /** @brief Devuelve el color RGB final editable. */
  EU::Vector3 &
  endColor() {
    return m_endColor;
  }

private:
  /** @brief Estado de simulaciÃ³n de una partÃ­cula. */
  struct Particle {
    EU::Vector3 position;
    EU::Vector3 velocity;
    float age = 0.0f;
    float lifetime = 1.0f;
  };

  /** @brief VÃ©rtice expandido enviado al shader. */
  struct Vertex {
    float position[3];
    float color[4];
  };

  /** @brief Constant buffer con la matriz ViewProjection. */
  struct Constants {
    XMMATRIX viewProjection;
  };

  /** @brief Inserta una partÃ­cula si existe capacidad. */
  void spawnParticle();
  /** @brief Produce un valor pseudoaleatorio determinista en [0, 1). */
  float random01();
  /** @brief Compila y crea los shaders del pass. */
  HRESULT compileShaders(Device &device);

  Transform *m_transform = nullptr;
  std::vector<Particle> m_particles;
  ID3D11Buffer *m_vertexBuffer = nullptr;
  ID3D11Buffer *m_constantBuffer = nullptr;
  ID3D11VertexShader *m_vertexShader = nullptr;
  ID3D11PixelShader *m_pixelShader = nullptr;
  ID3D11InputLayout *m_inputLayout = nullptr;
  ID3D11BlendState *m_additiveBlend = nullptr;
  ID3D11DepthStencilState *m_depthRead = nullptr;
  ID3D11RasterizerState *m_rasterizer = nullptr;
  float m_spawnAccumulator = 0.0f;
  float m_emissionRate = 24.0f;
  float m_lifetime = 1.8f;
  float m_startSize = 0.18f;
  float m_endSize = 0.03f;
  EU::Vector3 m_startColor = EU::Vector3(1.0f, 0.35f, 0.05f);
  EU::Vector3 m_endColor = EU::Vector3(0.08f, 0.02f, 0.0f);
  unsigned int m_randomState = 0x13579BDFu;
  bool m_enabled = true;
  bool m_ready = false;
  static constexpr unsigned int MaxParticles = 256;
};
