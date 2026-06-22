#pragma once
#include "Prerequisites.h"

/**
 * @file SamplerState.h
 * @brief Declaración de la clase SamplerState, encargada de administrar el estado del muestreador (Sampler) en DirectX 11.
 */

class Device;

class DeviceContext;

/**
 * @class SamplerState
 * @brief Clase responsable de inicializar, actualizar, renderizar y destruir el estado del muestreador en DirectX.
 *
 * Esta clase maneja el objeto ID3D11SamplerState que define cómo se muestrean las texturas
 * durante el proceso de renderizado. Permite configurar y aplicar el estado del muestreador
 * a la etapa correspondiente del pipeline gráfico.
 */
class
SamplerState {
public:

  /**
   * @brief Constructor por defecto.
   *
   * Inicializa un objeto SamplerState sin ningún estado configurado.
   */
  SamplerState() = default;

  /**
   * @brief Destructor por defecto.
   *
   * Libera los recursos asociados si es necesario.
   */
  ~SamplerState() = default;

  /**
   * @brief Inicializa el estado del muestreador.
   *
   * Crea y configura el objeto ID3D11SamplerState utilizando el dispositivo de DirectX.
   *
   * @param device Referencia al objeto Device utilizado para la creación del sampler.
   * @return Devuelve un código HRESULT que indica si la inicialización fue exitosa o no.
   */
  HRESULT
  init(Device& device);

  /**
   * @brief Actualiza los parámetros del muestreador.
   *
   * Permite modificar configuraciones internas del estado si se requiere durante la ejecución.
   */
  void
  update();

  /**
   * @brief Aplica el estado del muestreador al contexto de renderizado.
   *
   * Asigna el sampler al pipeline gráfico en las ranuras indicadas.
   *
   * @param deviceContext Referencia al contexto del dispositivo donde se aplicará el sampler.
   * @param StartSlot Índice inicial de la ranura en la que se establecerá el sampler.
   * @param NumSamplers Número de samplers a establecer comenzando desde StartSlot.
   */
  void
  render(DeviceContext& deviceContext,
        unsigned int StartSlot,
        unsigned int NumSamplers);

  /**
   * @brief Libera los recursos asociados al sampler.
   *
   * Destruye el objeto ID3D11SamplerState y limpia la memoria utilizada.
   */
  void
  destroy();

public:
  /**
   * @brief Puntero al objeto ID3D11SamplerState de DirectX.
   *
   * Representa el estado del muestreador activo en el pipeline gráfico.
   */
  ID3D11SamplerState* m_sampler = nullptr;

};
