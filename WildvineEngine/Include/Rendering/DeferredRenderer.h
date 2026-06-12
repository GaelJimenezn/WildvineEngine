/**
 * @file DeferredRenderer.h
 * @brief Declara la API de DeferredRenderer dentro del subsistema Rendering.
 * @ingroup rendering
 */
#pragma once
#include "Buffer.h"
#include "DepthStencilState.h"
#include "DepthStencilView.h"
#include "RasterizerState.h"
#include "Rendering/ISceneRenderer.h"
#include "Rendering/RenderScene.h"
#include "Rendering/RenderTypes.h"
#include "SamplerState.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "EngineUtilities/Utilities/EditorViewportPass.h"

class Device;
class DeviceContext;
class Camera;
class Material;

/**
 * @class DeferredRenderer
 * @brief Implementa un pipeline diferido con GBuffer y lighting pass.
 *
 * El renderer usa deferred shading para superficies opacas y mantiene un subpass
 * forward para transparencias, de modo que el pipeline del editor siga funcionando
 * con el contenido actual del engine.
 */
class
	DeferredRenderer : public ISceneRenderer {
public:
    /**
     * @brief Creates renderer-owned GPU resources, shaders, GBuffer targets, and pass
     * state.
     */
    HRESULT
    	init(Device& device) override;

    /** @brief Recreates size-dependent GBuffer/final-target resources. */
    void
    	resize(Device& device, unsigned int width, unsigned int height) override;

    /**
     * @brief Renders the scene through geometry, lighting, skybox, and transparent
     * passes.
     */
    void
    	render(DeviceContext& deviceContext,
           const Camera& camera,
           RenderScene& scene,
           EditorViewportPass& viewportPass) override;

    /** @brief Releases all GPU resources owned by the deferred renderer. */
    void
    	destroy() override;

    /** @brief Returns the shadow-map SRV produced by the shadow pass. */
    ID3D11ShaderResourceView*
    	getShadowMapSRV() const override { return m_shadowDepthSRV.m_textureFromImg; }

    /** @brief Returns the pre-shadow debug texture displayed by editor debug UI. */
    ID3D11ShaderResourceView*
    	getPreShadowSRV() const override { return m_preShadowDebugPass.getSRV(); }

    /** @brief Returns the GBuffer albedo/metallic SRV. */
    ID3D11ShaderResourceView*
    	getGBufferAlbedoMetallicSRV() const override {
    		return m_gBufferAlbedoMetallicSRV.m_textureFromImg;
    	}

    /** @brief Returns the GBuffer normal/roughness SRV. */
    ID3D11ShaderResourceView*
    	getGBufferNormalRoughnessSRV() const override {
    		return m_gBufferNormalRoughnessSRV.m_textureFromImg;
    	}

    /** @brief Returns the GBuffer world-position/AO SRV. */
    ID3D11ShaderResourceView*
    	getGBufferWorldAoSRV() const override {
    		return m_gBufferWorldAoSRV.m_textureFromImg;
    	}

    /** @brief Returns the GBuffer emissive/alpha SRV. */
    ID3D11ShaderResourceView*
    	getGBufferEmissiveAlphaSRV() const override {
    		return m_gBufferEmissiveAlphaSRV.m_textureFromImg;
    	}

    /** @brief Enables a lighting debug mode that visualizes shadow contribution. */
    void
    	setShaderFactorDebugEnabled(bool enabled) override {
    		m_shadowFactorDebugEnabled = enabled;
    	}

    /** @brief Selects which deferred debug view the lighting shader should output. */
    void
    	setDeferredDebugViewMode(int mode) override { m_deferredDebugViewMode = mode; }

    /** @brief Returns the renderer name used by debug/editor UI. */
    const char*
    	getDebugName() const override { return "DeferredRenderer"; }

private:
    /** @brief Splits scene objects into opaque and transparent queues. */
    void
    	buildQueues(RenderScene& scene, const Camera& camera);

    /** @brief Updates per-frame constants used by geometry and lighting shaders. */
    void
    	updatePerFrame(
    		const Camera& camera,
    		const RenderScene& scene,
    		DeviceContext& deviceContext
    	);

    /** @brief Computes light view/projection matrices for shadowing. */
    void
    	updateLightMatrices(const Camera& camera, const RenderScene& scene);

    /** @brief Runs the complete scene render into @p targetPass. */
    void
    	renderSceneToTarget(
    		DeviceContext& deviceContext,
    		RenderScene& scene,
    		EditorViewportPass& targetPass,
    		bool applyShadows
    	);

    /** @brief Binds all GBuffer render targets plus the shared depth buffer. */
    void
    	bindGBufferTargets(
    		DeviceContext& deviceContext,
    		ID3D11DepthStencilView* depthStencilView
    	);

    /** @brief Binds the final color target used by lighting/composition. */
    void
    	bindFinalTarget(
    		DeviceContext& deviceContext,
    		ID3D11RenderTargetView* renderTargetView,
    		ID3D11DepthStencilView* depthStencilView
    	);

    /**
     * @brief Unbinds deferred SRVs to avoid read/write hazards before rendering targets.
     */
    void
    	clearDeferredSRVs(DeviceContext& deviceContext);

    /** @brief Renders opaque objects into the GBuffer. */
    void
    	renderGeometryPass(DeviceContext& deviceContext);

    /** @brief Renders one object into the GBuffer. */
    void
    	renderGeometryObject(DeviceContext& deviceContext, const RenderObject& object);

    /** @brief Renders a fullscreen lighting pass from GBuffer inputs. */
    void
    	renderLightingPass(DeviceContext& deviceContext);

    /** @brief Renders the skybox into the final target. */
    void
    	renderSkyboxPass(DeviceContext& deviceContext, RenderScene& scene);

    /** @brief Renders sorted transparent objects with forward shading. */
    void
    	renderTransparentPass(DeviceContext& deviceContext);

    /** @brief Renders one object through the forward fallback path. */
    void
    	renderForwardObject(
    		DeviceContext& deviceContext,
    		const RenderObject& object,
    		RenderPassType passType
    	);

    /** @brief Renders shadow-casting objects into the shadow depth map. */
    void
    	renderShadowPass(DeviceContext& deviceContext);

    /** @brief Renders one object into the shadow map. */
    void
    	renderShadowObject(DeviceContext& deviceContext, const RenderObject& object);

    /** @brief Creates shadow-map texture, SRV, DSV, shader, and pass state. */
    HRESULT
    	createShadowResources(Device& device);

    /** @brief Creates all GBuffer textures, SRVs, and RTVs at the requested size. */
    HRESULT
    	createGBufferResources(Device& device, unsigned int width, unsigned int height);

    /** @brief Creates one typed GBuffer render target and matching SRV. */
    HRESULT
    	createGBufferTarget(Device& device,
                        unsigned int width,
                        unsigned int height,
                        DXGI_FORMAT format,
                        Texture& texture,
                        Texture& srv,
                        RenderTargetView& rtv);

    /** @brief Creates shaders/states needed by the fullscreen lighting pass. */
    HRESULT
    	createLightingResources(Device& device);

    /** @brief Creates fullscreen quad vertex and index buffers. */
    HRESULT
    	createFullScreenQuad(Device& device);

    /** @brief Creates blend states used by transparent material modes. */
    HRESULT
    	createBlendStates(Device& device);

    /** @brief Returns the blend state matching @p material blend settings. */
    ID3D11BlendState*
    	resolveBlendState(const Material* material) const;

    /** @brief Per-frame constant buffer. */
    Buffer m_perFrameBuffer;
    /** @brief Per-object constant buffer. */
    Buffer m_perObjectBuffer;
    /** @brief Per-material constant buffer. */
    Buffer m_perMaterialBuffer;
    /** @brief Lighting debug constant buffer. */
    Buffer m_lightingDebugBuffer;
    /** @brief Fullscreen quad vertex buffer for lighting. */
    Buffer m_fullscreenVertexBuffer;
    /** @brief Fullscreen quad index buffer for lighting. */
    Buffer m_fullscreenIndexBuffer;

    /** @brief Depth state used by transparent forward passes. */
    DepthStencilState m_transparentDepthStencil;
    /** @brief Depth-disabled state used by fullscreen lighting. */
    DepthStencilState m_disabledDepthStencil;
    /** @brief Depth state used by shadow rendering. */
    DepthStencilState m_shadowDepthStencil;

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
    /** @brief Shadow depth SRV wrapper. */
    Texture m_shadowDepthSRV;
    /** @brief Shadow depth-stencil view. */
    DepthStencilView m_shadowDSV;
    /** @brief Shadow pass shader program. */
    ShaderProgram m_shadowShader;
    /** @brief Shadow pass rasterizer state. */
    RasterizerState m_shadowRasterizer;
    /** @brief Square shadow-map resolution in pixels. */
    unsigned int m_shadowMapSize = 2048;

    /** @brief Shader used to write geometry attributes into the GBuffer. */
    ShaderProgram m_gBufferShader;
    /** @brief Shader used by the fullscreen deferred lighting pass. */
    ShaderProgram m_deferredLightingShader;
    /** @brief Sampler used when reading GBuffer/shadow textures. */
    SamplerState m_lightingSampler;
    /** @brief Rasterizer state used by fullscreen passes. */
    RasterizerState m_fullscreenRasterizer;

    /** @brief GBuffer albedo/metallic render texture. */
    Texture m_gBufferAlbedoMetallicTexture;
    /** @brief GBuffer albedo/metallic SRV wrapper. */
    Texture m_gBufferAlbedoMetallicSRV;
    /** @brief GBuffer albedo/metallic render-target view. */
    RenderTargetView m_gBufferAlbedoMetallicRTV;

    /** @brief GBuffer normal/roughness render texture. */
    Texture m_gBufferNormalRoughnessTexture;
    /** @brief GBuffer normal/roughness SRV wrapper. */
    Texture m_gBufferNormalRoughnessSRV;
    /** @brief GBuffer normal/roughness render-target view. */
    RenderTargetView m_gBufferNormalRoughnessRTV;

    /** @brief GBuffer world-position/AO render texture. */
    Texture m_gBufferWorldAoTexture;
    /** @brief GBuffer world-position/AO SRV wrapper. */
    Texture m_gBufferWorldAoSRV;
    /** @brief GBuffer world-position/AO render-target view. */
    RenderTargetView m_gBufferWorldAoRTV;

    /** @brief GBuffer emissive/alpha render texture. */
    Texture m_gBufferEmissiveAlphaTexture;
    /** @brief GBuffer emissive/alpha SRV wrapper. */
    Texture m_gBufferEmissiveAlphaSRV;
    /** @brief GBuffer emissive/alpha render-target view. */
    RenderTargetView m_gBufferEmissiveAlphaRTV;

    /** @brief Off-screen debug pass used before shadow application. */
    EditorViewportPass m_preShadowDebugPass;
    /** @brief Master flag controlling whether lighting samples the shadow map. */
    bool m_applyShadows = true;
    /** @brief Current deferred render width in pixels. */
    unsigned int m_renderWidth = 1280;
    /** @brief Current deferred render height in pixels. */
    unsigned int m_renderHeight = 720;

    /** @brief CPU-side per-frame constants. */
    CBPerFrame m_cbPerFrame{};
    /** @brief CPU-side per-object constants. */
    CBPerObject m_cbPerObject{};
    /** @brief CPU-side per-material constants. */
    CBPerMaterial m_cbPerMaterial{};
    /** @brief CPU-side debug data consumed by the deferred lighting shader. */
    struct
    	DeferredLightingDebugData {
        /** @brief Selected deferred debug output mode. */
        int DebugViewMode = 0;
        /** @brief Shadow contribution strength used by debug visualization. */
        float ShadowStrength = 1.0f;
        /** @brief Padding for constant-buffer alignment. */
        float pad0 = 0.0f;
        /** @brief Padding for constant-buffer alignment. */
        float pad1 = 0.0f;
    } m_lightingDebugData{};
    /** @brief True when shadow factor debug visualization is enabled. */
    bool m_shadowFactorDebugEnabled = false;
    /** @brief Current deferred debug view mode. */
    int m_deferredDebugViewMode = 0;

    /** @brief Opaque objects queued for GBuffer rendering. */
    std::vector<const RenderObject*> m_opaqueQueue;
    /** @brief Transparent objects queued for forward blended rendering. */
    std::vector<const RenderObject*> m_transparentQueue;
};
