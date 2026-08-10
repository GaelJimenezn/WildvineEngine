/**
 * @file AudioSystem.h
 * @brief Declara el administrador de audio DirectXTK del motor.
 * @ingroup audio
 */
#pragma once

#include <memory>
#include <vector>

class AudioSourceComponent;
class Camera;

/**
 * @class AudioSystem
 * @brief Inicializa DirectXTK y actualiza fuentes de audio 3D registradas.
 *
 * La implementaciÃ³n vive en AudioSystem.cpp para no propagar Audio.h al resto
 * del motor. Cada fuente mantiene su configuraciÃ³n en AudioSourceComponent.
 */
class AudioSystem {
public:
  /** @brief Construye un sistema de audio vacÃ­o. */
  AudioSystem();

  /** @brief Destruye el motor de audio y todas sus voces. */
  ~AudioSystem();

  AudioSystem(const AudioSystem &) = delete;
  AudioSystem &operator=(const AudioSystem &) = delete;

  /**
   * @brief Inicializa AudioEngine de DirectXTK.
   * @return true si se creÃ³ una salida de audio disponible.
   */
  bool init();

  /**
   * @brief Actualiza listener, fuentes y voces de DirectXTK.
   * @param camera CÃ¡mara que actÃºa como listener de audio 3D.
   */
  void update(const Camera &camera);

  /** @brief Detiene y libera todos los recursos de audio. */
  void destroy();

  /**
   * @brief Registra una fuente para que sea procesada cada frame.
   * @param source Componente de audio que se va a administrar.
   */
  void registerSource(AudioSourceComponent *source);

  /**
   * @brief Elimina una fuente del administrador sin destruir el componente.
   * @param source Componente de audio que se va a desregistrar.
   */
  void unregisterSource(AudioSourceComponent *source);

  /**
   * @brief Ajusta el volumen maestro de todas las voces.
   * @param volume Valor lineal entre 0.0 y 1.0.
   */
  void setMasterVolume(float volume);

  /** @brief Pausa el procesamiento de todas las fuentes de audio. */
  void pause();

  /** @brief Reanuda el procesamiento de todas las fuentes de audio. */
  void resume();

  /** @brief Detiene de inmediato todas las voces creadas por las fuentes. */
  void stopAll();

  /**
   * @brief Indica si el sistema de audio se encuentra pausado.
   * @return true si las fuentes estÃ¡n suspendidas.
   */
  bool isPaused() const;

  /**
   * @brief Indica si AudioEngine se inicializÃ³ correctamente.
   * @return true si hay una instancia disponible.
   */
  bool isReady() const;

private:
  struct State;

  std::unique_ptr<State> m_state;
  std::vector<AudioSourceComponent *> m_sources;
};
