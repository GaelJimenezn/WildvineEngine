/**
 * @file ISceneRenderer.h
 * @brief Declara una interfaz común para los renderers de escena.
 * @ingroup rendering
 */
#pragma once
#include "Prerequisites.h"

/** @brief Declara class Device. */
class
Device;
/** @brief Declara class DeviceContext. */
class
DeviceContext;
/** @brief Declara class Camera. */
class
Camera;
/** @brief Declara class RenderScene. */
class
RenderScene;
/** @brief Declara class EditorViewportPass. */
class
EditorViewportPass;
//class Texture;

/**
 * @enum RenderType
 * @brief Tipos de renderizado soportados por el motor.
 */
enum
/** @brief Declara class RenderType. */
class
  RenderType {
  /**
   * @brief Renderizado Forward tradicional.
   */
  Forward = 0,

  /**
   * @brief Renderizado Deferred basado en G-Buffer.
   */
  Deferred = 1
};

/**
 * @class ISceneRenderer
 * @brief Contrato base para cualquier renderer consumido por el pipeline principal.
 *
 * Esta interfaz define las operaciones mínimas que debe implementar
 * cualquier sistema de renderizado de escena dentro del motor.
 *
 * Entre sus responsabilidades se encuentran:
 * - Inicialización de recursos gráficos.
 * - Gestión de cambios de resolución.
 * - Renderizado de la escena.
 * - Liberación de recursos.
 * - Exposición de recursos de depuración cuando corresponda.
 *
 * Implementaciones típicas:
 * - ForwardRenderer
 * - DeferredRenderer
 */
class
ISceneRenderer {
public:

  /**
   * @brief Destructor virtual por defecto.
   */
  virtual
  ~ISceneRenderer() = default;

  /**
   * @brief Inicializa los recursos necesarios para el renderer.
   *
   * @param device Dispositivo gráfico utilizado para crear recursos.
   * @return HRESULT Resultado de la operación.
   */
  virtual HRESULT
  init(Device& device) = 0;

  /**
   * @brief Reconfigura los recursos dependientes de la resolución.
   *
   * @param device Dispositivo gráfico.
   * @param width Nuevo ancho de renderizado.
   * @param height Nueva altura de renderizado.
   */
  virtual void
  resize(Device& device, unsigned int width, unsigned int height) = 0;

  /**
   * @brief Ejecuta el proceso de renderizado de una escena.
   *
   * @param deviceContext Contexto de dispositivo utilizado para emitir comandos.
   * @param camera Cámara activa de la escena.
   * @param scene Escena a renderizar.
   * @param viewport Render target de salida.
   */
  virtual void 
  render(DeviceContext& deviceContext,

    const Camera& camera,
    RenderScene& scene,
    EditorViewportPass& viewport) = 0;

  /**
   * @brief Libera todos los recursos utilizados por el renderer.
   */
  virtual void
  destroy() = 0;

  /**
   * @brief Obtiene el Shader Resource View del Shadow Map.
   *
   * @return SRV del Shadow Map o nullptr si no está soportado.
   */
  virtual ID3D11ShaderResourceView*
  getShadowMapSRV() const { return nullptr; }

  /**
   * @brief Obtiene el SRV utilizado para depuración previa al pase de sombras.
   *
   * @return Shader Resource View o nullptr si no está disponible.
   */
  virtual ID3D11ShaderResourceView*
  getPreShadowSRV() const { return nullptr; }

  /**
   * @brief Obtiene el SRV del buffer Albedo/Metallic.
   *
   * @return Shader Resource View o nullptr si no aplica.
   */
  virtual ID3D11ShaderResourceView*
  getGBufferAlbedoMetallicSRV() const { return nullptr; }

  /**
   * @brief Obtiene el SRV del buffer Normal/Roughness.
   *
   * @return Shader Resource View o nullptr si no aplica.
   */
  virtual ID3D11ShaderResourceView*
  getGBufferNormalRoughnessSRV() const { return nullptr; }

  /**
   * @brief Obtiene el SRV del buffer World Position/Ambient Occlusion.
   *
   * @return Shader Resource View o nullptr si no aplica.
   */
  virtual ID3D11ShaderResourceView*
  getGBufferWorldAoSRV() const { return nullptr; }

  /**
   * @brief Obtiene el SRV del buffer Emissive/Alpha.
   *
   * @return Shader Resource View o nullptr si no aplica.
   */
  virtual ID3D11ShaderResourceView*
  getGBufferEmissiveAlphaSRV() const { return nullptr; }

  /**
   * @brief Activa o desactiva la visualización del factor de sombras.
   *
   * Implementaciones que no soporten esta característica
   * pueden ignorar el parámetro.
   *
   * @param enabled Estado deseado.
   */
  virtual void
  setShadowFactorDebugEnabled(bool enabled) { (void)enabled; }

  /**
   * @brief Configura el modo de visualización de depuración Deferred.
   *
   * Implementaciones que no utilicen Deferred Rendering
   * pueden ignorar este parámetro.
   *
   * @param mode Modo de depuración.
   */
  virtual void
  setDeferredDebugViewMode(int mode) { (void)mode; }

  /**
   * @brief Obtiene el nombre descriptivo del renderer.
   *
   * Utilizado principalmente para depuración y herramientas de editor.
   *
   * @return Nombre del renderer.
   */
  virtual 
  /** @brief Declara o ejecuta getDebugName. */
  const char*
  getDebugName() const = 0;
};
