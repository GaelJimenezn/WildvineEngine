/**
 * @file SkyboxComponent.h
 * @brief Declara utilidades de SkyboxComponent usadas por WildvineEngine.
 */
#pragma once
#include "Prerequisites.h"
#include "MeshComponent.h"
#include "Skybox.h"

/**
 * @class SkyboxComponent
 * @brief Componente especializado para manejar un skybox dentro del sistema ECS.
 *
 * Hereda de MeshComponent y encapsula la información necesaria para representar
 * un skybox, incluyendo los vértices específicos del cubo.
 */
class
SkyboxComponent :
public
MeshComponent
{
public:
  /**
   * @brief Constructor por defecto.
   */
  SkyboxComponent() : MeshComponent() {}

  /**
   * @brief Destructor por defecto.
   */
  ~SkyboxComponent() = default;

private:
  /**
   * @brief Contenedor de vértices del skybox.
   */
  std::vector<SkyboxVertex> m_skyVertex;


};
