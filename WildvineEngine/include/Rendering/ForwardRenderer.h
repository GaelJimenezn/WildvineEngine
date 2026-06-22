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
 * @brief Renderer basado en forward rendering que gestiona múltiples passes de render.
 *
 * Esta clase se encarga de:
 * - Generar sombras (shadow pass)
 * - Renderizar objetos opacos y transparentes
 * - Renderizar skybox
 * - Manejar buffers y estados gráficos necesarios
 */
class
ForwardRenderer {
public:
  /**
   * @brief Inicializa el renderer y sus recursos.
   * @param device Referencia al dispositivo gráfico.
   * @return HRESULT indicando éxito o fallo.
   */
  HRESULT init(Device& device);

  /**
   * @brief Ajusta el tamaño del render target.
   * @param device Dispositivo gráfico.
   * @param width Nuevo ancho.
   * @param height Nuevo alto.
   */
  void 
  resize(Device& device,
    unsigned int width,
    unsigned int height);

  /**
   * @brief Actualiza los datos por frame (buffers constantes).
   * @param camera Cámara activa.
   * @param scene Escena a renderizar.
   * @param deviceContext Contexto del dispositivo.
   */
  void 
  updatePerFrame(const Camera& camera,
    const RenderScene& scene,
    DeviceContext& deviceContext);

  /**
   * @brief Ejecuta el pipeline completo de render.
   * @param deviceContext Contexto del dispositivo.
   * @param camera Cámara activa.
   * @param scene Escena a renderizar.
   * @param viewportPass Pass de viewport del editor.
   */
  void 
  render(DeviceContext& deviceContext,
    const Camera& camera,
    RenderScene& scene,
    EditorViewportPass& viewportPass);

  /**
   * @brief Libera todos los recursos del renderer.
   */
  void 
  destroy();

  /**
   * @brief Obtiene el Shader Resource View del shadow map.
   * @return Puntero a ID3D11ShaderResourceView.
   */
  ID3D11ShaderResourceView* getShadowMapSRV() const { return m_shadowDepthSRV.m_textureFromImg; }

  /**
   * @brief Obtiene el SRV del pre-shadow debug pass.
   * @return Puntero a ID3D11ShaderResourceView.
   */
  ID3D11ShaderResourceView* getPreShadowSRV() const { return m_preShadowDebugPass.getSRV(); }

private:
  /**
   * @brief Construye las colas de render (opacos y transparentes).
   * @param scene Escena a procesar.
   * @param camera Cámara activa.
   */
  void
  buildQueues(RenderScene& scene, const Camera& camera);

  /**
   * @brief Renderiza el pass de debug previo a sombras.
   * @param deviceContext Contexto del dispositivo.
   * @param scene Escena.
   */
  void
  renderPreShadowDebugPass(DeviceContext& deviceContext, RenderScene& scene);

  /** @brief Renderiza el pass de sombras. */
  void
  renderShadowPass(DeviceContext& deviceContext);

  /** @brief Renderiza objetos opacos. */
  void
  renderOpaquePass(DeviceContext& deviceContext);

  /** @brief Renderiza objetos transparentes. */
  void 
  renderTransparentPass(DeviceContext& deviceContext);

  /**
   * @brief Renderiza el skybox.
   * @param deviceContext Contexto del dispositivo.
   * @param scene Escena.
   */
  void 
  renderSkyboxPass(DeviceContext& deviceContext, RenderScene& scene);

  /**
   * @brief Renderiza un objeto según el tipo de pass.
   * @param deviceContext Contexto del dispositivo.
   * @param object Objeto a renderizar.
   * @param passType Tipo de pass.
   */
  void 
  renderObject(DeviceContext& deviceContext, const RenderObject& object, RenderPassType passType);

  /**
   * @brief Renderiza un objeto en el pass de sombras.
   * @param deviceContext Contexto del dispositivo.
   * @param object Objeto a renderizar.
   */
  void 
  renderShadowObject(DeviceContext& deviceContext, const RenderObject& object);

  /**
   * @brief Crea los recursos necesarios para sombras.
   * @param device Dispositivo gráfico.
   * @return HRESULT indicando éxito o fallo.
   */
  HRESULT createShadowResources(Device& device);

  /**
   * @brief Actualiza las matrices de iluminación para sombras.
   * @param camera Cámara activa.
   * @param scene Escena.
   */
  void 
  updateLightMatrices(const Camera& camera, const RenderScene& scene);

  /**
   * @brief Crea estados de blending.
   * @param device Dispositivo gráfico.
   * @return HRESULT indicando éxito o fallo.
   */
  HRESULT createBlendStates(Device& device);

  /**
   * @brief Resuelve el estado de blending según el material.
   * @param material Material del objeto.
   * @return Puntero a ID3D11BlendState.
   */
  ID3D11BlendState* resolveBlendState(const Material* material) const;

private:
  /** @brief Buffer constante por frame. */
  Buffer m_perFrameBuffer;

  /** @brief Buffer constante por objeto. */
  Buffer m_perObjectBuffer;

  /** @brief Buffer constante por material. */
  Buffer m_perMaterialBuffer;

  /** @brief Estado de profundidad para transparencia. */
  DepthStencilState m_transparentDepthStencil;

  /** @brief Estados de blending. */
  ID3D11BlendState* m_alphaBlendState = nullptr;
  ID3D11BlendState* m_opaqueBlendState = nullptr;
  ID3D11BlendState* m_additiveBlendState = nullptr;
  ID3D11BlendState* m_premultipliedBlendState = nullptr;

  /** @brief Factor de mezcla. */
  float 
  m_blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

  /** @brief Textura de profundidad para sombras. */
  Texture m_shadowDepthTexture;

  /** @brief Shader Resource View del shadow map. */
  Texture m_shadowDepthSRV;

  /** @brief Depth Stencil View para sombras. */
  DepthStencilView m_shadowDSV;

  /** @brief Shader para el pass de sombras. */
  ShaderProgram m_shadowShader;

  /** @brief Rasterizer para sombras. */
  RasterizerState m_shadowRasterizer;

  /** @brief Tamaño del shadow map. */
  unsigned 
  int m_shadowMapSize = 2048;

  /** @brief Pass de debug previo a sombras. */
  EditorViewportPass m_preShadowDebugPass;

  /** @brief Indica si se aplican sombras. */
  bool 
  m_applyShadows = true;

  /** @brief Constant buffers CPU-side. */
  CBPerFrame m_cbPerFrame{};
  CBPerObject m_cbPerObject{};
  CBPerMaterial m_cbPerMaterial{};

  /** @brief Cola de objetos opacos. */
  std::vector<const RenderObject*> m_opaqueQueue;

  /** @brief Cola de objetos transparentes. */
  std::vector<const RenderObject*> m_transparentQueue;
};