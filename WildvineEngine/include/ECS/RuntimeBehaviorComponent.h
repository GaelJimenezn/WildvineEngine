/**
 * @file RuntimeBehaviorComponent.h
 * @brief Declara un comportamiento demostrativo ejecutable solo en Play.
 * @ingroup ecs
 */
#pragma once

#include <cmath>

#include "ECS/Component.h"
#include "ECS/Transform.h"

/**
 * @class RuntimeBehaviorComponent
 * @brief Aplica movimiento oscilante y rotaciÃ³n a un Transform en runtime.
 *
 * Este componente no depende de scripting externo. BaseApp activa o desactiva
 * su ejecuciÃ³n al cambiar entre los modos Edit y Play.
 */
class
RuntimeBehaviorComponent : public Component {
public:
  /**
   * @brief Construye un comportamiento asociado a un Transform.
   * @param transform Transform que recibirÃ¡ el movimiento durante Play.
   */
  explicit RuntimeBehaviorComponent(Transform *transform)
      : Component(ComponentType::BEHAVIOR)
      , m_transform(transform) {
  }

  /** @brief No requiere inicializaciÃ³n adicional. */
  void
  init() override {
  }

  /**
   * @brief Actualiza el desplazamiento y la rotaciÃ³n de runtime.
   * @param deltaTime Tiempo transcurrido desde el frame anterior en segundos.
   */
  void
  update(float deltaTime) override {
    if (!m_running || !m_transform) {
      return;
    }

    m_elapsedTime += deltaTime;
    const float offset = std::sin(m_elapsedTime * m_moveFrequency) * m_moveRange;
    EU::Vector3 position = m_startPosition;
    position.x += offset;

    EU::Vector3 rotation = m_startRotation;
    rotation.y += m_elapsedTime * m_rotationSpeed;
    m_transform->setTransform(position, rotation, m_startScale);
    m_transform->rebuildMatrixFromVectors();
  }

  /** @brief No genera render directo. */
  void
  render(DeviceContext &deviceContext) override {
    (void)deviceContext;
  }

  /** @brief Libera las referencias no propietarias del componente. */
  void
  destroy() override {
    m_transform = nullptr;
  }

  /**
   * @brief Activa o detiene el comportamiento de runtime.
   * @param running true para iniciar desde el estado actual del Transform.
   */
  void
  setRunning(bool running) {
    m_running = running;
    m_elapsedTime = 0.0f;
    if (!m_running || !m_transform) {
      return;
    }

    m_startPosition = m_transform->getPosition();
    m_startRotation = m_transform->getRotation();
    m_startScale = m_transform->getScale();
  }

  /**
   * @brief Indica si el comportamiento se ejecuta actualmente.
   * @return true si el modo Play lo ha activado.
   */
  bool
  isRunning() const {
    return m_running;
  }

private:
  Transform *m_transform = nullptr;
  EU::Vector3 m_startPosition;
  EU::Vector3 m_startRotation;
  EU::Vector3 m_startScale;
  float m_elapsedTime = 0.0f;
  float m_moveRange = 1.5f;
  float m_moveFrequency = 1.25f;
  float m_rotationSpeed = 0.8f;
  bool m_running = false;
};
