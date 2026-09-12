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

/** @brief Declara class Device. */
class Device;
/** @brief Declara class DeviceContext. */
class DeviceContext;
/** @brief Declara class Camera. */
class Camera;
/** @brief Declara class Material. */
class Material;

/**
 * @class DeferredRenderer
 * @brief Implementa un pipeline diferido con GBuffer y lighting pass.
 *
 * El renderer usa deferred shading para superficies opacas y mantiene un subpass
 * forward para transparencias, de modo que el pipeline del editor siga funcionando
 * con el contenido actual del engine.
 */
class DeferredRenderer : public ISceneRenderer {
public:
  /** @brief Declara o ejecuta init. */
  HRESULT
  init(Device &device) override;

  /** @brief Declara o ejecuta resize. */
  void resize(Device &device, unsigned int width, unsigned int height) override;

  /** @brief Declara o ejecuta render. */
  void render(DeviceContext &deviceContext,
              const Camera &camera,
              RenderScene &scene,
              EditorViewportPass &viewportPass) override;

  /** @brief Declara o ejecuta destroy. */
  void destroy() override;

  /** @brief Declara o ejecuta getShadowMapSRV. */
  ID3D11ShaderResourceView *
  getShadowMapSRV() const override {
    return m_shadowDepthSRV.m_textureFromImg;
  }

  /** @brief Declara o ejecuta getPreShadowSRV. */
  ID3D11ShaderResourceView *
  getPreShadowSRV() const override {
    return m_preShadowDebugPass.getSRV();
  }

  /** @brief Declara o ejecuta getGBufferAlbedoMetallicSRV. */
  ID3D11ShaderResourceView *
  getGBufferAlbedoMetallicSRV() const override {
    return m_gBufferAlbedoMetallicSRV.m_textureFromImg;
  }

  /** @brief Declara o ejecuta getGBufferNormalRoughnessSRV. */
  ID3D11ShaderResourceView *
  getGBufferNormalRoughnessSRV() const override {
    return m_gBufferNormalRoughnessSRV.m_textureFromImg;
  }

  /** @brief Declara o ejecuta getGBufferWorldAoSRV. */
  ID3D11ShaderResourceView *
  getGBufferWorldAoSRV() const override {
    return m_gBufferWorldAoSRV.m_textureFromImg;
  }

  /** @brief Declara o ejecuta getGBufferEmissiveAlphaSRV. */
  ID3D11ShaderResourceView *
  getGBufferEmissiveAlphaSRV() const override {
    return m_gBufferEmissiveAlphaSRV.m_textureFromImg;
  }

  /** @brief Declara o ejecuta setShadowFactorDebugEnabled. */
  void
  setShadowFactorDebugEnabled(bool enabled) override {
    m_shadowFactorDebugEnabled = enabled;
  }

  /** @brief Declara o ejecuta setDeferredDebugViewMode. */
  void
  setDeferredDebugViewMode(int mode) override {
    m_deferredDebugViewMode = mode;
  }

  /** @brief Declara o ejecuta getDebugName. */
  const char *
  getDebugName() const override {
    return "DeferredRenderer";
  }

private:
  /** @brief Declara o ejecuta buildQueues. */
  void buildQueues(RenderScene &scene, const Camera &camera);
  /** @brief Declara o ejecuta updatePerFrame. */
  void updatePerFrame(const Camera &camera,
                      const RenderScene &scene,
                      DeviceContext &deviceContext);
  /** @brief Declara o ejecuta updateLightMatrices. */
  void updateLightMatrices(const Camera &camera, const RenderScene &scene);
  /** @brief Declara o ejecuta renderSceneToTarget. */
  void renderSceneToTarget(DeviceContext &deviceContext,
                           RenderScene &scene,
                           EditorViewportPass &targetPass,
                           bool applyShadows);
  /** @brief Declara o ejecuta bindGBufferTargets. */
  void bindGBufferTargets(DeviceContext &deviceContext,
                          ID3D11DepthStencilView *depthStencilView);
  /** @brief Declara o ejecuta bindFinalTarget. */
  void bindFinalTarget(DeviceContext &deviceContext,
                       ID3D11RenderTargetView *renderTargetView,
                       ID3D11DepthStencilView *depthStencilView);
  /** @brief Declara o ejecuta clearDeferredSRVs. */
  void clearDeferredSRVs(DeviceContext &deviceContext);
  /** @brief Declara o ejecuta renderGeometryPass. */
  void renderGeometryPass(DeviceContext &deviceContext);
  /** @brief Declara o ejecuta renderGeometryObject. */
  void renderGeometryObject(DeviceContext &deviceContext, const RenderObject &object);
  /** @brief Declara o ejecuta renderLightingPass. */
  void renderLightingPass(DeviceContext &deviceContext);
  /** @brief Declara o ejecuta renderSkyboxPass. */
  void renderSkyboxPass(DeviceContext &deviceContext, RenderScene &scene);
  /** @brief Declara o ejecuta renderTransparentPass. */
  void renderTransparentPass(DeviceContext &deviceContext);
  /** @brief Declara o ejecuta renderForwardObject. */
  void renderForwardObject(DeviceContext &deviceContext,
                           const RenderObject &object,
                           RenderPassType passType);
  /** @brief Declara o ejecuta renderShadowPass. */
  void renderShadowPass(DeviceContext &deviceContext);
  /** @brief Declara o ejecuta renderShadowObject. */
  void renderShadowObject(DeviceContext &deviceContext, const RenderObject &object);
  /** @brief Declara o ejecuta createShadowResources. */
  HRESULT
  createShadowResources(Device &device);
  /** @brief Declara o ejecuta createGBufferResources. */
  HRESULT
  createGBufferResources(Device &device, unsigned int width, unsigned int height);
  /** @brief Declara o ejecuta createGBufferTarget. */
  HRESULT
  createGBufferTarget(Device &device,
                      unsigned int width,
                      unsigned int height,
                      DXGI_FORMAT format,
                      Texture &texture,
                      Texture &srv,
                      RenderTargetView &rtv);
  /** @brief Declara o ejecuta createLightingResources. */
  HRESULT
  createLightingResources(Device &device);
  /** @brief Declara o ejecuta createFullScreenQuad. */
  HRESULT
  createFullScreenQuad(Device &device);
  /** @brief Declara o ejecuta createBlendStates. */
  HRESULT
  createBlendStates(Device &device);
  /** @brief Declara o ejecuta resolveBlendState. */
  ID3D11BlendState *resolveBlendState(const Material *material) const;

private:
  Buffer m_perFrameBuffer;
  Buffer m_perObjectBuffer;
  Buffer m_perMaterialBuffer;
  Buffer m_lightingDebugBuffer;
  Buffer m_fullscreenVertexBuffer;
  Buffer m_fullscreenIndexBuffer;

  DepthStencilState m_transparentDepthStencil;
  DepthStencilState m_disabledDepthStencil;
  DepthStencilState m_shadowDepthStencil;

  ID3D11BlendState *m_alphaBlendState = nullptr;
  ID3D11BlendState *m_opaqueBlendState = nullptr;
  ID3D11BlendState *m_additiveBlendState = nullptr;
  ID3D11BlendState *m_premultipliedBlendState = nullptr;
  float m_blendFactor[4] = {0.0f, 0.0f, 0.0f, 0.0f};

  Texture m_shadowDepthTexture;
  Texture m_shadowDepthSRV;
  DepthStencilView m_shadowDSV;
  ShaderProgram m_shadowShader;
  RasterizerState m_shadowRasterizer;
  unsigned int m_shadowMapSize = 2048;

  ShaderProgram m_gBufferShader;
  ShaderProgram m_deferredLightingShader;
  SamplerState m_lightingSampler;
  RasterizerState m_fullscreenRasterizer;

  Texture m_gBufferAlbedoMetallicTexture;
  Texture m_gBufferAlbedoMetallicSRV;
  RenderTargetView m_gBufferAlbedoMetallicRTV;

  Texture m_gBufferNormalRoughnessTexture;
  Texture m_gBufferNormalRoughnessSRV;
  RenderTargetView m_gBufferNormalRoughnessRTV;

  Texture m_gBufferWorldAoTexture;
  Texture m_gBufferWorldAoSRV;
  RenderTargetView m_gBufferWorldAoRTV;

  Texture m_gBufferEmissiveAlphaTexture;
  Texture m_gBufferEmissiveAlphaSRV;
  RenderTargetView m_gBufferEmissiveAlphaRTV;

  EditorViewportPass m_preShadowDebugPass;
  bool m_applyShadows = true;
  unsigned int m_renderWidth = 1280;
  unsigned int m_renderHeight = 720;

  CBPerFrame m_cbPerFrame{};
  CBPerObject m_cbPerObject{};
  CBPerMaterial m_cbPerMaterial{};
  /** @brief Declara struct DeferredLightingDebugData. */
  struct DeferredLightingDebugData {
    int DebugViewMode = 0;
    float ShadowStrength = 1.0f;
    float pad0 = 0.0f;
    float pad1 = 0.0f;
  } m_lightingDebugData{};
  bool m_shadowFactorDebugEnabled = false;
  int m_deferredDebugViewMode = 0;

  std::vector<const RenderObject *> m_opaqueQueue;
  std::vector<const RenderObject *> m_transparentQueue;
};
