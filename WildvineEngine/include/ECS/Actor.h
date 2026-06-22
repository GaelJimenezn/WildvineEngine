#pragma once
#include "Prerequisites.h"
#include "Entity.h"
#include "Buffer.h"
#include "Texture.h"
#include "Transform.h" 
#include "SamplerState.h"
#include "RasterizerState.h"
#include "ShaderProgram.h"
#include "DepthStencilState.h"

class Device;

class DeviceContext;

class MeshComponent;

/**
 * @class Actor
 * @brief Representa una entidad dentro de la escena que posee mallas, texturas y estado de renderizado.
 *
 * La clase Actor hereda de Entity y agrega funcionalidades completas para renderizado,
 * aplicación de texturas, buffers, transformaciones y sombreado.
 */
class 
Actor : public Entity {
public:

  /**
   * @brief Constructor por defecto.
   */
  Actor() = default;

  /**
   * @brief Constructor que inicializa un actor con un dispositivo gráfico.
   * @param device Referencia al dispositivo gráfico.
   */
  Actor(Device& device);

  /**
   * @brief Destructor virtual por defecto.
   */
  virtual 
  ~Actor() = default;

  void 
  awake() override{}

  /**
   * @brief Inicializa el actor. Implementación vacía.
   */
  void
  init() override {}

  /**
   * @brief Actualiza la lógica del actor por frame.
   * @param deltaTime Tiempo transcurrido entre frames.
   * @param deviceContext Contexto del dispositivo para actualizaciones gráficas.
   */
  void
  update(float deltaTime, DeviceContext& deviceContext) override;

  /**
   * @brief Renderiza el actor.
   * @param deviceContext Contexto del dispositivo usado para el render.
   */
  void
  render(DeviceContext& deviceContext) override;


  void
  renderForSkybox(DeviceContext& deviceContext);


  /**
   * @brief Libera los recursos asociados al actor.
   */
  void
  destroy();

  /**
   * @brief Asigna las mallas que componen al actor.
   * @param device Dispositivo gráfico asociado.
   * @param meshes Conjunto de componentes MeshComponent.
   */
  void
  setMesh(Device& device, std::vector<MeshComponent> meshes);

  /**
   * @brief Obtiene el nombre del actor.
   * @return Nombre como cadena de texto.
   */
  std::string 
  getName() { return m_name; }

  /**
   * @brief Establece el nombre del actor.
   * @param name Nuevo nombre.
   */
  void
  setName(const std::string& name) { m_name = name; }

  /**
   * @brief Asigna texturas al actor.
   * @param textures Vector de texturas.
   */
  void
  setTextures(std::vector<Texture> textures) { m_textures = textures; }

  /**
   * @brief Define si el actor puede proyectar sombras.
   * @param v Valor booleano.
   */
  void
  setCastShadow(bool v) { castShadow = v; }

  /**
   * @brief Indica si el actor está configurado para proyectar sombras.
   * @return true si proyecta sombras, false en caso contrario.
   */
  bool
  canCastShadow() const { return castShadow; }

  /**
   * @brief Renderiza únicamente la sombra del actor.
   * @param deviceContext Contexto del dispositivo utilizado para dibujar la sombra.
   */
  void
  renderShadow(DeviceContext& deviceContext);

private:
  std::vector<MeshComponent> m_meshes;      ///< Conjunto de componentes de malla del actor.
  std::vector<Texture> m_textures;          ///< Texturas aplicadas al actor.
  std::vector<Buffer> m_vertexBuffers;      ///< Buffers de vértices asociados a las mallas.
  std::vector<Buffer> m_indexBuffers;       ///< Buffers de índices asociados a las mallas.

  //BlendState m_blendState;                // Estado de blending usado por el actor.
  //RasterizerState m_rasterizer;                // Estado de rasterización usado por el actor.
  SamplerState m_sampler;                   ///< Estado de muestreo de texturas.
  CBChangesEveryFrame m_model;              ///< Constant buffer con las transformaciones por frame.
  Buffer m_modelBuffer;                     ///< Buffer que contiene @c m_model.

  // Recursos para sombras
  ShaderProgram m_shaderShadow;             ///< Shader program utilizado para el renderizado de sombras.
  Buffer m_shaderBuffer;                    ///< Buffer auxiliar para datos de sombras.
  //BlendState m_shadowBlendState;          // Estado de blending específico para sombras.
  DepthStencilState m_shadowDepthStencilState; // Estado de profundidad/estencil para sombras.
  CBChangesEveryFrame m_cbShadow;           ///< Constant buffer exclusivo para sombreado.

  XMFLOAT4 m_LightPos;                      ///< Posición de la luz para proyección de sombras.
  std::string m_name = "Actor";             ///< Nombre identificador del actor.
  bool castShadow = true;                   ///< Indica si el actor proyecta sombras.
};
