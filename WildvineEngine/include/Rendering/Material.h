#pragma once
#pragma once
#include "Prerequisites.h"
#include "Rendering/RenderTypes.h"

class ShaderProgram;
class RasterizerState;
class DepthStencilState;
class SamplerState;

/**
 * @class Material
 * @brief Contiene el estado del pipeline gráfico y los shaders compartidos por múltiples instancias.
 */
class
Material {

public:

	/**
	 * @brief Asigna el programa de shaders a utilizar.
	 * @param shader Puntero al ShaderProgram que se vinculará.
	 */
	void 
	setShader(ShaderProgram* shader) { m_shader = shader; }
	
	/**
	 * @brief Define el estado de rasterización (culling, wireframe, etc.).
	 * @param state Puntero al RasterizerState configurado.
	 */
	void 
	setRasterizerState(RasterizerState* state) { m_rasterizerState = state; }

	/**
	 * @brief Define el comportamiento de escritura y prueba en los buffers de profundidad y stencil.
	 * @param state Puntero al DepthStencilState configurado.
	 */
	void 
	setDepthStencilState(DepthStencilState* state) { m_depthStencilState = state; }

	/**
	 * @brief Define el estado del sampler para el filtrado de texturas.
	 * @param state Puntero al SamplerState configurado.
	 */
	void 
	setSamplerState(SamplerState* state) { m_samplerState = state; }

	/**
	 * @brief Establece el dominio del material (Opaco, Enmascarado, Transparente).
	 * @param domain Valor del MaterialDomain.
	 */
	void 
	setDomain(MaterialDomain domain) { m_domain = domain; }

	/**
	 * @brief Establece el modo de mezcla a utilizar (Solo relevante para materiales transparentes).
	 * @param blendMode Modo de mezcla desde BlendMode.
	 */
	void 
	setBlendMode(BlendMode blendMode) { m_blendMode = blendMode; }


	/**
	 * @brief Obtiene el programa de shaders asignado.
	 * @return Puntero al ShaderProgram actual.
	 */
	ShaderProgram* getShader() const { return m_shader; }

	/**
	 * @brief Obtiene el estado de rasterización asignado.
	 * @return Puntero al RasterizerState actual.
	 */
	RasterizerState* getRasterizerState() const { return m_rasterizerState; }

	/**
	 * @brief Obtiene el estado de Depth/Stencil asignado.
	 * @return Puntero al DepthStencilState actual.
	 */
	DepthStencilState* getDepthStencilState() const { return m_depthStencilState; }

	/**
	 * @brief Obtiene el estado del sampler asignado.
	 * @return Puntero al SamplerState actual.
	 */
	SamplerState* getSamplerState() const { return m_samplerState; }

	/**
	 * @brief Obtiene el dominio de renderizado del material.
	 * @return El MaterialDomain configurado.
	 */
	MaterialDomain 
	getDomain() const { return m_domain; }

	/**
	 * @brief Obtiene el modo de mezcla del material.
	 * @return El BlendMode configurado.
	 */
	BlendMode 
	getBlendMode() const { return m_blendMode; }

private:
	ShaderProgram* m_shader = nullptr;             /**< Puntero al programa de shaders. */
	RasterizerState* m_rasterizerState = nullptr;  /**< Puntero al estado del rasterizador. */
	DepthStencilState* m_depthStencilState = nullptr; /**< Puntero al estado de Depth/Stencil. */
	SamplerState* m_samplerState = nullptr;        /**< Puntero al estado del muestreador de texturas. */
	MaterialDomain m_domain = MaterialDomain::Opaque; /**< Dominio por defecto del material. */
	BlendMode m_blendMode = BlendMode::Opaque;        /**< Modo de mezcla por defecto. */
};