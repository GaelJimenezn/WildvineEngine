
#pragma once
#include "Prerequisites.h"
#include "Rendering/RenderTypes.h"

class ShaderProgram;
class RasterizerState;
class DepthStencilState;
class SamplerState;

/**
 * @class Material
 * @brief Describes shared render state used by one or more material instances.
 *
 * A material stores non-owning pointers to shader and pipeline state objects plus the
 * high-level domain/blend settings used by render queues and blend-state resolution.
 */
class
	Material {
public:
    /** @brief Assigns the shader program used to render this material. */
    void
    	setShader(ShaderProgram* shader) { m_shader = shader; }

    /** @brief Assigns the rasterizer state used by this material. */
    void
    	setRasterizerState(RasterizerState* state) { m_rasterizerState = state; }

    /** @brief Assigns the depth-stencil state used by this material. */
    void
    	setDepthStencilState(DepthStencilState* state) { m_depthStencilState = state; }

    /** @brief Assigns the default sampler state used by this material. */
    void
    	setSamplerState(SamplerState* state) { m_samplerState = state; }

    /** @brief Sets the render domain used for queue classification. */
    void
    	setDomain(MaterialDomain domain) { m_domain = domain; }

    /** @brief Sets the blend mode requested during transparent/composite passes. */
    void
    	setBlendMode(BlendMode blendMode) { m_blendMode = blendMode; }

    /** @brief Returns the shader program assigned to this material. */
    ShaderProgram*
    	getShader() const { return m_shader; }
    /** @brief Returns the rasterizer state assigned to this material. */
    RasterizerState*
    	getRasterizerState() const { return m_rasterizerState; }
    /** @brief Returns the depth-stencil state assigned to this material. */
    DepthStencilState*
    	getDepthStencilState() const { return m_depthStencilState; }
    /** @brief Returns the sampler state assigned to this material. */
    SamplerState*
    	getSamplerState() const { return m_samplerState; }
    /** @brief Returns this material render domain. */
    MaterialDomain
    	getDomain() const { return m_domain; }
    /** @brief Returns this material blend mode. */
    BlendMode
    	getBlendMode() const { return m_blendMode; }

private:
    /// @brief Shader principal del material.
    ShaderProgram* m_shader = nullptr;
    /// @brief Estado de rasterizacion asociado.
    RasterizerState* m_rasterizerState = nullptr;
    /// @brief Estado de profundidad/estencil asociado.
    DepthStencilState* m_depthStencilState = nullptr;
    /// @brief Sampler por defecto para texturas del material.
    SamplerState* m_samplerState = nullptr;
    /// @brief Dominio de render del material.
    MaterialDomain m_domain = MaterialDomain::Opaque;
    /// @brief Modo de mezcla solicitado por el material.
    BlendMode m_blendMode = BlendMode::Opaque;
};
