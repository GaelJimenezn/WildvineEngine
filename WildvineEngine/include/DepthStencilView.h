/**
 * @file DepthStencilView.h
 * @brief Declara la API pÃºblica de DepthStencilView dentro de WildvineEngine.
 */
#pragma once
#include "Prerequisites.h"

/** @brief Declara class Device. */
class Device;

/** @brief Declara class DeviceContext. */
class DeviceContext;

/** @brief Declara class Texture. */
class Texture;

/**
 * @brief Encapsula una vista de profundidad y stencil en DirectX.
 *
 * Esta clase administra la creaciÃ³n, actualizaciÃ³n, renderizado
 * y destrucciÃ³n de un recurso DepthStencilView para el pipeline grÃ¡fico.
 */
class DepthStencilView {
public:
  ID3D11DepthStencilView *
  getNativeView() const {
    return m_depthStencilView;
  }

  /**
   * @brief Constructor por defecto.
   */
  DepthStencilView() = default;

  /**
   * @brief Destructor por defecto.
   */
  ~DepthStencilView() = default;

  /**
   * @brief Inicializa la vista de profundidad y stencil.
   *
   * @param device Referencia al dispositivo de DirectX.
   * @param depthStencil Textura que servirÃ¡ como buffer de profundidad/stencil.
   * @param format Formato DXGI usado para la vista.
   * @return HRESULT CÃ³digo de estado de la operaciÃ³n (S_OK si fue exitosa).
   */
  HRESULT
  init(Device &device, Texture &depthStencil, DXGI_FORMAT format);

  /** @brief Declara o ejecuta init. */
  HRESULT
  init(Device &device,
       Texture &depthStencil,
       DXGI_FORMAT format,
       D3D11_DSV_DIMENSION viewDimension);

  /**
   * @brief Actualiza el estado interno de la vista.
   *
   * Actualmente no realiza ninguna operaciÃ³n.
   */
  void update() {};

  /**
   * @brief Renderiza usando la vista de profundidad y stencil.
   *
   * @param deviceContext Contexto del dispositivo de DirectX.
   */
  void render(DeviceContext &deviceContext);

  /**
   * @brief Libera los recursos asociados al DepthStencilView.
   */
  void destroy();

public:
  /** @brief Puntero al recurso de DepthStencilView de DirectX. */
  ID3D11DepthStencilView *m_depthStencilView = nullptr;
};
