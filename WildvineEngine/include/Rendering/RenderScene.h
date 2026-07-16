/**
 * @file RenderScene.h
 * @brief Declara la estructura RenderScene usada por el pipeline de renderizado.
 * @ingroup rendering
 *
 * RenderScene es el contenedor de datos de escena por frame: el SceneGraph
 * lo rellena con objetos opacos, transparentes y listas de luces clasificadas
 * por tipo, y el renderer lo consume para generar el frame final.
 */
#pragma once
#include "Prerequisites.h"
#include "Rendering/RenderTypes.h"

/** @brief Declara class Skybox. */
class
Skybox;

/**
 * @class RenderScene
 * @brief Contenedor de datos de escena preparados para el pipeline de render.
 *
 * Agrupa los elementos necesarios para el renderizado, incluyendo:
 * - Objetos opacos y transparentes
 * - Luces direccionales
 * - Skybox
 *
 * Esta estructura suele ser generada antes del render y consumida por el renderer.
 */
class
RenderScene {
public:
  /**
   * @brief Limpia todos los elementos de la escena.
   *
   * Vacía las listas de objetos y luces, y reinicia referencias.
   */
  void 
  clear();

public:
  /** @brief Lista de objetos opacos. */
  std::vector<RenderObject> opaqueObjects;

  /** @brief Lista de objetos transparentes. */
  std::vector<RenderObject> transparentObjects;

  /** @brief Lista de luces direccionales. */
  std::vector<LightData> directionalLights;

  /** @brief Lista de luces puntuales. */
  std::vector<LightData> pointLights;

  /** @brief Lista de luces tipo spotlight. */
  std::vector<LightData> spotLights;

  /** @brief Lista de luces rectangulares. */
  std::vector<LightData> rectLights;

  /** @brief Skybox de la escena. */
  Skybox* skybox = nullptr;
};
