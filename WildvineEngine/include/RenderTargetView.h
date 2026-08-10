/**
 * @file RenderTargetView.h
 * @brief Declara la API pÃºblica de RenderTargetView dentro de WildvineEngine.
 */
#pragma once
#include "Prerequisites.h"

// Forward Declarations
/** @brief Declara class Device. */
class Device;

/** @brief Declara class DeviceContext. */
class DeviceContext;

/** @brief Declara class Texture. */
class Texture;

/** @brief Declara class DepthStencilView. */
class DepthStencilView;

/**
 * @class RenderTargetView
 * @brief Encapsula un Render Target View (RTV) de DirectX 11.
 *
 * Esta clase administra la creaciÃ³n, uso y destrucciÃ³n de un
 * ID3D11RenderTargetView, el cual se utiliza para renderizar
 * grÃ¡ficos en una textura o en el back buffer.
 */
class RenderTargetView {
public:
  ID3D11RenderTargetView *
  getNativeView() const {
    return m_renderTargetView;
  }

  /**
   * @brief Constructor por defecto.
   */
  RenderTargetView() = default;

  /**
   * @brief Destructor por defecto.
   */
  ~RenderTargetView() = default;

  /**
   * @brief Inicializa el Render Target View usando el back buffer.
   *
   * @param device Referencia al dispositivo de DirectX.
   * @param backBuffer Textura del back buffer.
   * @param Format Formato de la textura (DXGI_FORMAT).
   * @return HRESULT CÃ³digo de
   * resultado (S_OK si se inicializÃ³ correctamente).
   */
  HRESULT
  init(Device &device, Texture &backBuffer, DXGI_FORMAT Format);

  /**
   * @brief Inicializa el Render Target View con una textura personalizada.
   *
   * @param device Referencia al dispositivo de DirectX.
   * @param inTex Textura de entrada.
   * @param ViewDimension DimensiÃ³n del RTV (por ejemplo, TEXTURE2D, TEXTURE2DARRAY,
   *
   * etc.).
   * @param Format Formato de la textura (DXGI_FORMAT).
   * @return HRESULT CÃ³digo de
   * resultado (S_OK si se inicializÃ³ correctamente).
   */
  HRESULT
  init(Device &device,
       Texture &inTex,
       D3D11_RTV_DIMENSION ViewDimension,
       DXGI_FORMAT Format);

  /**
   * @brief Actualiza el estado del Render Target View.
   *
   * FunciÃ³n placeholder que puede usarse para lÃ³gica de actualizaciÃ³n
   * relacionada al render target.
   */
  void update();

  /**
   * @brief Renderiza utilizando este Render Target View y un DepthStencilView.
   *
   * @param deviceContext Contexto del dispositivo para emitir comandos de render.
   * @param depthStencilView Referencia al DepthStencilView asociado.
   * @param numViews NÃºmero de vistas a aplicar.
   * @param ClearColor Color con el que se limpia el render target (RGBA).
   */
  void render(DeviceContext &deviceContext,
              DepthStencilView &depthStencilView,
              unsigned int numViews,
              const float ClearColor[4]);

  /**
   * @brief Renderiza utilizando este Render Target View sin un DepthStencilView.
   *
   * @param deviceContext Contexto del dispositivo.
   * @param numViews NÃºmero de vistas a aplicar.
   */
  void render(DeviceContext &deviceContext, unsigned int numViews);

  /**
   * @brief Libera los recursos asociados al Render Target View.
   */
  void destroy();

private:
  /**
   * @brief Puntero al objeto ID3D11RenderTargetView de DirectX 11.
   */
  ID3D11RenderTargetView *m_renderTargetView = nullptr;
};
