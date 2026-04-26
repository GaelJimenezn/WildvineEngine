#pragma once
#include "Prerequisites.h"
#include "Rendering/RenderTypes.h"

class Skybox;

/**
 * @class RenderScene
 * @brief Estructura de datos temporal que agrupa todos los elementos visibles de la escena en un frame.
 */
class
RenderScene {

public:
	/**
	 * @brief Limpia todas las listas y punteros de la escena, preparándola para ser poblada en el siguiente frame.
	 */
	void 
	clear();

public:
	std::vector<RenderObject> opaqueObjects;       /**< Lista de objetos a renderizar sin transparencia. */
	std::vector<RenderObject> transparentObjects;  /**< Lista de objetos que requieren blending (transparencias). */
	std::vector<LightData> directionalLights;      /**< Lista de luces direccionales activas en la escena. */
	Skybox* skybox = nullptr;                      /**< Puntero al Skybox actual a dibujar de fondo. */
};