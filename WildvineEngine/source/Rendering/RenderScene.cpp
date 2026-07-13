/**
 * @file RenderScene.cpp
 * @brief Implementa la logica de RenderScene dentro del subsistema Rendering.
 * @ingroup rendering
 */
#include "Rendering/RenderScene.h"

void
RenderScene::clear() {
	opaqueObjects.clear();
	transparentObjects.clear();
	directionalLights.clear();
	pointLights.clear();
	spotLights.clear();
	rectLights.clear();
	skybox = nullptr;
}