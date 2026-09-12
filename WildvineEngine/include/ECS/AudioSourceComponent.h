/**
 * @file AudioSourceComponent.h
 * @brief Declara la fuente de sonido espacial del sistema ECS.
 * @ingroup audio
 */
#pragma once

#include <string>

#include "ECS/Component.h"

class AudioSystem;
class Transform;

/**
 * @class AudioSourceComponent
 * @brief Representa un archivo WAV reproducible desde la posiciÃ³n de un actor.
 *
 * AudioSystem resuelve el archivo, crea la voz de DirectXTK y aplica el audio
 * 3D usando el Transform asociado como emisor.
 */
class
AudioSourceComponent : public Component {
public:
  /**
   * @brief Construye una fuente enlazada a un Transform.
   * @param transform Transform que define la posiciÃ³n del emisor.
   */
  explicit AudioSourceComponent(Transform *transform)
      : Component(ComponentType::AUDIO)
      , m_transform(transform) {
  }

  /** @brief No requiere inicializaciÃ³n adicional. */
  void
  init() override {
  }

  /** @brief AudioSystem actualiza esta fuente de forma centralizada. */
  void
  update(float deltaTime) override {
    (void)deltaTime;
  }

  /** @brief No genera render directo. */
  void
  render(DeviceContext &deviceContext) override {
    (void)deviceContext;
  }

  /** @brief Libera la referencia no propietaria al Transform. */
  void
  destroy() override {
    m_transform = nullptr;
  }

  /**
   * @brief Asigna el archivo WAV que reproducirÃ¡ la fuente.
   * @param path Ruta relativa o absoluta a un archivo WAV PCM compatible.
   */
  void
  setAudioPath(const std::string &path) {
    if (m_audioPath != path) {
      m_audioPath = path;
      m_assetDirty = true;
    }
  }

  /** @brief Solicita comenzar la reproducciÃ³n en la prÃ³xima actualizaciÃ³n. */
  void
  play() {
    m_playRequested = true;
    m_stopRequested = false;
  }

  /** @brief Solicita detener inmediatamente la reproducciÃ³n actual. */
  void
  stop() {
    m_stopRequested = true;
    m_playRequested = false;
  }

  /**
   * @brief Establece el volumen lineal de la fuente.
   * @param volume Valor entre 0.0 y 1.0.
   */
  void
  setVolume(float volume) {
    m_volume = volume < 0.0f ? 0.0f : (volume > 1.0f ? 1.0f : volume);
  }

  /**
   * @brief Ajusta el tono de reproducciÃ³n en semitonos normalizados.
   * @param pitch Valor entre -1.0 y 1.0; 0.0 conserva el tono original.
   */
  void
  setPitch(float pitch) {
    m_pitch = pitch < -1.0f ? -1.0f : (pitch > 1.0f ? 1.0f : pitch);
  }

  /**
   * @brief Activa o desactiva la repeticiÃ³n continua del sonido.
   * @param loop true para reiniciar el sonido al llegar a su final.
   */
  void
  setLoop(bool loop) {
    m_loop = loop;
  }

  /**
   * @brief Define si esta fuente inicia automÃ¡ticamente al entrar a Play.
   * @param autoActivate true para comenzar durante el modo Play.
   */
  void
  setAutoActivate(bool autoActivate) {
    m_autoActivate = autoActivate;
  }

  /**
   * @brief Silencia o restaura la fuente sin modificar su volumen configurado.
   * @param muted true para silenciar la voz de esta fuente.
   */
  void
  setMuted(bool muted) {
    m_muted = muted;
  }

  /**
   * @brief Ajusta el rango audible de la atenuaciÃ³n espacial 3D.
   * @param minDistance Distancia con volumen completo.
   * @param maxDistance Distancia a partir de la cual deja de oÃ­rse.
   */
  void
  setAttenuation(float minDistance, float maxDistance) {
    m_minDistance = minDistance < 0.0f ? 0.0f : minDistance;
    m_maxDistance = maxDistance > m_minDistance ? maxDistance : m_minDistance + 0.01f;
  }

  /**
   * @brief Activa o desactiva la atenuaciÃ³n espacial 3D de la fuente.
   * @param spatial true para ubicar el sonido en la posiciÃ³n del actor.
   */
  void
  setSpatial(bool spatial) {
    if (m_spatial != spatial) {
      m_spatial = spatial;
      m_assetDirty = true;
    }
  }

  /**
   * @brief Indica si la fuente usa atenuaciÃ³n espacial 3D.
   * @return true cuando el audio se posiciona usando el Transform del actor.
   */
  bool
  isSpatial() const {
    return m_spatial;
  }

  /** @brief Devuelve el tono configurado para la fuente. */
  float
  getPitch() const {
    return m_pitch;
  }

  /** @brief Indica si la fuente se repite al terminar. */
  bool
  isLooping() const {
    return m_loop;
  }

  /** @brief Indica si la fuente debe iniciar al entrar al modo Play. */
  bool
  isAutoActivate() const {
    return m_autoActivate;
  }

  /** @brief Indica si la fuente estÃ¡ silenciada. */
  bool
  isMuted() const {
    return m_muted;
  }

  /** @brief Devuelve la distancia de volumen completo. */
  float
  getMinDistance() const {
    return m_minDistance;
  }

  /** @brief Devuelve la distancia mÃ¡xima audible. */
  float
  getMaxDistance() const {
    return m_maxDistance;
  }

  /**
   * @brief Devuelve la ruta de audio configurada.
   * @return Ruta del archivo WAV.
   */
  const std::string &
  getAudioPath() const {
    return m_audioPath;
  }

  /**
   * @brief Devuelve el volumen lineal configurado.
   * @return Valor entre 0.0 y 1.0.
   */
  float
  getVolume() const {
    return m_volume;
  }

private:
  friend class AudioSystem;

  Transform *m_transform = nullptr;
  std::string m_audioPath;
  float m_volume = 1.0f;
  float m_pitch = 0.0f;
  float m_minDistance = 1.0f;
  float m_maxDistance = 25.0f;
  bool m_spatial = false;
  bool m_loop = false;
  bool m_autoActivate = false;
  bool m_muted = false;
  bool m_assetDirty = true;
  bool m_playRequested = false;
  bool m_stopRequested = false;
};
