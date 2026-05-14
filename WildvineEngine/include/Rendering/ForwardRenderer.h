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
 * actualiza buffers por frame y compone el resultado final dentro del viewport del editor.
 */
class
	ForwardRenderer {
public:
	/**
	 * @brief Inicializa buffers, shaders y estados del renderer.
	 */
	HRESULT init(Device& device);

	/**
	 * @brief Reconstuye los recursos dependientes del tamano del viewport.
	 */
	void resize(Device& device, unsigned int width, unsigned int height);

	/**
	 * @brief Actualiza constantes globales usadas por el frame actual.
	 */
	void updatePerFrame(const Camera& camera, const RenderScene& scene, DeviceContext& deviceContext);

	/**
	 * @brief Renderiza la escena completa sobre el `EditorViewportPass`.
	 */
	void render(DeviceContext& deviceContext,
		const Camera& camera,
		RenderScene& scene,
		EditorViewportPass& viewportPass);

	/**
	 * @brief Libera los recursos internos del renderer.
	 */
	void destroy();
	ID3D11ShaderResourceView* getShadowMapSRV() const { return m_shadowDepthSRV.m_textureFromImg; }
	ID3D11ShaderResourceView* getPreShadowSRV() const { return m_preShadowDebugPass.getSRV(); }

private:
	void buildQueues(RenderScene& scene, const Camera& camera);
	void renderPreShadowDebugPass(DeviceContext& deviceContext, RenderScene& scene);
	void renderShadowPass(DeviceContext& deviceContext);
	void renderOpaquePass(DeviceContext& deviceContext);
	void renderTransparentPass(DeviceContext& deviceContext);
	void renderSkyboxPass(DeviceContext& deviceContext, RenderScene& scene);
	void renderObject(DeviceContext& deviceContext, const RenderObject& object, RenderPassType passType);
	void renderShadowObject(DeviceContext& deviceContext, const RenderObject& object);
	HRESULT createShadowResources(Device& device);
	void updateLightMatrices(const Camera& camera, const RenderScene& scene);
	HRESULT createBlendStates(Device& device);
	ID3D11BlendState* resolveBlendState(const Material* material) const;

private:
	Buffer m_perFrameBuffer;
	Buffer m_perObjectBuffer;
	Buffer m_perMaterialBuffer;
	DepthStencilState m_transparentDepthStencil;
	ID3D11BlendState* m_alphaBlendState = nullptr;
	ID3D11BlendState* m_opaqueBlendState = nullptr;
	ID3D11BlendState* m_additiveBlendState = nullptr;
	ID3D11BlendState* m_premultipliedBlendState = nullptr;
	float m_blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	Texture m_shadowDepthTexture;
	Texture m_shadowDepthSRV;
	DepthStencilView m_shadowDSV;
	ShaderProgram m_shadowShader;
	RasterizerState m_shadowRasterizer;
	unsigned int m_shadowMapSize = 2048;
	EditorViewportPass m_preShadowDebugPass;
	bool m_applyShadows = true;

	CBPerFrame m_cbPerFrame{};
	CBPerObject m_cbPerObject{};
	CBPerMaterial m_cbPerMaterial{};

	std::vector<const RenderObject*> m_opaqueQueue;
	std::vector<const RenderObject*> m_transparentQueue;
};