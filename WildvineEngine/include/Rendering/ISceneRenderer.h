/**
 * @file ISceneRenderer.h
 * @brief Declara una interfaz comÃºn para los renderers de escena.
 * @ingroup rendering
 */
#pragma once
#include "Prerequisites.h"

/** @brief Declara class Device. */
class Device;
/** @brief Declara class DeviceContext. */
class DeviceContext;
/** @brief Declara class Camera. */
class Camera;
/** @brief Declara class RenderScene. */
class RenderScene;
/** @brief Declara class EditorViewportPass. */
class EditorViewportPass;
// class Texture;

/**
 * @brief Tipos de renderizado soportados por el motor.
 */
enum class
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
 * Esta interfaz define las operaciones mÃ­nimas que debe implementar
 * cualquier sistema de renderizado de escena dentro del motor.
 *
 * Entre sus responsabilidades se encuentran:
 * - InicializaciÃ³n de recursos grÃ¡ficos.
 * - GestiÃ³n de cambios de resoluciÃ³n.
 * - Renderizado de la escena.
 * - LiberaciÃ³n de recursos.
 * - ExposiciÃ³n de recursos de depuraciÃ³n cuando corresponda.
 *
 * Implementaciones tÃ­picas:
 * - ForwardRenderer
 * - DeferredRenderer
 */
class ISceneRenderer {
public:
  /**
   * @brief Destructor virtual por defecto.
   */
  virtual ~ISceneRenderer() = default;

  /**
   * @brief Inicializa los recursos necesarios para el renderer.
   *
   * @param device Dispositivo grÃ¡fico utilizado para crear recursos.
   * @return HRESULT Resultado de la operaciÃ³n.
   */
  virtual HRESULT init(Device &device) = 0;

  /**
   * @brief Reconfigura los recursos dependientes de la resoluciÃ³n.
   *
   * @param device Dispositivo grÃ¡fico.
   * @param width Nuevo ancho de renderizado.
   * @param height Nueva altura de renderizado.
   */
  virtual void resize(Device &device, unsigned int width, unsigned int height) = 0;

  /**
   * @brief Ejecuta el proceso de renderizado de una escena.
   *
   * @param deviceContext Contexto de dispositivo utilizado para emitir comandos.
   * @param camera CÃ¡mara activa de la escena.
   * @param scene Escena a renderizar.
   * @param viewport Render target de salida.
   */
  virtual void render(DeviceContext &deviceContext,

                      const Camera &camera,
                      RenderScene &scene,
                      EditorViewportPass &viewport) = 0;

  /**
   * @brief Libera todos los recursos utilizados por el renderer.
   */
  virtual void destroy() = 0;

  /**
   * @brief Obtiene el Shader Resource View del Shadow Map.
   *
   * @return SRV del Shadow Map o nullptr si no estÃ¡ soportado.
   */
  virtual ID3D11ShaderResourceView *
  getShadowMapSRV() const {
    return nullptr;
  }

  /**
   * @brief Obtiene el SRV utilizado para depuraciÃ³n previa al pase de sombras.
   *
   * @return Shader Resource View o nullptr si no estÃ¡ disponible.
   */
  virtual ID3D11ShaderResourceView *
  getPreShadowSRV() const {
    return nullptr;
  }

  /**
   * @brief Obtiene el SRV del buffer Albedo/Metallic.
   *
   * @return Shader Resource View o nullptr si no aplica.
   */
  virtual ID3D11ShaderResourceView *
  getGBufferAlbedoMetallicSRV() const {
    return nullptr;
  }

  /**
   * @brief Obtiene el SRV del buffer Normal/Roughness.
   *
   * @return Shader Resource View o nullptr si no aplica.
   */
  virtual ID3D11ShaderResourceView *
  getGBufferNormalRoughnessSRV() const {
    return nullptr;
  }

  /**
   * @brief Obtiene el SRV del buffer World Position/Ambient Occlusion.
   *
   * @return Shader Resource View o nullptr si no aplica.
   */
  virtual ID3D11ShaderResourceView *
  getGBufferWorldAoSRV() const {
    return nullptr;
  }

  /**
   * @brief Obtiene el SRV del buffer Emissive/Alpha.
   *
   * @return Shader Resource View o nullptr si no aplica.
   */
  virtual ID3D11ShaderResourceView *
  getGBufferEmissiveAlphaSRV() const {
    return nullptr;
  }

  /**
   * @brief Activa o desactiva la visualizaciÃ³n del factor de sombras.
   *
   * Implementaciones que no soporten esta caracterÃ­stica
   * pueden ignorar el parÃ¡metro.
   *
   * @param enabled Estado deseado.
   */
  virtual void
  setShadowFactorDebugEnabled(bool enabled) {
    (void)enabled;
  }

  /**
   * @brief Configura el modo de visualizaciÃ³n de depuraciÃ³n Deferred.
   *
   * Implementaciones que no utilicen Deferred Rendering
   * pueden ignorar este parÃ¡metro.
   *
   * @param mode Modo de depuraciÃ³n.
   */
  virtual void
  setDeferredDebugViewMode(int mode) {
    (void)mode;
  }

  /**
   * @brief Obtiene el nombre descriptivo del renderer.
   *
   * Utilizado principalmente para depuraciÃ³n y herramientas de editor.
   *
   * @return Nombre del renderer.
   */
  virtual
      /** @brief Declara o ejecuta getDebugName. */
      const char *
      getDebugName() const = 0;
};
