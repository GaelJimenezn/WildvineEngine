/**
 * @file LightComponent.h
 * @brief Declara la API de LightComponent dentro del subsistema ECS.
 * @ingroup ecs
 */
#pragma once
#include "Prerequisites.h"
#include "ECS/Component.h"
#include "Rendering/RenderTypes.h"

/** @brief Declara class DeviceContext. */
class DeviceContext;

/**
 * @class LightComponent
 * @brief Componente ECS que dota a una entidad de propiedades de iluminaciÃ³n.
 *
 * Almacena un LightData con todos los parÃ¡metros de la luz (tipo, color,
 * intensidad, rango, Ã¡ngulo de cono, direcciÃ³n, posiciÃ³n) y un flag de
 * proyecciÃ³n de sombras. El SceneGraph lee este componente para poblar
 * el RenderScene con las listas de luces por tipo.
 */
class LightComponent : public Component {
public:
  /** @brief Constructor. Inicializa el componente sin tipo especÃ­fico (NONE). */
  LightComponent()
      : Component(ComponentType::NONE) {
  }

  /** @brief InicializaciÃ³n del componente (sin operaciÃ³n). */
  void
  init() override {
  }

  /** @brief ActualizaciÃ³n por frame (sin lÃ³gica propia). */
  void
  update(float deltaTime) override {
  }

  /** @brief Render delegado (no tiene representaciÃ³n visual propia). */
  void
  render(DeviceContext &deviceContext) override {
  }

  /** @brief Libera recursos (sin recursos GPU propios). */
  void
  destroy() override {
  }

  /**
   * @brief Accede de forma mutable a los datos de la luz.
   *
   * Permite modificar tipo, color, intensidad, rango, direcciÃ³n, etc.
   *
   * @return Referencia mutable a LightData.
   */
  LightData &
  getLightData() {
    return m_light;
  }

  /**
   * @brief Accede de forma constante a los datos de la luz.
   * @return Referencia constante a LightData.
   */
  const LightData &
  getLightData() const {
    return m_light;
  }

  /**
   * @brief Habilita o deshabilita la proyecciÃ³n de sombras de esta luz.
   *
   * Solo la luz direccional principal proyecta sombras en el pipeline actual.
   *
   * @param value true para activar la proyecciÃ³n de sombras.
   */
  void
  setCastShadow(bool value) {
    m_castShadow = value;
  }

  /**
   * @brief Indica si esta luz proyecta sombras.
   * @return true si la luz genera shadow map.
   */
  bool
  canCastShadow() const {
    return m_castShadow;
  }

private:
  /** @brief ParÃ¡metros completos de la luz (tipo, color, rango, Ã¡ngulo, etc.). */
  LightData m_light;
  bool m_castShadow = false; /**< @brief true si esta luz genera un shadow map. */
};
