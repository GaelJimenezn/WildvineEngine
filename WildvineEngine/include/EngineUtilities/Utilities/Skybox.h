#pragma once
#include "Prerequisites.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "Buffer.h"
#include "SamplerState.h"
#include "Model3D.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "EngineUtilities\Utilities\Camera.h"
#include "ECS\Actor.h"

class Device;
class DeviceContext;

/**
 * @class Skybox
 * @brief Renders the environment cubemap behind the scene.
 *
 * Skybox owns the shader, constant buffer, sampler, rasterizer/depth state, cubemap
 * texture wrapper, and cube actor/model references required to draw the background.
 */
class 
Skybox {
public:
	/** @brief Creates an empty skybox renderer. */
	Skybox()  = default;
	/** @brief Does not automatically release resources; call destroy() in shutdown flow. */
	~Skybox() = default;

	/** @brief Initializes skybox shaders, states, buffers, cube geometry, and cubemap binding. */
	HRESULT 
	init(Device& device, DeviceContext* deviceContext, Texture& cubemap);
	
	/** @brief Updates skybox frame constants from the current camera. */
	void 
	update(DeviceContext& deviceContext, Camera& camera);

	/** @brief Draws the skybox cube using cubemap sampling state. */
	void
	render(DeviceContext& deviceContext);

	/** @brief Releases skybox-owned resources when implemented. */
	void
	destroy() {}

private:
	/** @brief Shader program used by the skybox pass. */
	ShaderProgram m_shaderProgram;
	/** @brief Constant buffer containing view/projection data. */
	Buffer m_constantBuffer;
	/** @brief Sampler used to sample the cubemap. */
	SamplerState m_samplerState;
	/** @brief Rasterizer state usually configured for inside-out cube rendering. */
	RasterizerState m_rasterizerState;
	/** @brief Depth state used to render background behind scene geometry. */
	DepthStencilState m_depthStencilState;
	/** @brief Cubemap texture sampled by the skybox shader. */
	Texture m_skyboxTexture;
	/** @brief Cube model used as skybox geometry. */
	Model3D* m_cubeModel = nullptr;
	/** @brief Actor wrapper that draws the skybox cube. */
	EU::TSharedPointer<Actor> m_skybox;

};
