/**
 * @file RenderPipeline.h
 * @brief Declara el orquestador de renderers de escena del motor.
 * @ingroup rendering
 */
#pragma once
#include "Rendering/ForwardRenderer.h"
#include "Rendering/DeferredRenderer.h"

/**
 * @class RenderPipeline
 * @brief Selecciona y ejecuta el renderer activo para el frame actual.
 */
class
RenderPipeline {
public:
	HRESULT init(Device& device, RendererType initialRenderer = RendererType::Deferred);
	HRESULT setRendererType(RendererType rendererType, Device& device);
	void resize(Device& device, unsigned int width, unsigned int height);
	void render(DeviceContext& deviceContext,
		const Camera& camera,
		RenderScene& scene,
		EditorViewportPass& viewportPass);
	void destroy();

	RendererType getRendererType() const { return m_activeRendererType; }
	const char* getActiveRendererName() const;
	ID3D11ShaderResourceView* getShadowMapSRV() const;
	ID3D11ShaderResourceView* getPreShadowSRV() const;
	ID3D11ShaderResourceView* getGBufferAlbedoMetallicSRV() const;
	ID3D11ShaderResourceView* getGBufferNormalRoughnessSRV() const;
	ID3D11ShaderResourceView* getGBufferWorldAoSRV() const;
	ID3D11ShaderResourceView* getGBufferEmissiveAlphaSRV() const;
	void setShadowFactorDebugEnabled(bool enabled);

private:
	HRESULT ensureRendererInitialized(RendererType rendererType, Device& device);
	ISceneRenderer* resolveRenderer(RendererType rendererType);
	const ISceneRenderer* resolveRenderer(RendererType rendererType) const;

private:
	ForwardRenderer m_forwardRenderer;
	DeferredRenderer m_deferredRenderer;
	ISceneRenderer* m_activeRenderer = nullptr;
	RendererType m_activeRendererType = RendererType::Deferred;
	bool m_forwardInitialized = false;
	bool m_deferredInitialized = false;
};
