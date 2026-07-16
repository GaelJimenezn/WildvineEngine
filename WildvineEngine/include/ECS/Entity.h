/**
 * @file Entity.h
 * @brief Declara la API de Entity dentro del sistema de entidades.
 */
#pragma once
#include "Prerequisites.h"
#include "Component.h"

/** @brief Declara class DeviceContext. */
class
DeviceContext;

/**
 * @class Entity
 * @brief Clase base para todas las entidades del motor.
 *
 * Define un conjunto de funciones virtuales puras para administrar el ciclo de vida
 * de una entidad: inicialización, actualización, renderizado y destrucción.
 * Las entidades pueden contener múltiples componentes derivados de Component.
 */
class
Entity {
public:
  /**
   * @brief Constructor por defecto de Entity.
   */
  Entity() = default;

  /**
   * @brief Destructor virtual de Entity.
   */
  virtual
  ~Entity() = default;

  /** @brief Declara o ejecuta awake. */
  virtual void
  awake() = 0;


  /**
   * @brief Inicializa la entidad. Debe ser implementado por clases derivadas.
   */
  virtual void
  init() = 0;

  /**
   * @brief Actualiza la entidad por frame.
   * @param deltaTime Tiempo entre frames.
   * @param deviceContext Contexto del dispositivo para operaciones gráficas.
   */
  virtual void
  update(float deltaTime, DeviceContext& deviceContext) = 0;

  /**
   * @brief Renderiza la entidad.
   * @param deviceContext Contexto del dispositivo utilizado para dibujar.
   */
  virtual void
  render(DeviceContext& deviceContext) = 0;

  /**
   * @brief Destruye la entidad y libera sus recursos.
   */
  virtual void
  destroy() = 0;

  /**
   * @brief Agrega un componente a la entidad.
   * @tparam T Tipo del componente (debe heredar de Component).
   * @param component Puntero compartido al componente.
   */
  template <typename T> void
  addComponent(EU::TSharedPointer<T> component) {
    static_assert(
      std::is_base_of<Component,
      T>::value,
      "T must be derived from Component"
    );
    m_components.push_back(component.template dynamic_pointer_cast<Component>());
  }

  /**
   * @brief Obtiene un componente del tipo solicitado, si existe.
   * @tparam T Tipo del componente a buscar.
   * @return Puntero compartido al componente encontrado, o vacío si no existe.
   */
  template<typename T>
  /** @brief Declara o ejecuta getComponent. */
  EU::TSharedPointer<T>
    getComponent() {
    for (auto& component : m_components) {
      EU::TSharedPointer<T> specificComponent =
        component.template dynamic_pointer_cast<T>();
      if (specificComponent) {
        return specificComponent;
      }
    }
    return EU::TSharedPointer<T>();
  }

private:
protected:
  /** @brief Indica si la entidad está activa.. */
  bool m_isActive;
  /** @brief Identificador único de la entidad.. */
  int m_id;
  /** @brief Lista de componentes asociados a la entidad.. */
  std::vector<EU::TSharedPointer<Component>> m_components;
};
