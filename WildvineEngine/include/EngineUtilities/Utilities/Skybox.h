/**
 * @file Skybox.h
 * @brief Declara utilidades de Skybox usadas por WildvineEngine.
 */
#pragma once
#include "Prerequisites.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "Buffer.h"
#include "SamplerState.h"
#include "Model3D.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "EngineUtilities\Utilities\Camera.h"
#include "ECS\Actor.h"

/** @brief Declara class Device. */
class Device;
/** @brief Declara class DeviceContext. */
class DeviceContext;

/**
 * @class Skybox
 * @brief Inicializa, actualiza y renderiza un skybox.
 *
 * Maneja los recursos necesarios como shaders, buffers, estados de rasterizaciÃ³n,
 * profundidad y textura cÃºbica para representar el entorno.
 */
class Skybox {
public:
  /**
   * @brief Constructor por defecto.
   */
  Skybox() = default;

  /**
   * @brief Destructor por defecto.
   */
  ~Skybox() = default;

  /**
   * @brief Inicializa los recursos del skybox.
   *
   * @param device Referencia al dispositivo grÃ¡fico.
   * @param deviceContext Contexto del dispositivo.
   * @param cubemap Textura cÃºbica utilizada como skybox.
   * @return HRESULT Resultado de la operaciÃ³n.
   */
  HRESULT
  init(Device &device, DeviceContext *deviceContext, Texture &cubemap);

  /**
   * @brief Actualiza el estado del skybox segÃºn la cÃ¡mara.
   *
   * @param deviceContext Contexto del dispositivo.
   * @param camera CÃ¡mara utilizada para la vista.
   */
  void update(DeviceContext &deviceContext, Camera &camera);

  /**
   * @brief Renderiza el skybox.
   *
   * @param deviceContext Contexto del dispositivo.
   */
  void render(DeviceContext &deviceContext);

  /**
   * @brief Libera recursos del skybox.
   */
  void
  destroy() {
  }

private:
  /** @brief Programa de shaders utilizado para el skybox. */
  ShaderProgram m_shaderProgram;

  /** @brief Buffer constante para enviar datos al shader. */
  Buffer m_constantBuffer;

  /** @brief Estado de muestreo de texturas. */
  SamplerState m_samplerState;

  /** @brief Estado de rasterizaciÃ³n. */
  RasterizerState m_rasterizerState;

  /** @brief Estado de profundidad y stencil. */
  DepthStencilState m_depthStencilState;

  /** @brief Textura cÃºbica del skybox. */
  Texture m_skyboxTexture;

  /** @brief Modelo 3D del cubo utilizado para el skybox. */
  Model3D *m_cubeModel = nullptr;

  /** @brief Actor asociado al skybox. */
  EU::TSharedPointer<Actor> m_skybox;
};
