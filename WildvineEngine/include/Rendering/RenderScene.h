#pragma once
#include "Prerequisites.h"
#include "Rendering/RenderTypes.h"

class Skybox;

/**
 * @class RenderScene
 * @brief Per-frame render packet built from ECS and scene graph data.
 *
 * Renderers consume this lightweight structure instead of walking gameplay entities
 * directly.
 * Objects are separated into opaque and transparent lists so each renderer can sort and
 * draw
 * them with pass-appropriate state.
 */
class
	RenderScene {
public:
	/**
	 * @brief Clears all gathered objects and lights for reuse in a new frame.
	 * @post Object and light arrays are empty; the skybox pointer is reset by the
	 * implementation if required.
	 */
	void
		clear();

public:
	/** @brief Opaque render objects gathered for depth-friendly rendering. */
	std::vector<RenderObject> opaqueObjects;
	/** @brief Transparent render objects gathered for sorted blending passes. */
	std::vector<RenderObject> transparentObjects;
	/** @brief Directional lights visible to the current render scene. */
	std::vector<LightData> directionalLights;
	/** @brief Optional skybox renderer used by skybox passes; non-owning. */
	Skybox* skybox = nullptr;
};
