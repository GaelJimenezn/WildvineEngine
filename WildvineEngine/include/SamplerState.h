#pragma once
#include "Prerequisites.h"

/**
 * @file SamplerState.h
 * @brief DeclaraciÃ³n de la clase SamplerState, encargada de administrar el estado del
 * muestreador (Sampler) en DirectX 11.
 */

class Device;

/** @brief Declara class DeviceContext. */
class DeviceContext;

/**
 * @class SamplerState
 * @brief Clase responsable de inicializar, actualizar, renderizar y destruir el estado
 * del muestreador en DirectX.
 *
 * Esta clase maneja el objeto ID3D11SamplerState que define cÃ³mo se muestrean las
 * texturas
 * durante el proceso de renderizado. Permite configurar y aplicar el estado del
 * muestreador
 * a la etapa correspondiente del pipeline grÃ¡fico.
 */
class SamplerState {
public:
  /**
   * @brief Constructor por defecto.
   *
   * Inicializa un objeto SamplerState sin ningÃºn estado configurado.
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
   * @param device Referencia al objeto Device utilizado para la creaciÃ³n del sampler.
   * @return S_OK si la inicializaciÃ³n fue exitosa; HRESULT de error en otro caso.
   */
  HRESULT
  init(Device &device);

  /**
   * @brief Actualiza los parÃ¡metros del muestreador.
   *
   * Permite modificar configuraciones internas del estado si se requiere durante la
   * ejecuciÃ³n.
   */
  void update();

  /**
   * @brief Aplica el estado del muestreador al contexto de renderizado.
   *
   * Asigna el sampler al pipeline grÃ¡fico en las ranuras indicadas.
   *
   * @param deviceContext Referencia al contexto del dispositivo donde se aplicarÃ¡ el
   * sampler.
   * @param StartSlot Ãndice inicial de la ranura en la que se establecerÃ¡ el
   * sampler.
   * @param NumSamplers NÃºmero de samplers a establecer comenzando desde StartSlot.
   */
  void
  render(DeviceContext &deviceContext, unsigned int StartSlot, unsigned int NumSamplers);

  /**
   * @brief Libera los recursos asociados al sampler.
   *
   * Destruye el objeto ID3D11SamplerState y limpia la memoria utilizada.
   */
  void destroy();

public:
  /**
   * @brief Puntero al objeto ID3D11SamplerState de DirectX.
   *
   * Representa el estado del muestreador activo en el pipeline grÃ¡fico.
   */
  ID3D11SamplerState *m_sampler = nullptr;
};
