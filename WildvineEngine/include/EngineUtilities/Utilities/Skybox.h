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

class 
Skybox {
public:
	Skybox()  = default;
	~Skybox() = default;

	HRESULT 
	init(Device& device, DeviceContext* deviceContext, Texture& cubemap);
	
	void 
	update(DeviceContext& deviceContext, Camera& camera);

	void
	render(DeviceContext& deviceContext);

	void
	destroy() {}

private:
	ShaderProgram m_shaderProgram;
	Buffer m_constantBuffer;
	SamplerState m_samplerState;
	RasterizerState m_rasterizerState;
	DepthStencilState m_depthStencilState;
	Texture m_skyboxTexture;
	Model3D* m_cubeModel = nullptr;
	EU::TSharedPointer<Actor> m_skybox;

};