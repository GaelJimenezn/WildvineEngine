/**
 * @file Actor.h
 * @brief Declara la API de Actor dentro del sistema de entidades.
 */
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

/** @brief Declara class Device. */
class Device;

/** @brief Declara class DeviceContext. */
class DeviceContext;

/** @brief Declara class MeshComponent. */
class MeshComponent;

/**
 * @class Actor
 * @brief Representa una entidad dentro de la escena que posee mallas, texturas y estado
 * de renderizado.
 *
 * La clase Actor hereda de Entity y agrega funcionalidades completas para renderizado,
 * aplicaciÃ³n de texturas, buffers, transformaciones y sombreado.
 */
class Actor : public Entity {
public:
  /**
   * @brief Constructor por defecto.
   */
  Actor() = default;

  /**
   * @brief Constructor que inicializa un actor con un dispositivo grÃ¡fico.
   * @param device Referencia al dispositivo grÃ¡fico.
   */
  Actor(Device &device);

  /**
   * @brief Destructor virtual por defecto.
   */
  virtual ~Actor() = default;

  /** @brief Declara o ejecuta awake. */
  void
  awake() override {
  }

  /**
   * @brief Inicializa el actor. ImplementaciÃ³n vacÃ­a.
   */
  void
  init() override {
  }

  /**
   * @brief Actualiza la lÃ³gica del actor por frame.
   * @param deltaTime Tiempo transcurrido entre frames.
   * @param deviceContext Contexto del dispositivo para actualizaciones grÃ¡ficas.
   */
  void update(float deltaTime, DeviceContext &deviceContext) override;

  /**
   * @brief Renderiza el actor.
   * @param deviceContext Contexto del dispositivo usado para el render.
   */
  void render(DeviceContext &deviceContext) override;

  /** @brief Declara o ejecuta renderForSkybox. */
  void renderForSkybox(DeviceContext &deviceContext);

  /**
   * @brief Libera los recursos asociados al actor.
   */
  void destroy();

  /**
   * @brief Asigna las mallas que componen al actor.
   * @param device Dispositivo grÃ¡fico asociado.
   * @param meshes Conjunto de componentes MeshComponent.
   */
  void setMesh(Device &device, std::vector<MeshComponent> meshes);

  /**
   * @brief Obtiene el nombre del actor.
   * @return Nombre como cadena de texto.
   */
  std::string
  getName() {
    return m_name;
  }

  /**
   * @brief Establece el nombre del actor.
   * @param name Nuevo nombre.
   */
  void
  setName(const std::string &name) {
    m_name = name;
  }

  /**
   * @brief Asigna texturas al actor.
   * @param textures Vector de texturas.
   */
  void
  setTextures(std::vector<Texture> textures) {
    m_textures = textures;
  }

  /**
   * @brief Define si el actor puede proyectar sombras.
   * @param v Valor booleano.
   */
  void
  setCastShadow(bool v) {
    castShadow = v;
  }

  /**
   * @brief Indica si el actor estÃ¡ configurado para proyectar sombras.
   * @return true si proyecta sombras, false en caso contrario.
   */
  bool
  canCastShadow() const {
    return castShadow;
  }

  /**
   * @brief Renderiza Ãºnicamente la sombra del actor.
   * @param deviceContext Contexto del dispositivo utilizado para dibujar la sombra.
   */
  void renderShadow(DeviceContext &deviceContext);

private:
  /** @brief Conjunto de componentes de malla del actor. */
  std::vector<MeshComponent> m_meshes;
  std::vector<Texture> m_textures; ///< Texturas aplicadas al actor.
  /** @brief Buffers de vÃ©rtices asociados a las mallas. */
  std::vector<Buffer> m_vertexBuffers;
  /** @brief Buffers de Ã­ndices asociados a las mallas. */
  std::vector<Buffer> m_indexBuffers;

  // BlendState m_blendState;                // Estado de blending usado por el actor.
  // RasterizerState m_rasterizer;
  //  Estado de rasterizaciÃ³n usado por el actor.
  SamplerState m_sampler; ///< Estado de muestreo de texturas.
  /** @brief Constant buffer con las transformaciones por frame. */
  CBChangesEveryFrame m_model;
  Buffer m_modelBuffer; ///< Buffer que contiene @c m_model.

  // Recursos para sombras
  /** @brief Shader program utilizado para el renderizado de sombras. */
  ShaderProgram m_shaderShadow;
  Buffer m_shaderBuffer; ///< Buffer auxiliar para datos de sombras.
  // BlendState m_shadowBlendState;          // Estado de blending especÃ­fico para
  // sombras.
  /** @brief Estado de profundidad/estencil para sombras. */
  DepthStencilState m_shadowDepthStencilState;
  CBChangesEveryFrame m_cbShadow; ///< Constant buffer exclusivo para sombreado.

  /** @brief PosiciÃ³n de la luz para proyecciÃ³n de sombras. */
  XMFLOAT4 m_LightPos;
  std::string m_name = "Actor"; ///< Nombre identificador del actor.
  bool castShadow = true;       ///< Indica si el actor proyecta sombras.
};
