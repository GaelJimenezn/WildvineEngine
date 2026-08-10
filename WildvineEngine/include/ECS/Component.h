/**
 * @file Component.h
 * @brief Declara la API de Component dentro del sistema de entidades.
 */
#pragma once
#include "Prerequisites.h"

/**
 * @class DeviceContext
 * @brief DeclaraciÃ³n adelantada del contexto de dispositivo usado para operaciones de
 * render.
 */
class DeviceContext;

/**
 * @class Component
 * @brief Clase base abstracta para todos los componentes del motor.
 *
 * Define la interfaz fundamental que todos los componentes deben implementar:
 * inicializaciÃ³n, actualizaciÃ³n, renderizado y destrucciÃ³n.
 * Cada componente posee un tipo definido por ComponentType.
 */
class
Component {
public:
  /**
   * @brief Constructor por defecto.
   */
  Component() = default;

  /**
   * @brief Constructor que asigna un tipo de componente.
   * @param type Tipo del componente segÃºn ComponentType.
   */
  Component(const ComponentType type)
      : m_type(type) {
  }

  /**
   * @brief Destructor virtual por defecto.
   */
  virtual ~Component() = default;

  /**
   * @brief Inicializa el componente. Debe ser implementada por las clases derivadas.
   */
  virtual void init() = 0;

  /**
   * @brief Actualiza el componente en cada frame.
   * @param deltaTime Tiempo transcurrido entre frames.
   */
  virtual void update(float deltaTime) = 0;

  /**
   * @brief Renderiza el componente.
   * @param deviceContext Contexto del dispositivo para operaciones grÃ¡ficas.
   */
  virtual void render(DeviceContext &deviceContext) = 0;

  /**
   * @brief Libera los recursos internos del componente.
   */
  virtual void destroy() = 0;

  /**
   * @brief Obtiene el tipo del componente.
   * @return Tipo del componente.
   */
  ComponentType
  getType() const {
    return m_type;
  }

protected:
  ComponentType m_type; ///< Tipo del componente definido por ComponentType.
};
