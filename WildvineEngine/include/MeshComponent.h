/**
 * @file MeshComponent.h
 * @brief Declara la API pÃºblica de MeshComponent dentro de WildvineEngine.
 */
#pragma once
#include "Prerequisites.h"
#include "ECS\Component.h"

/**
 * @brief DeclaraciÃ³n adelantada de la clase DeviceContext.
 */
class DeviceContext;

/**
 * @class MeshComponent
 * @brief Clase encargada de almacenar y manejar los datos de una malla,
 *        incluyendo sus vÃ©rtices e Ã­ndices, asÃ­ como su inicializaciÃ³n,
 *        actualizaciÃ³n y renderizado.
 */
class MeshComponent : public Component {
public:
  /**
   * @brief Constructor por defecto de MeshComponent.
   *        Inicializa el nÃºmero de vÃ©rtices e Ã­ndices en cero.
   */
  MeshComponent()
      : m_numVertex(0)
      , m_numIndex(0)
      , Component(ComponentType::MESH) {
  }
  /**
   * @brief Destructor virtual por defecto.
   */
  virtual ~MeshComponent() = default;

  /**
   * @brief Inicializa los recursos o configuraciones necesarias de la malla.
   */
  void init() override {};

  /**
   * @brief Actualiza el estado de la malla segÃºn el tiempo transcurrido.
   * @param deltaTime Tiempo en segundos desde el Ãºltimo frame.
   */
  void update(float deltaTime) override {};

  /**
   * @brief Renderiza la malla utilizando el contexto del dispositivo.
   * @param deviceContext Contexto del dispositivo utilizado para dibujar.
   */
  void render(DeviceContext &deviceContext) override {};

  /**
   * @brief Libera los recursos asociados a la malla.
   */
  void destroy() override {};

public:
  /** @brief Nombre identificador de la malla. */
  std::string m_name;

  /** @brief Vector que contiene los vÃ©rtices de la malla. */
  std::vector<SimpleVertex> m_vertex;
  std::vector<SkyboxVertex> m_skyVertex;

  /** @brief Vector que contiene los Ã­ndices de la malla. */
  std::vector<unsigned int> m_index;

  /** @brief NÃºmero total de vÃ©rtices de la malla. */
  int m_numVertex;

  /** @brief NÃºmero total de Ã­ndices de la malla. */
  int m_numIndex;

  /** @brief Ãndice de material usado por esta malla. */
  int m_materialIndex = 0;

  /** @brief Transform local original del nodo. */
  XMFLOAT4X4 m_localTransform =
      XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
};
