#pragma once
#include "Prerequisites.h"
#include "ECS\Component.h"


/**
 * @brief Declaración adelantada de la clase DeviceContext.
 */
class 
DeviceContext;

/**
 * @class MeshComponent
 * @brief Clase encargada de almacenar y manejar los datos de una malla,
 *        incluyendo sus vértices e índices, así como su inicialización,
 *        actualización y renderizado.
 */
class
MeshComponent: 
public 
Component{
public:

  /**
   * @brief Constructor por defecto de MeshComponent.
   *        Inicializa el número de vértices e índices en cero.
   */
  MeshComponent() : m_numVertex(0), m_numIndex(0), Component(ComponentType::MESH) {}
  /**
   * @brief Destructor virtual por defecto.
   */
  virtual
  ~MeshComponent() = default;

  /**
   * @brief Inicializa los recursos o configuraciones necesarias de la malla.
   */
  void
  init() override {};

  /**
   * @brief Actualiza el estado de la malla según el tiempo transcurrido.
   * @param deltaTime Tiempo en segundos desde el último frame.
   */
  void
  update(float deltaTime) override {};

  /**
   * @brief Renderiza la malla utilizando el contexto del dispositivo.
   * @param deviceContext Contexto del dispositivo utilizado para dibujar.
   */
  void
  render(DeviceContext& deviceContext) override {};

  /**
   * @brief Libera los recursos asociados a la malla.
   */
  void
    destroy() override {};

public:

  /** @brief Nombre identificador de la malla. */
  std::string m_name;

  /** @brief Vector que contiene los vértices de la malla. */
  std::vector<SimpleVertex> m_vertex;
  std::vector<SkyboxVertex> m_skyVertex;

  /** @brief Vector que contiene los índices de la malla. */
  std::vector<unsigned int> m_index;

  /** @brief Número total de vértices de la malla. */
  int m_numVertex;

  /** @brief Número total de índices de la malla. */
  int m_numIndex;
};
