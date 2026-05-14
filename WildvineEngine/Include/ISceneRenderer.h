/** 
* @File : ISceneRenderer.h
* @brief : Interfaz para renderizadores de escenas.
* @ingroup Rendering
*/

#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;
class Camera;
class RenderScene;
class EdtorViewportPass;
class Tecture;
enum class
	RenderPassType {
	Forward = 0,
	Deferred,
};

/**
* @class ISceneRenderer
* @brief Contrato base para cualquier render consumido por el pipeline pricipal. 
*/

class IsceneRenderer {
public: 
	virtual ~IsceneRenderer() = default;


	virtual HRESULT init(Device& device) = 0;
	virtual void resize(Device& device, unsigned int width, unsigned int height) = 0;
	virtual void render(DeviceContext& deviceContext,
		const Camera& camera, 
		RenderScene& scene, 
		EdtorViewportPass& editorViewportPass) = 0;
	virtual void destroy() = 0;

	virtual ID3D11ShaderResourceView* getShadowMapSRV() const { return nullptr; }
	virtual ID3D11ShaderResourceView* getPreShadowSRV() const { return nullptr; }
	virtual ID3D11ShaderResourceView* getGBufferAlbedoMetallicSRV() const { return nullptr; }
	virtual ID3D11ShaderResourceView* getGBufferNormalRoughnessSRV() const { return nullptr; }
	virtual ID3D11ShaderResourceView* getGBufferWorldAoSRV() const { return nullptr; }
	virtual ID3D11ShaderResourceView* getGBufferEmissiveAlphaSRV() const { return nullptr; }
	virtual void setShadowFactorDebugEnabled(bool enabled) { (void)enabled; }
	virtual void setDeferredDebugViewMode(int mode) { (void)mode; }
	virtual const char* getDebugName() const = 0;
};