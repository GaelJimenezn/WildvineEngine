/**
 * @file ParticleEmitterComponent.h
 * @brief Emisor CPU de partÃ­culas billboard configurable.
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

/** @brief Forma espacial usada para generar nuevas partÃ­culas. */
enum class ParticleEmitterShape : uint8_t {
  Point,  ///< Emite desde el origen del actor.
  Box,    ///< Distribuye nacimientos dentro de una caja.
  Sphere, ///< Distribuye nacimientos dentro de una esfera.
  Cone    ///< Proyecta partÃ­culas dentro de un cono orientable.
};

/** @brief Estrategia temporal usada para emitir partÃ­culas. */
enum class ParticleEmissionMode : uint8_t {
  Continuous, ///< Mantiene una tasa estable de partÃ­culas por segundo.
  Burst       ///< Emite un grupo completo cuando se dispara manualmente.
};

/** @brief OperaciÃ³n usada para combinar partÃ­culas y escena. */
enum class ParticleBlendMode : uint8_t {
  Additive, ///< Suma luz y resulta apropiado para fuego o chispas.
  Alpha     ///< Interpola con la escena y resulta apropiado para humo.
};

/** @brief Configuraciones preparadas para efectos comunes. */
enum class ParticlePreset : uint8_t {
  Fire,   ///< Llama ascendente naranja con mezcla aditiva.
  Smoke,  ///< Volumen gris expansivo con mezcla alpha.
  Sparks, ///< RÃ¡faga cÃ³nica afectada por gravedad descendente.
  Rain    ///< Volumen de gotas que cae desde una caja.
};

/**
 * @struct ParticleEmitterSettings
 * @brief Agrupa todos los parÃ¡metros serializables del emisor.
 *
 * Las direcciones y fuerzas usan coordenadas de mundo con Z como eje vertical.
 * `spread` representa una desviaciÃ³n normalizada entre cero y uno.
 */
struct
ParticleEmitterSettings {
  ParticleEmitterShape shape = ParticleEmitterShape::Point; ///< Forma del emisor.
  ParticleEmissionMode emissionMode =
      ParticleEmissionMode::Continuous;                      ///< Estrategia temporal.
  ParticleBlendMode blendMode = ParticleBlendMode::Additive; ///< Mezcla de color.
  EU::Vector3 direction =
      EU::Vector3(0.0f, 0.0f, 1.0f); ///< DirecciÃ³n central normalizable.
  EU::Vector3 gravity =
      EU::Vector3(0.0f, 0.0f, 0.15f); ///< AceleraciÃ³n mundial por segundo.
  EU::Vector3 boxExtents =
      EU::Vector3(0.5f, 0.5f, 0.5f); ///< SemitamaÃ±o de la caja emisora.
  float sphereRadius = 0.5f;         ///< Radio del volumen esfÃ©rico.
  float coneAngleDegrees = 20.0f;    ///< Semiapertura angular del cono.
  float spread = 0.25f;              ///< Ruido direccional normalizado.
  float minSpeed = 0.65f;            ///< Velocidad inicial mÃ­nima.
  float maxSpeed = 1.30f;            ///< Velocidad inicial mÃ¡xima.
  float emissionRate = 24.0f;        ///< PartÃ­culas continuas por segundo.
  float lifetime = 1.8f;             ///< Vida individual en segundos.
  float startSize = 0.18f;           ///< TamaÃ±o al nacer.
  float endSize = 0.03f;             ///< TamaÃ±o al morir.
  EU::Vector3 startColor = EU::Vector3(1.0f, 0.35f, 0.05f); ///< Color RGB al nacer.
  EU::Vector3 endColor = EU::Vector3(0.08f, 0.02f, 0.0f);   ///< Color RGB al morir.
  uint32_t burstCount = 64; ///< Cantidad generada por Burst.
  bool enabled = true;      ///< Activa simulaciÃ³n y dibujo.
};

/** @brief Emite, simula y renderiza partÃ­culas billboard configurables. */
class
ParticleEmitterComponent : public Component {
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

  /** @brief Aplica un preset completo y reinicia la simulaciÃ³n. */
  void applyPreset(ParticlePreset preset);

  /** @brief Solicita una nueva emisiÃ³n cuando se usa el modo Burst. */
  void triggerBurst();

  /** @brief Elimina todas las partÃ­culas vivas sin cambiar la configuraciÃ³n. */
  void clearParticles();

  /** @brief Devuelve la configuraciÃ³n editable y serializable. */
  ParticleEmitterSettings &
  settings() {
    return m_settings;
  }

  /** @brief Devuelve la configuraciÃ³n serializable de solo lectura. */
  const ParticleEmitterSettings &
  settings() const {
    return m_settings;
  }

  /** @brief Activa o pausa la emisiÃ³n y el dibujo. */
  void
  setEnabled(bool enabled) {
    m_settings.enabled = enabled;
    if (enabled && m_settings.emissionMode == ParticleEmissionMode::Burst)
      triggerBurst();
  }
  /** @brief Indica si el emisor estÃ¡ activo. */
  bool
  isEnabled() const {
    return m_settings.enabled;
  }
  /** @brief Devuelve el número actual de partículas vivas. */
  unsigned int
  getLiveParticleCount() const {
    return static_cast<unsigned int>(m_particles.size());
  }
  /** @brief Devuelve la tasa editable en partÃ­culas por segundo. */
  float &
  emissionRate() {
    return m_settings.emissionRate;
  }
  /** @brief Devuelve la vida editable en segundos. */
  float &
  lifetime() {
    return m_settings.lifetime;
  }
  /** @brief Devuelve el tamaÃ±o inicial editable. */
  float &
  startSize() {
    return m_settings.startSize;
  }
  /** @brief Devuelve el tamaÃ±o final editable. */
  float &
  endSize() {
    return m_settings.endSize;
  }
  /** @brief Devuelve el color RGB inicial editable. */
  EU::Vector3 &
  startColor() {
    return m_settings.startColor;
  }
  /** @brief Devuelve el color RGB final editable. */
  EU::Vector3 &
  endColor() {
    return m_settings.endColor;
  }

private:
  /** @brief Estado de simulaciÃ³n de una partÃ­cula. */
  struct
  Particle {
    EU::Vector3 position;
    EU::Vector3 velocity;
    float age = 0.0f;
    float lifetime = 1.0f;
  };

  /** @brief VÃ©rtice expandido enviado al shader. */
  struct
  Vertex {
    float position[3];
    float color[4];
  };

  /** @brief Constant buffer con la matriz ViewProjection. */
  struct
  Constants {
    XMMATRIX viewProjection;
  };

  /** @brief Inserta una partÃ­cula si existe capacidad. */
  void spawnParticle();
  /** @brief Calcula posiciÃ³n y direcciÃ³n segÃºn la forma configurada. */
  void sampleEmitter(EU::Vector3 &position, EU::Vector3 &direction);
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
  ID3D11BlendState *m_alphaBlend = nullptr;
  ID3D11DepthStencilState *m_depthRead = nullptr;
  ID3D11RasterizerState *m_rasterizer = nullptr;
  ParticleEmitterSettings m_settings;
  float m_spawnAccumulator = 0.0f;
  unsigned int m_randomState = 0x13579BDFu;
  bool m_burstPending = false;
  bool m_ready = false;
  static constexpr unsigned int MaxParticles = 1024;
};
