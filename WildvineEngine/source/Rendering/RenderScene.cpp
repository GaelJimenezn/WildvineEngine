#include "Rendering/RenderScene.h"

void
RenderScene::clear() {
	opaqueObjects.clear();
	transparentObjects.clear();
	directionalLights.clear();
	skybox = nullptr;
}
