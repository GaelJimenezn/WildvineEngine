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
  Forward = 0, 
  Deferred = 1
};

/**
*@class ISceneRenderer
* @brief Contrato base para cualquier renderer consumido por el pipeline principal.
*/
class
ISceneRenderer {
public:
  virtual ~ISceneRenderer() = default;

  virtual HRESULT init(Device& device) = 0;
  virtual void resize(Device& device, unsigned int width, unsigned int height) = 0;
  virtual void render(DeviceContext& deviceContext, 
    const Camera& camera, 
    RenderScene& scene, 
    EditorViewportPass& viewport) = 0;
  virtual void destroy() = 0;

  virtual 
  ID3D11ShaderResourceView* getShadowMapSRV() const { return nullptr; }

  virtual 
  ID3D11ShaderResourceView* getPreShadowSRV() const { return nullptr; }

  virtual 
  ID3D11ShaderResourceView* getGBufferAlbedoMetallicSRV() const { return nullptr; }

  virtual 
  ID3D11ShaderResourceView* getGBufferNormalRoughnessSRV() const { return nullptr; }

  virtual 
  ID3D11ShaderResourceView* getGBufferWorldAoSRV() const { return nullptr; }

  virtual 
  ID3D11ShaderResourceView* getGBufferEmissiveAlphaSRV() const { return nullptr; }

  virtual void 
  setShaderFactorDebugEnabled(bool enabled) { (void)enabled; }

  virtual void 
  setDeferredDebugViewMode(int mode) { (void)mode; }

  virtual 
  const char* getDebugName() const = 0;
};