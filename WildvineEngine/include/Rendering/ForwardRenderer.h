/**
 * @file ForwardRenderer.h
 * @brief Declara la API de ForwardRenderer dentro del subsistema Rendering.
 * @ingroup rendering
 */
#pragma once
#include "Prerequisites.h"
#include "Buffer.h"
#include "DepthStencilState.h"
#include "DepthStencilView.h"
#include "RasterizerState.h"
#include "Rendering/RenderScene.h"
#include "Rendering/RenderTypes.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "EngineUtilities/Utilities/EditorViewportPass.h"

class Device;
class DeviceContext;
class Camera;
class Material;

/**
 * @class ForwardRenderer
 * @brief Ejecuta el pipeline de render forward del motor.
 *
 * Esta clase construye colas opacas y transparentes, genera recursos de sombras,
 * actualiza buffers por frame y compone el resultado final dentro del viewport del
 * editor.
 */
class
	ForwardRenderer {
public:
	/**
	 * @brief Inicializa buffers, shaders y estados del renderer.
	 */
	HRESULT
		init(Device& device);

	/**
	 * @brief Reconstuye los recursos dependientes del tamano del viewport.
	 */
	void
		resize(Device& device, unsigned int width, unsigned int height);

	/**
	 * @brief Actualiza constantes globales usadas por el frame actual.
	 */
	void
		updatePerFrame(
			const Camera& camera,
			const RenderScene& scene,
			DeviceContext& deviceContext
		);

	/**
	 * @brief Renderiza la escena completa sobre el `EditorViewportPass`.
	 */
	void
		render(DeviceContext& deviceContext,
		const Camera& camera,
		RenderScene& scene,
		EditorViewportPass& viewportPass);

	/**
	 * @brief Libera los recursos internos del renderer.
	 */
	void
		destroy();
	/** @brief Returns the shadow-map shader-resource view produced by the shadow pass. */
	ID3D11ShaderResourceView*
		getShadowMapSRV() const { return m_shadowDepthSRV.m_textureFromImg; }
	/** @brief Returns the pre-shadow debug pass texture shown by editor debug UI. */
	ID3D11ShaderResourceView*
		getPreShadowSRV() const { return m_preShadowDebugPass.getSRV(); }

private:
	/** @brief Splits scene objects into opaque and transparent queues for pass execution. */
	void
		buildQueues(RenderScene& scene, const Camera& camera);
	/** @brief Renders an editor debug preview before the shadow pass. */
	void
		renderPreShadowDebugPass(DeviceContext& deviceContext, RenderScene& scene);
	/** @brief Renders shadow-casting objects into the shadow depth map. */
	void
		renderShadowPass(DeviceContext& deviceContext);
	/** @brief Renders queued opaque objects with depth-friendly state. */
	void
		renderOpaquePass(DeviceContext& deviceContext);
	/** @brief Renders queued transparent objects with resolved blend state. */
	void
		renderTransparentPass(DeviceContext& deviceContext);
	/** @brief Renders the scene skybox after opaque geometry. */
	void
		renderSkyboxPass(DeviceContext& deviceContext, RenderScene& scene);
	/** @brief Renders one object for the requested render pass. */
	void
		renderObject(
			DeviceContext& deviceContext,
			const RenderObject& object,
			RenderPassType passType
		);
	/** @brief Renders one object into the shadow map. */
	void
		renderShadowObject(DeviceContext& deviceContext, const RenderObject& object);
	/** @brief Creates shadow-map texture, SRV, DSV, shader, and rasterizer resources. */
	HRESULT
		createShadowResources(Device& device);
	/**
	 * @brief Updates light view/projection matrices from the active camera and scene lights.
	 */
	void
		updateLightMatrices(const Camera& camera, const RenderScene& scene);
	/** @brief Creates blend states used by transparent material modes. */
	HRESULT
		createBlendStates(Device& device);
	/** @brief Returns the D3D blend state matching a material blend mode. */
	ID3D11BlendState*
		resolveBlendState(const Material* material) const;

private:
	/** @brief Per-frame constant buffer bound to scene shaders. */
	Buffer m_perFrameBuffer;
	/** @brief Per-object constant buffer updated before each draw. */
	Buffer m_perObjectBuffer;
	/** @brief Per-material constant buffer updated before each material draw. */
	Buffer m_perMaterialBuffer;
	/** @brief Depth state used while rendering transparent objects. */
	DepthStencilState m_transparentDepthStencil;
	/** @brief Standard alpha blend state. */
	ID3D11BlendState* m_alphaBlendState = nullptr;
	/** @brief Opaque/no-blend state. */
	ID3D11BlendState* m_opaqueBlendState = nullptr;
	/** @brief Additive blend state. */
	ID3D11BlendState* m_additiveBlendState = nullptr;
	/** @brief Premultiplied-alpha blend state. */
	ID3D11BlendState* m_premultipliedBlendState = nullptr;
	/** @brief Blend factor array passed to Output-Merger blend calls. */
	float m_blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	/** @brief Shadow depth texture. */
	Texture m_shadowDepthTexture;
	/** @brief Shader-resource wrapper for the shadow depth texture. */
	Texture m_shadowDepthSRV;
	/** @brief Depth-stencil view used by the shadow pass. */
	DepthStencilView m_shadowDSV;
	/** @brief Shader program used for depth-only shadow rendering. */
	ShaderProgram m_shadowShader;
	/** @brief Rasterizer state used by shadow-map rendering. */
	RasterizerState m_shadowRasterizer;
	/** @brief Square shadow-map resolution in pixels. */
	unsigned int m_shadowMapSize = 2048;
	/** @brief Off-screen debug pass used to inspect pre-shadow output. */
	EditorViewportPass m_preShadowDebugPass;
	/** @brief Master flag that enables shadow application in lighting. */
	bool m_applyShadows = true;

	/** @brief CPU-side per-frame constants. */
	CBPerFrame m_cbPerFrame{};
	/** @brief CPU-side per-object constants. */
	CBPerObject m_cbPerObject{};
	/** @brief CPU-side per-material constants. */
	CBPerMaterial m_cbPerMaterial{};

	/** @brief Opaque queue sorted for efficient forward rendering. */
	std::vector<const RenderObject*> m_opaqueQueue;
	/** @brief Transparent queue sorted back-to-front for blending. */
	std::vector<const RenderObject*> m_transparentQueue;
};
