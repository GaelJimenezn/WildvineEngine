/**
 * @file SwapChain.h
 * @brief Declara la API pÃºblica de SwapChain dentro de WildvineEngine.
 */
#pragma once
#include "Prerequisites.h"

/** @brief Declara class Device. */
class Device;

/** @brief Declara class DeviceContext. */
class DeviceContext;

/** @brief Declara class Window. */
class Window;

/** @brief Declara class Texture. */
class Texture;

/**
 * @class SwapChain
 * @brief Encapsula la funcionalidad de un Swap Chain en DirectX 11.
 *
 * El Swap Chain administra el intercambio de buffers entre el back buffer y
 * el front buffer, lo cual permite mostrar los fotogramas renderizados en
 * pantalla. TambiÃ©n gestiona la configuraciÃ³n de multisampling para mejorar
 * la calidad visual.
 */
class SwapChain {
public:
  /**
   * @brief Constructor por defecto.
   */
  SwapChain() = default;

  /**
   * @brief Destructor por defecto.
   */
  ~SwapChain() = default;

  /**
   * @brief Inicializa el Swap Chain.
   *
   * @param device Referencia al dispositivo de DirectX.
   * @param deviceContext Contexto del dispositivo.
   * @param backBuffer Textura asociada al back buffer.
   * @param window Ventana donde se presentarÃ¡ el contenido renderizado.
   * @return HRESULT CÃ³digo de resultado (S_OK si se inicializÃ³ correctamente).
   */
  HRESULT
  init(Device &device, DeviceContext &deviceContext, Texture &backBuffer, Window window);
  // multi aliasing mejora la calidad de pÃ­xeles

  /**
   * @brief Actualiza el estado del Swap Chain.
   *
   * FunciÃ³n placeholder que puede usarse para lÃ³gica de actualizaciÃ³n
   * relacionada con el swap chain.
   */
  void update();

  /**
   * @brief Renderiza utilizando el Swap Chain.
   *
   * Esta funciÃ³n puede contener lÃ³gica de render previo a la presentaciÃ³n
   * de los buffers.
   */
  void render();

  /**
   * @brief Libera los recursos asociados al Swap Chain.
   */
  void destroy();

  /**
   * @brief Presenta el contenido del back buffer en la ventana.
   *
   * Intercambia los buffers (front y back) para mostrar en pantalla
   * el fotograma renderizado mÃ¡s reciente.
   */
  void present();

  /** @brief Declara o ejecuta resizeBuffers. */
  HRESULT
  resizeBuffers(unsigned int width, unsigned int height);

  /** @brief Declara o ejecuta getBackBuffer. */
  HRESULT
  getBackBuffer(Texture &backBuffer);

public:
  /**
   * @brief Puntero al objeto IDXGISwapChain de DirectX 11.
   */
  IDXGISwapChain *m_swapChain = nullptr;

  /**
   * @brief Tipo de driver utilizado (hardware, referencia, etc.).
   */
  D3D_DRIVER_TYPE m_driverType = D3D_DRIVER_TYPE_NULL;

private:
  /**
   * @brief Nivel de caracterÃ­sticas de DirectX soportado.
   */
  D3D_FEATURE_LEVEL m_featureLevel = D3D_FEATURE_LEVEL_11_0;

  /**
   * @brief NÃºmero de muestras de multisampling utilizadas.
   */
  unsigned int m_sampleCount;

  /**
   * @brief NÃºmero de niveles de calidad soportados para multisampling.
   */
  unsigned int m_qualityLevels;

  /**
   * @brief Puntero al objeto IDXGIDevice de DirectX.
   */
  IDXGIDevice *m_dxgiDevice = nullptr;

  /**
   * @brief Puntero al objeto IDXGIAdapter de DirectX.
   */
  IDXGIAdapter *m_dxgiAdapter = nullptr;

  /**
   * @brief Puntero al objeto IDXGIFactory de DirectX.
   */
  IDXGIFactory *m_dxgiFactory = nullptr;
};
