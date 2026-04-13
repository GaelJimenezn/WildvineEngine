#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;
class Window;
class Texture;

/**
 * @class SwapChain
 * @brief Encapsula un @c IDXGISwapChain en Direct3D 11 para administrar buffers de presentación.
 *
 * Un Swap Chain es responsable de la gestión de los buffers de renderizado que se presentan
 * en pantalla (front y back buffer).
 * Esta clase maneja su creación, actualización, renderizado y presentación final.
 *
 * También soporta configuración de **MSAA (Multisample Anti-Aliasing)** para suavizado de bordes.
 */
class 
SwapChain {
public:
  /**
   * @brief Constructor por defecto.
   */
  SwapChain() = default;

  /**
   * @brief Destructor por defecto.
   * @details No libera automáticamente los recursos COM; llamar a destroy().
   */
  ~SwapChain() = default;

  /**
   * @brief Inicializa el Swap Chain y obtiene el back buffer.
   *
   * Crea el objeto @c IDXGISwapChain asociado a una ventana específica,
   * obteniendo además la textura del back buffer para el renderizado.
   *
   * @param device       Dispositivo con el que se crea el recurso.
   * @param deviceContext Contexto de dispositivo asociado.
   * @param backBuffer   Textura que representará el back buffer.
   * @param window       Ventana de la aplicación donde se presentará la imagen.
   * @return @c S_OK si fue exitoso; código @c HRESULT en caso contrario.
   *
   * @post Si retorna @c S_OK, @c m_swapChain != nullptr.
   */
  HRESULT 
  init(Device& device,
       DeviceContext& deviceContext,
       Texture& backBuffer,
       Window window);

  /**
   * @brief Actualiza parámetros internos del Swap Chain.
   *
   * Método de marcador para soportar cambios dinámicos, como resize de ventana,
   * reconfiguración de MSAA u otros ajustes.
   *
   * @note Actualmente no realiza ninguna operación.
   */
  void 
  update();

  /**
   * @brief Ejecuta operaciones de renderizado relacionadas con el Swap Chain.
   *
   * Usualmente se utilizaría para depuración o para sincronizar buffers
   * antes de la presentación.
   *
   * @note Actualmente no realiza ninguna operación.
   */
  void 
  render();

  /**
   * @brief Libera todos los recursos asociados al Swap Chain.
   *
   * También libera las interfaces relacionadas de DXGI (device, adapter, factory).
   *
   * @post @c m_swapChain == nullptr.
   */
  void 
  destroy();

  /**
   * @brief Presenta el back buffer en pantalla.
   *
   * Llama a @c IDXGISwapChain::Present para mostrar el contenido renderizado
   * en la ventana asociada.
   *
   * @note Si se utiliza V-Sync, puede configurarse en la implementación de este método.
   */
  void 
  present();

  HRESULT 
  resizeBuffers(unsigned int width, unsigned int height);
  
  HRESULT 
  getBackBuffer(Texture& backBuffer);

public:
  /**
   * @brief Objeto principal del Swap Chain en Direct3D 11.
   */
  IDXGISwapChain* m_swapChain = nullptr;

  /**
   * @brief Tipo de driver utilizado (hardware, referencia, software, etc.).
   */
  D3D_DRIVER_TYPE m_driverType = D3D_DRIVER_TYPE_NULL;

private:
  /**
   * @brief Nivel de características de Direct3D soportado por el dispositivo.
   */
  D3D_FEATURE_LEVEL m_featureLevel = D3D_FEATURE_LEVEL_11_0;

  /**
   * @brief Número de muestras para MSAA.
   *
   * Ejemplo: 4 = 4x MSAA (4 muestras por píxel).
   */
  unsigned int m_sampleCount;

  /**
   * @brief Niveles de calidad soportados para la configuración de MSAA.
   */
  unsigned int m_qualityLevels;

  /**
   * @brief Interfaz DXGI para el dispositivo.
   */
  IDXGIDevice* m_dxgiDevice = nullptr;

  /**
   * @brief Interfaz DXGI para el adaptador (GPU).
   */
  IDXGIAdapter* m_dxgiAdapter = nullptr;

  /**
   * @brief Interfaz DXGI para la fábrica (creación de swap chains).
   */
  IDXGIFactory* m_dxgiFactory = nullptr;
};
