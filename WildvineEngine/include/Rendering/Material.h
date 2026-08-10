/**
 * @file Material.h
 * @brief Declara la API de Material dentro del subsistema de renderizado.
 */
#pragma once
#include "Prerequisites.h"
#include "Rendering/RenderTypes.h"

/** @brief Declara class ShaderProgram. */
class ShaderProgram;
/** @brief Declara class RasterizerState. */
class RasterizerState;
/** @brief Declara class DepthStencilState. */
class DepthStencilState;
/** @brief Declara class SamplerState. */
class SamplerState;

/**
 * @class Material
 * @brief Representa un material grÃ¡fico que encapsula estados de render y shader.
 *
 * Esta clase define cÃ³mo se renderiza un objeto, incluyendo:
 * - Shader utilizado
 * - Estados de rasterizaciÃ³n, profundidad y muestreo
 * - Dominio del material (opaque, transparent, etc.)
 * - Modo de blending
 */
class Material {
public:
  /**
   * @brief Asigna el shader del material.
   * @param shader Puntero al ShaderProgram.
   */
  void
  setShader(ShaderProgram *shader) {
    m_shader = shader;
  }

  /**
   * @brief Asigna el estado de rasterizaciÃ³n.
   * @param state Puntero a RasterizerState.
   */
  void
  setRasterizerState(RasterizerState *state) {
    m_rasterizerState = state;
  }

  /**
   * @brief Asigna el estado de profundidad y stencil.
   * @param state Puntero a DepthStencilState.
   */
  void
  setDepthStencilState(DepthStencilState *state) {
    m_depthStencilState = state;
  }

  /**
   * @brief Asigna el estado de sampler.
   * @param state Puntero a SamplerState.
   */
  void
  setSamplerState(SamplerState *state) {
    m_samplerState = state;
  }

  /**
   * @brief Define el dominio del material.
   * @param domain Tipo de dominio (opaque, transparent, etc.).
   */
  void
  setDomain(MaterialDomain domain) {
    m_domain = domain;
  }

  /**
   * @brief Define el modo de blending.
   * @param blendMode Tipo de blending.
   */
  void
  setBlendMode(BlendMode blendMode) {
    m_blendMode = blendMode;
  }

  /**
   * @brief Obtiene el shader del material.
   * @return Puntero a ShaderProgram.
   */
  ShaderProgram *
  getShader() const {
    return m_shader;
  }

  /**
   * @brief Obtiene el estado de rasterizaciÃ³n.
   * @return Puntero a RasterizerState.
   */
  RasterizerState *
  getRasterizerState() const {
    return m_rasterizerState;
  }

  /**
   * @brief Obtiene el estado de profundidad y stencil.
   * @return Puntero a DepthStencilState.
   */
  DepthStencilState *
  getDepthStencilState() const {
    return m_depthStencilState;
  }

  /**
   * @brief Obtiene el estado de sampler.
   * @return Puntero a SamplerState.
   */
  SamplerState *
  getSamplerState() const {
    return m_samplerState;
  }

  /**
   * @brief Obtiene el dominio del material.
   * @return MaterialDomain actual.
   */
  MaterialDomain
  getDomain() const {
    return m_domain;
  }

  /**
   * @brief Obtiene el modo de blending.
   * @return BlendMode actual.
   */
  BlendMode
  getBlendMode() const {
    return m_blendMode;
  }

private:
  /** @brief Shader asociado al material. */
  ShaderProgram *m_shader = nullptr;

  /** @brief Estado de rasterizaciÃ³n. */
  RasterizerState *m_rasterizerState = nullptr;

  /** @brief Estado de profundidad y stencil. */
  DepthStencilState *m_depthStencilState = nullptr;

  /** @brief Estado de sampler. */
  SamplerState *m_samplerState = nullptr;

  /** @brief Dominio del material. */
  MaterialDomain m_domain = MaterialDomain::Opaque;

  /** @brief Modo de blending. */
  BlendMode m_blendMode = BlendMode::Opaque;
};
