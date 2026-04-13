#include "SwapChain.h"
#include "Device.h"
#include "DeviceContext.h"
#include "Texture.h"
#include "Window.h"

HRESULT 
SwapChain::init(Device& device, 
								DeviceContext& deviceContext, 
								Texture& backBuffer, 
								Window window) {
	// Check if Window is valid
	if (!window.m_hWnd) {
		ERROR("SwapChain", "init", "Invalid window handle. (m_hWnd is nullptr)");
		return E_POINTER;
	}

	HRESULT hr = S_OK;

	// Create the swap chain device and context
  unsigned int createDeviceFlags = 0;
#ifdef _DEBUG
  createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  D3D_DRIVER_TYPE driverTypes[] = {
      D3D_DRIVER_TYPE_HARDWARE,
      D3D_DRIVER_TYPE_WARP,
      D3D_DRIVER_TYPE_REFERENCE,
  };
  unsigned int numDriverTypes = ARRAYSIZE(driverTypes);

  D3D_FEATURE_LEVEL featureLevels[] = {
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0,
  };
  unsigned int numFeatureLevels = ARRAYSIZE(featureLevels);

  // Create the device
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

    if (SUCCEEDED(hr)) {
      MESSAGE("SwapChain", "init", "Device created successfully.");
      break;
    }
  }

  if (FAILED(hr)) {
    ERROR("SwapChain", "init",
      ("Failed to create D3D11 device. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // Config the MSAA settings
  m_sampleCount = 4;
  hr = device.m_device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM,
                                                      m_sampleCount,
                                                      &m_qualityLevels);
  if (FAILED(hr) || m_qualityLevels == 0) {
    ERROR("SwapChain", "init",
      ("MSAA not supported or invalid quality level. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // Config the swap chain description
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

  // Get the DXGI factory
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

  // Create the swap chain
  hr = m_dxgiFactory->CreateSwapChain(device.m_device, &sd, &m_swapChain);

  if (FAILED(hr)) {
    ERROR("SwapChain", "init",
      ("Failed to create swap chain. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // Get the backbuffer
  hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
    reinterpret_cast<void**>(&backBuffer));
  if (FAILED(hr)) {
    ERROR("SwapChain", "init",
      ("Failed to get back buffer. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  return S_OK;
}

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

void 
SwapChain::present() {
  if (m_swapChain) {
    HRESULT hr = m_swapChain->Present(0, 0);
    if (FAILED(hr)) {
      ERROR("SwapChain", "present", 
        ("Failed to present swap chain. HRESULT: " + std::to_string(hr)).c_str());
    }
  } else {
    ERROR("SwapChain", "present", "Swap chain is not initialized.");
	}
}

HRESULT 
SwapChain::resizeBuffers(unsigned int width, unsigned int height) {
  if (!m_swapChain) {
    ERROR("SwapChain", "resizeBuffers", "Swap chain is not initialized.");
    return E_POINTER;
  }

  // 0,0 y DXGI_FORMAT_UNKNOWN = mantener cantidad de buffers y formato actual
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

HRESULT SwapChain::getBackBuffer(Texture& backBuffer)
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
