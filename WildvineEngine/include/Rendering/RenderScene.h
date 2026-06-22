#pragma once
#include "Prerequisites.h"
#include "Rendering/RenderTypes.h"

class Skybox;

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

  /** @brief Skybox de la escena. */
  Skybox* skybox = nullptr;
};