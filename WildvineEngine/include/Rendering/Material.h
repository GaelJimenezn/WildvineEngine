
#pragma once
#include "Prerequisites.h"
#include "Rendering/RenderTypes.h"

class ShaderProgram;
class RasterizerState;
class DepthStencilState;
class SamplerState;

class
Material {
public:
    void
    setShader(ShaderProgram* shader) { m_shader = shader; }

    void
    setRasterizerState(RasterizerState* state) { m_rasterizerState = state; }

    void
    setDepthStencilState(DepthStencilState* state) { m_depthStencilState = state; }

    void
    setSamplerState(SamplerState* state) { m_samplerState = state; }

    void
    setDomain(MaterialDomain domain) { m_domain = domain; }

    void
    setBlendMode(BlendMode blendMode) { m_blendMode = blendMode; }

    ShaderProgram* getShader() const { return m_shader; }
    RasterizerState* getRasterizerState() const { return m_rasterizerState; }
    DepthStencilState* getDepthStencilState() const { return m_depthStencilState; }
    SamplerState* getSamplerState() const { return m_samplerState; }
    MaterialDomain getDomain() const { return m_domain; }
    BlendMode getBlendMode() const { return m_blendMode; }

private:
    ShaderProgram* m_shader = nullptr;                  ///< Shader principal del material.
    RasterizerState* m_rasterizerState = nullptr;       ///< Estado de rasterizacion asociado.
    DepthStencilState* m_depthStencilState = nullptr;   ///< Estado de profundidad/estencil asociado.
    SamplerState* m_samplerState = nullptr;             ///< Sampler por defecto para texturas del material.
    MaterialDomain m_domain = MaterialDomain::Opaque;   ///< Dominio de render del material.
    BlendMode m_blendMode = BlendMode::Opaque;          ///< Modo de mezcla solicitado por el material.
};