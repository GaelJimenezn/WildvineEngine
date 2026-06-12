/**
*@file ISceneRenderer.h
* @brief Declara una interfaz comun para los renderers de escena.
* @ingroup rendering
*/
#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;
class Camera;
class RenderScene;
class EditorViewportPass;
//class Texture;

enum class
	RenderType {
  /** @brief Forward renderer implementation. */
  Forward = 0, 
  /** @brief Deferred renderer implementation. */
  Deferred = 1
};

/**
*@class ISceneRenderer
* @brief Contrato base para cualquier renderer consumido por el pipeline principal.
*/
class
	ISceneRenderer {
public:
  /** @brief Virtual destructor for polymorphic renderer cleanup. */
  virtual
  	~ISceneRenderer() = default;

  /** @brief Creates renderer-owned GPU resources. */
  virtual HRESULT
  	init(Device& device) = 0;

  /** @brief Recreates size-dependent renderer resources. */
  virtual void
  	resize(Device& device, unsigned int width, unsigned int height) = 0;

  /** @brief Renders @p scene from @p camera into @p viewport. */
  virtual void
  	render(DeviceContext& deviceContext,
    const Camera& camera, 
    RenderScene& scene, 
    EditorViewportPass& viewport) = 0;

  /** @brief Releases renderer-owned GPU resources. */
  virtual void
  	destroy() = 0;

  /** @brief Returns a shadow-map SRV when the renderer exposes one. */
  virtual ID3D11ShaderResourceView*
  	getShadowMapSRV() const { return nullptr; }

  /** @brief Returns a pre-shadow/debug SRV when the renderer exposes one. */
  virtual ID3D11ShaderResourceView*
  	getPreShadowSRV() const { return nullptr; }

  /** @brief Returns the GBuffer albedo/metallic SRV for deferred debug UI. */
  virtual ID3D11ShaderResourceView*
  	getGBufferAlbedoMetallicSRV() const { return nullptr; }

  /** @brief Returns the GBuffer normal/roughness SRV for deferred debug UI. */
  virtual ID3D11ShaderResourceView*
  	getGBufferNormalRoughnessSRV() const { return nullptr; }

  /** @brief Returns the GBuffer world-position/AO SRV for deferred debug UI. */
  virtual ID3D11ShaderResourceView*
  	getGBufferWorldAoSRV() const { return nullptr; }

  /** @brief Returns the GBuffer emissive/alpha SRV for deferred debug UI. */
  virtual ID3D11ShaderResourceView*
  	getGBufferEmissiveAlphaSRV() const { return nullptr; }

  /**
   * @brief Enables or disables shadow-factor visualization in renderers that support it.
   */
  virtual void
  	setShaderFactorDebugEnabled(bool enabled) { (void)enabled; }

  /** @brief Selects deferred debug view mode in renderers that support it. */
  virtual void
  	setDeferredDebugViewMode(int mode) { (void)mode; }

  /** @brief Returns a stable renderer name for editor/debug UI. */
  virtual const char*
  	getDebugName() const = 0;
};
