/**
 * @file RasterizerState.h
 * @brief Declara la API pÃºblica de RasterizerState dentro de WildvineEngine.
 */
#pragma once
#include "Prerequisites.h"

/** @brief Declara class Device. */
class Device;
/** @brief Declara class DeviceContext. */
class DeviceContext;

/**
 * @class RasterizerState
 * @brief Encapsula el estado de rasterizaciÃ³n para el pipeline grÃ¡fico.
 *
 * Esta clase gestiona la configuraciÃ³n de cÃ³mo se rasterizan los polÃ­gonos,
 * incluyendo modo de relleno, culling y clipping de profundidad.
 */
class RasterizerState {
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
   * @brief Inicializa el estado de rasterizaciÃ³n con configuraciÃ³n por defecto.
   *
   * @param device Dispositivo grÃ¡fico.
   * @return HRESULT Resultado de la operaciÃ³n.
   */
  HRESULT
  init(Device device);

  /**
   * @brief Inicializa el estado de rasterizaciÃ³n con parÃ¡metros personalizados.
   *
   * @param device Referencia al dispositivo grÃ¡fico.
   * @param fill Modo de relleno (wireframe o sÃ³lido).
   * @param cull Modo de descarte de caras.
   * @param frontCCW Indica si las caras frontales estÃ¡n en sentido antihorario.
   * @param depthClip Habilita o deshabilita el clipping de profundidad.
   * @return HRESULT Resultado de la operaciÃ³n.
   */
  HRESULT
  init(Device &device,
       D3D11_FILL_MODE fill,
       D3D11_CULL_MODE cull,
       bool frontCCW,
       bool depthClip);

  /**
   * @brief Actualiza el estado de rasterizaciÃ³n si es necesario.
   */
  void update();

  /**
   * @brief Aplica el estado de rasterizaciÃ³n al contexto de renderizado.
   *
   * @param deviceContext Contexto del dispositivo.
   */
  void render(DeviceContext &deviceContext);

  /**
   * @brief Libera los recursos asociados al estado de rasterizaciÃ³n.
   */
  void destroy();

private:
  /**
   * @brief Puntero al estado de rasterizaciÃ³n de Direct3D.
   */
  ID3D11RasterizerState *m_rasterizerState = nullptr;
};
