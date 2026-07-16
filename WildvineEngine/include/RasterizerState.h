/**
 * @file RasterizerState.h
 * @brief Declara la API pública de RasterizerState dentro de WildvineEngine.
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
 * @class RasterizerState
 * @brief Encapsula el estado de rasterización para el pipeline gráfico.
 *
 * Esta clase gestiona la configuración de cómo se rasterizan los polígonos,
 * incluyendo modo de relleno, culling y clipping de profundidad.
 */
class
RasterizerState {
public:

  /**
   * @brief Constructor por defecto.
   */
  RasterizerState() = default;

  /**
   * @brief Destructor por defecto.
   */
  ~RasterizerState() = default;


  /**
   * @brief Inicializa el estado de rasterización con configuración por defecto.
   *
   * @param device Dispositivo gráfico.
   * @return HRESULT Resultado de la operación.
   */
  HRESULT
    init(Device device);

  /**
   * @brief Inicializa el estado de rasterización con parámetros personalizados.
   *
   * @param device Referencia al dispositivo gráfico.
   * @param fill Modo de relleno (wireframe o sólido).
   * @param cull Modo de descarte de caras.
   * @param frontCCW Indica si las caras frontales están en sentido antihorario.
   * @param depthClip Habilita o deshabilita el clipping de profundidad.
   * @return HRESULT Resultado de la operación.
   */
  HRESULT
    init(Device& device,
        D3D11_FILL_MODE fill,
        D3D11_CULL_MODE cull,
        bool frontCCW,
        bool depthClip);


  /**
   * @brief Actualiza el estado de rasterización si es necesario.
   */
  void
  update();

  /**
   * @brief Aplica el estado de rasterización al contexto de renderizado.
   *
   * @param deviceContext Contexto del dispositivo.
   */
  void
  render(DeviceContext& deviceContext);


  /**
   * @brief Libera los recursos asociados al estado de rasterización.
   */
  void
    destroy();

private:

  /**
   * @brief Puntero al estado de rasterización de Direct3D.
   */
  ID3D11RasterizerState* m_rasterizerState = nullptr;
};
