/**
 * @file MeshRendererComponent.h
 * @brief Declara la API de MeshRendererComponent dentro del subsistema ECS.
 * @ingroup ecs
 */
#pragma once
#include "Prerequisites.h"
#include "ECS/Component.h"

/** @brief Declara class Mesh. */
class Mesh;
/** @brief Declara class MaterialInstance. */
class MaterialInstance;
/** @brief Declara class DeviceContext. */
class DeviceContext;

/**
 * @class MeshRendererComponent
 * @brief Componente ECS que asocia una malla y uno o varios materiales a una entidad.
 *
 * ActÃºa como puente entre la entidad de escena y el sistema de render:
 * el DeferredRenderer consulta este componente para obtener la malla
 * y la(s) instancia(s) de material que se deben dibujar.
 */
class
MeshRendererComponent : public Component {
public:
  /** @brief Constructor. Registra el componente como tipo MESH. */
  MeshRendererComponent()
      : Component(ComponentType::MESH) {
  }

  /** @brief InicializaciÃ³n sin operaciÃ³n (los recursos se asignan por setter). */
  void
  init() override {
  }

  /** @brief ActualizaciÃ³n por frame (sin lÃ³gica propia en este componente). */
  void
  update(float deltaTime) override {
  }

  /** @brief Render delegado (el renderer usa los datos, no este mÃ©todo). */
  void
  render(DeviceContext &deviceContext) override {
  }

  /** @brief Libera referencias internas (no destruye los recursos GPU). */
  void
  destroy() override {
  }

  /**
   * @brief Asigna la malla que este componente presentarÃ¡ al renderer.
   * @param mesh Puntero a la malla (no propietario; la malla debe vivir mÃ¡s que el
   * componente).
   */
  void
  setMesh(Mesh *mesh) {
    m_mesh = mesh;
  }

  /**
   * @brief Devuelve la malla asignada.
   * @return Puntero a la malla, o nullptr si no se asignÃ³ ninguna.
   */
  Mesh *
  getMesh() const {
    return m_mesh;
  }

  /**
   * @brief Asigna una Ãºnica instancia de material al componente.
   *
   * Limpia la lista de instancias mÃºltiples y la reemplaza con esta.
   *
   * @param materialInstance Puntero a la instancia de material (no propietario).
   */
  void
  setMaterialInstance(MaterialInstance *materialInstance) {
    m_materialInstance = materialInstance;
    m_materialInstances.clear();
    if (materialInstance) {
      m_materialInstances.push_back(materialInstance);
    }
  }

  /**
   * @brief Devuelve la primera instancia de material asignada.
   * @return Puntero a la instancia principal, o nullptr si no se asignÃ³ ninguna.
   */
  MaterialInstance *
  getMaterialInstance() const {
    return m_materialInstance;
  }

  /**
   * @brief Asigna mÃºltiples instancias de material (una por submesh).
   *
   * La primera instancia de la lista se usa como instancia principal.
   *
   * @param materialInstances Vector de punteros a instancias de material.
   */
  void
  setMaterialInstances(const std::vector<MaterialInstance *> &materialInstances) {
    m_materialInstances = materialInstances;
    m_materialInstance =
        m_materialInstances.empty() ? nullptr : m_materialInstances.front();
  }

  /**
   * @brief Agrega una instancia de material a la lista.
   *
   * Si no hay instancia principal, la primera aÃ±adida la convierte en principal.
   *
   * @param materialInstance Puntero a la instancia de material a aÃ±adir.
   */
  void
  addMaterialInstance(MaterialInstance *materialInstance) {
    if (!materialInstance) {
      return;
    }
    if (!m_materialInstance) {
      m_materialInstance = materialInstance;
    }
    m_materialInstances.push_back(materialInstance);
  }

  /**
   * @brief Devuelve la lista completa de instancias de material.
   * @return Referencia constante al vector de instancias.
   */
  const std::vector<MaterialInstance *> &
  getMaterialInstances() const {
    return m_materialInstances;
  }

  /**
   * @brief Indica si la malla es visible para el renderer.
   * @return true si la malla debe renderizarse.
   */
  bool
  isVisible() const {
    return m_visible;
  }

  /**
   * @brief Habilita o deshabilita la visibilidad de la malla.
   * @param visible true para mostrar, false para ocultar.
   */
  void
  setVisible(bool visible) {
    m_visible = visible;
  }

  /**
   * @brief Indica si la malla proyecta sombras.
   * @return true si debe generar shadow map.
   */
  bool
  canCastShadow() const {
    return m_castShadow;
  }

  /**
   * @brief Habilita o deshabilita la proyecciÃ³n de sombras.
   * @param value true para proyectar sombras.
   */
  void
  setCastShadow(bool value) {
    m_castShadow = value;
  }

private:
  /** @brief Malla asociada al componente (no propietario). */
  Mesh *m_mesh = nullptr;
  /** @brief Instancia de material principal. */
  MaterialInstance *m_materialInstance = nullptr;
  /** @brief Instancias de material por submesh. */
  std::vector<MaterialInstance *> m_materialInstances;
  bool m_visible = true;    /**< @brief Flag de visibilidad. */
  bool m_castShadow = true; /**< @brief Flag de proyecciÃ³n de sombras. */
};
