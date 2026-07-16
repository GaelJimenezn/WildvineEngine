/**
 * @file DepthStencilState.h
 * @brief Declara la API pública de DepthStencilState dentro de WildvineEngine.
 */
#pragma once
#include "Prerequisites.h"

/** @brief Declara class Device. */
class
Device;
/** @brief Declara class DeviceContext. */
class
DeviceContext;


/**
 * @class DepthStencilState
 * @brief Gestiona el estado de profundidad y stencil en el pipeline gráfico.
 *
 * Permite configurar cómo se manejan las pruebas de profundidad y las operaciones
 * de stencil durante el renderizado.
 */
class
DepthStencilState {
public:

  /**
   * @brief Constructor por defecto.
   */
  DepthStencilState() = default;

  /**
   * @brief Destructor por defecto.
   */
  ~DepthStencilState() = default;


  /**
   * @brief Inicializa el estado de profundidad y stencil.
   *
   * @param device Referencia al dispositivo gráfico.
   * @param depthEnable Habilita o deshabilita la prueba de profundidad.
   * @param writeMask Máscara de escritura de profundidad.
   * @param depthFunc Función de comparación de profundidad.
   * @return HRESULT Resultado de la operación.
   */
  HRESULT
  init(Device& device,
        bool depthEnable,
        D3D11_DEPTH_WRITE_MASK writeMask,
        D3D11_COMPARISON_FUNC depthFunc);


  /**
   * @brief Actualiza el estado de profundidad y stencil si es necesario.
   */
  void
  update();

  /**
   * @brief Aplica el estado al contexto de renderizado.
   *
   * @param deviceContext Contexto del dispositivo.
   * @param stencilRef Valor de referencia para operaciones de stencil.
   * @param reset Indica si se debe restaurar el estado por defecto.
   */
  void
  render(DeviceContext& deviceContext, unsigned int stencilRef = 0, bool reset = false);


  /**
   * @brief Libera los recursos asociados al estado de profundidad y stencil.
   */
  void
  destroy();

private:
  /**
   * @brief Puntero al estado de profundidad y stencil de Direct3D.
   */
  ID3D11DepthStencilState* m_depthStencilState = nullptr;
};
