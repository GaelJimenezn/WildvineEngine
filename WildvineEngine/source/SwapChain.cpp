#include "SwapChain.h"
#include "Device.h"
#include "DeviceContext.h"
#include "Texture.h"
#include "Window.h"

//
// La función `init` se encarga de inicializar el Swap Chain, que es un componente clave en la renderización de Direct3D.
// Su propósito principal es gestionar el intercambio de los buffers de la pantalla,
// permitiendo que la imagen renderizada en el "back buffer" se muestre en el "front buffer" (la pantalla).
//
HRESULT
SwapChain::init(Device& device,
  DeviceContext& deviceContext,
  Texture& backBuffer,
  Window window) {
  // Se verifica que el handle de la ventana sea válido antes de continuar.
  if (!window.m_hWnd) {
    ERROR("SwapChain", "init", "Invalid window handle. (m_hWnd is nullptr)");
    return E_POINTER;
  }

  HRESULT
  hr = S_OK;

  //
  // Se configuran los flags para la creación del dispositivo. En modo de depuración,
  // se agrega el flag para permitir mensajes de depuración de Direct3D.
  //
  unsigned int createDeviceFlags = 0;
#ifdef _DEBUG
  createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  //
  // Se definen los tipos de controladores que se intentarán. Se prefiere el hardware,
  // seguido de WARP (renderización por software de alto rendimiento) y de referencia.
  //
  D3D_DRIVER_TYPE driverTypes[] = {
      D3D_DRIVER_TYPE_HARDWARE,
      D3D_DRIVER_TYPE_WARP,
      D3D_DRIVER_TYPE_REFERENCE,
  };
  unsigned int numDriverTypes = ARRAYSIZE(driverTypes);

  //
  // Se definen los niveles de características de Direct3D que se soportarán.
  // Se intenta con el más reciente (11.0) y se retrocede si es necesario.
  //
  D3D_FEATURE_LEVEL featureLevels[] = {
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0,
  };
  unsigned int numFeatureLevels = ARRAYSIZE(featureLevels);

  //
  // Se intenta crear el dispositivo y el contexto de Direct3D. La función `D3D11CreateDevice`
  // intenta con los niveles de características y tipos de controladores definidos.
  //
  for (unsigned int driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
    D3D_DRIVER_TYPE driverType = driverTypes[driverTypeIndex];
    hr = D3D11CreateDevice(nullptr,
        driverType,
        nullptr,
        createDeviceFlags,
        featureLevels,
        numFeatureLevels,
        D3D11_SDK_VERSION,
        &device.m_device,
        &m_featureLevel,
        &deviceContext.m_deviceContext);

    // Si la creación del dispositivo es exitosa, se sale del bucle.
    if (SUCCEEDED(hr)) {
      MESSAGE("SwapChain", "init", "Device created successfully.");
      break;
    }
  }

  // Si la creación del dispositivo falla después de todos los intentos, se devuelve un error.
  if (FAILED(hr)) {
    ERROR("SwapChain", "init",
      ("Failed to create D3D11 device. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  //
  // Se configuran los ajustes de MSAA (Multisample Anti-Aliasing) para suavizar bordes.
  // Se verifica si la tarjeta gráfica soporta el número de muestras y los niveles de calidad deseados.
  //
  m_sampleCount = 4;
  hr = device.m_device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM,
    m_sampleCount,
    &m_qualityLevels);
  if (FAILED(hr) || m_qualityLevels == 0) {
    ERROR("SwapChain", "init",
      ("MSAA not supported or invalid quality level. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  //
  // Se configura la descripción del Swap Chain (`DXGI_SWAP_CHAIN_DESC`).
  // Se especifican propiedades como el tamaño de la ventana, el formato del buffer,
  // la tasa de refresco, el modo de ventana, y la configuración de MSAA.
  //
  DXGI_SWAP_CHAIN_DESC sd;
  memset(&sd, 0, sizeof(sd));
  sd.BufferCount = 1;
  sd.BufferDesc.Width = window.m_width;
  sd.BufferDesc.Height = window.m_height;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = window.m_hWnd;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
  sd.SampleDesc.Count = m_sampleCount;
  sd.SampleDesc.Quality = m_qualityLevels - 1;

  //
  // Se obtienen las interfaces de DXGI para crear el Swap Chain.
  // Primero se consulta la interfaz `IDXGIDevice`, luego el adaptador y finalmente la fábrica.
  //
  hr = device.m_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&m_dxgiDevice);
  if (FAILED(hr)) {
    ERROR("SwapChain", "init",
      ("Failed to query IDXGIDevice. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  hr = m_dxgiDevice->GetAdapter(&m_dxgiAdapter);
  if (FAILED(hr)) {
    ERROR("SwapChain", "init",
      ("Failed to get IDXGIAdapter. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  hr = m_dxgiAdapter->GetParent(__uuidof(IDXGIFactory),
                                 reinterpret_cast<void**>(&m_dxgiFactory));
  if (FAILED(hr)) {
    ERROR("SwapChain", "init",
      ("Failed to get IDXGIFactory. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  //
  // Con la fábrica de DXGI, se crea el Swap Chain final usando la descripción configurada.
  //
  hr = m_dxgiFactory->CreateSwapChain(device.m_device, 
                                      &sd, &m_swapChain);

  if (FAILED(hr)) {
    ERROR("SwapChain", "init",
      ("Failed to create swap chain. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  //
  // Finalmente, se obtiene el puntero al back buffer del Swap Chain. Este back buffer
  // es una textura donde se renderizará el contenido de la escena.
  //
  hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                             reinterpret_cast<void**>(&backBuffer));
  if (FAILED(hr)) {
    ERROR("SwapChain", "init",
      ("Failed to get back buffer. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }
  return S_OK;
}

//
// La función `destroy` se encarga de liberar todos los recursos de Direct3D relacionados
// con el Swap Chain, en el orden correcto para evitar problemas.
//
void
SwapChain::destroy() {
  if (m_swapChain) {
    SAFE_RELEASE(m_swapChain);
  }
  if (m_dxgiDevice) {
    SAFE_RELEASE(m_dxgiDevice);
  }
  if (m_dxgiAdapter) {
    SAFE_RELEASE(m_dxgiAdapter);
  }
  if (m_dxgiFactory) {
    SAFE_RELEASE(m_dxgiFactory);
  }
}

//
// La función `present` intercambia el back buffer con el front buffer.
// Esto hace que la imagen renderizada en el back buffer sea visible en la pantalla.
//
void
SwapChain::present() {
  if (m_swapChain) {
    HRESULT hr = m_swapChain->Present(0, 0);
    if (FAILED(hr)) {
      ERROR("SwapChain", "present",
        ("Failed to present swap chain. HRESULT: " + std::to_string(hr)).c_str());
    }
  }
  else {
    ERROR("SwapChain", "present", "Swap chain is not initialized.");
  }
}

HRESULT 
SwapChain::resizeBuffers(UINT width, UINT height)
{
  if (!m_swapChain) {
    ERROR("SwapChain", "resizeBuffers", "Swap chain is not initialized.");
    return E_POINTER;
  }

  // Mantiene cantidad de buffers y formato actual
  HRESULT hr = m_swapChain->ResizeBuffers(
    0,
    width,
    height,
    DXGI_FORMAT_UNKNOWN,
    0
  );

  if (FAILED(hr)) {
    ERROR("SwapChain", "resizeBuffers",
      ("ResizeBuffers failed. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  return S_OK;
}

HRESULT 
SwapChain::getBackBuffer(Texture& backBuffer)
{
  if (!m_swapChain) {
    ERROR("SwapChain", "getBackBuffer", "Swap chain is not initialized.");
    return E_POINTER;
  }

  // IMPORTANTE: backBuffer debe ser un ID3D11Texture2D* internamente
  HRESULT hr = m_swapChain->GetBuffer(
    0, __uuidof(ID3D11Texture2D),
    reinterpret_cast<void**>(&backBuffer)
  );

  if (FAILED(hr)) {
    ERROR("SwapChain", "getBackBuffer",
      ("Failed to get back buffer. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  return S_OK;
}