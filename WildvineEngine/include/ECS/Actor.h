#pragma once
#include "Prerequisites.h"
#include "Entity.h"
#include "Buffer.h"
#include "Texture.h"
#include "Transform.h"
#include "SamplerState.h"
#include "RasterizerState.h"
//#include "BlendState.h"
#include "ShaderProgram.h"
#include "DepthStencilState.h"

class Device;
class DeviceContext;
class MeshComponent;

/**
 * @class Actor
 * @brief Representa una entidad gráfica con mallas, texturas y estados de renderizado.
 *
 * Un Actor es una entidad del motor que contiene mallas, texturas y recursos de
 * renderizado
 * necesarios para dibujar un objeto en la escena.
 * Administra buffers de vértices e índices, estados de rasterización, blending y shaders,
 * además de soportar renderizado de sombras.
 */
class
	Actor : public Entity {
public:
	/**
	 * @brief Constructor por defecto.
	 */
	Actor() = default;

	/**
	 * @brief Constructor que inicializa el actor con un dispositivo.
	 * @param device Dispositivo usado para inicializar recursos del actor.
	 */
	Actor(Device& device);

	/**
	 * @brief Destructor virtual por defecto.
	 */
	virtual
		~Actor() = default;

	/** @brief Actor early setup hook; currently no additional work is required. */
	void
		awake() override {}

	/**
	 * @brief Inicializa el actor.
	 *
	 * Método heredado de @c Entity.
	 * Puede usarse para inicializar recursos adicionales en clases derivadas.
	 */
	void
		init() override {}

	/**
	 * @brief Actualiza el actor en cada frame.
	 *
	 * @param deltaTime     Tiempo transcurrido desde la última actualización.
	 * @param deviceContext Contexto del dispositivo para operaciones gráficas.
	 *
	 * @note Este método puede actualizar animaciones, transformaciones u otros recursos
	 * dependientes del tiempo.
	 */
	void
		update(float deltaTime, DeviceContext& deviceContext) override;

	/**
	 * @brief Renderiza el actor en la escena.
	 *
	 * Configura estados de render, buffers y shaders antes de dibujar las mallas asociadas
	 * al actor.
	 *
	 * @param deviceContext Contexto del dispositivo para operaciones gráficas.
	 */
	void
		render(DeviceContext& deviceContext) override;

	/** @brief Renders actor geometry with skybox-specific state and shaders. */
	void
		renderForSkybox(DeviceContext& deviceContext);

	/**
	 * @brief Libera todos los recursos asociados al actor.
	 *
	 * Incluye buffers, estados, shaders y texturas.
	 */
	void
		destroy();

	/**
	 * @brief Establece las mallas del actor.
	 *
	 * Inicializa buffers de vértices e índices asociados a las mallas.
	 *
	 * @param device Dispositivo con el cual se inicializan las mallas.
	 * @param meshes Vector de componentes de malla que se asignarán al actor.
	 */
	void
		setMesh(Device& device, std::vector<MeshComponent> meshes);

	/**
	 * @brief Obtiene el nombre del actor.
	 * @return Nombre actual del actor.
	 */
	std::string
		getName() { return m_name; }

	/**
	 * @brief Establece el nombre del actor.
	 * @param name Nuevo nombre para el actor.
	 */
	void
		setName(const std::string& name) { m_name = name; }

	/**
	 * @brief Establece las texturas del actor.
	 * @param textures Vector de texturas a asignar al actor.
	 */
	void
		setTextures(std::vector<Texture> textures) { m_textures = textures; }

	/**
	 * @brief Define si el actor proyecta sombras.
	 * @param v Valor booleano que habilita o deshabilita las sombras.
	 */
	void
		setCastShadow(bool v) { castShadow = v; }

	/**
	 * @brief Indica si el actor puede proyectar sombras.
	 * @return @c true si el actor proyecta sombras; @c false en caso contrario.
	 */
	bool
		canCastShadow() const { return castShadow; }

	/**
	 * @brief Renderiza la sombra del actor.
	 *
	 * Usa shaders y estados específicos de shadow mapping para dibujar la proyección del
	 * actor en el mapa de sombras.
	 *
	 * @param deviceContext Contexto del dispositivo para operaciones gráficas.
	 */
	void
		renderShadow(DeviceContext& deviceContext);

private:
	std::vector<MeshComponent> m_meshes;   ///< Conjunto de componentes de malla del actor.
	std::vector<Texture> m_textures;       ///< Texturas aplicadas al actor.
	std::vector<Buffer> m_vertexBuffers;   ///< Buffers de vértices asociados a las mallas.
	std::vector<Buffer> m_indexBuffers;    ///< Buffers de índices asociados a las mallas.

	//BlendState m_blendstate;               ///< Estado de blending usado por el actor.
	// RasterizerState m_rasterizer;               ///< Estado de rasterización usado por el
	// actor.
	SamplerState m_sampler;                ///< Estado de muestreo de texturas.
	/// @brief Constante de buffer para transformaciones por frame.
	CBChangesEveryFrame m_model;
	Buffer m_modelBuffer;                  ///< Constant buffer que contiene @c m_model.

	// Recursos para sombras
	ShaderProgram m_shaderShadow;          ///< Shader program usado para renderizar sombras.
	Buffer m_shaderBuffer;                 ///< Buffer auxiliar para datos de sombras.
	//BlendState m_shadowBlendState;         ///< Estado de blending específico para sombras.
	/// @brief Estado de profundidad/esténcil para sombras.
	DepthStencilState m_shadowDepthStencilState;
	CBChangesEveryFrame m_cbShadow;        ///< Constant buffer específico de sombras.

	/// @brief Posición de la luz usada para proyectar sombras.
	XMFLOAT4 m_LightPos;
	std::string m_name = "Actor";          ///< Nombre identificador del actor.
	bool castShadow = true;                ///< Indica si el actor proyecta sombras.
};
