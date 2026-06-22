#include "DepthStencilView.h"
#include "Device.h"
#include "DeviceContext.h"
#include "Texture.h"

//
// La función `init` inicializa el DepthStencilView. 
// Aquí se crea la vista de Direct3D que nos permite usar la textura como un buffer de profundidad.
//
HRESULT
DepthStencilView::init(Device& device, Texture& depthStencil, DXGI_FORMAT format) {
	if (!device.m_device) {
		ERROR("DepthStencilView", "init", "Device is null.");
	}
	if (!depthStencil.m_texture) {
		ERROR("DepthStencilView", "init", "Texture is null.");
		return E_FAIL;
	}

	// Config depth stencil view description
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	memset(&descDSV, 0, sizeof(descDSV));
	descDSV.Format = format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	descDSV.Texture2D.MipSlice = 0;

	// Create depth stencil view
	HRESULT hr = device.m_device->CreateDepthStencilView(depthStencil.m_texture,
		&descDSV,
		&m_depthStencilView);

	if (FAILED(hr)) {
		ERROR("DepthStencilView", "init",
			("Failed to create depth stencil view. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	if (!device.m_device) {
		ERROR("DepthStencilView", "init", "Device is null.");
		return E_POINTER;
	}

	return S_OK;
}

HRESULT
DepthStencilView::init(Device& device,
	Texture& depthStencil,
	DXGI_FORMAT format,
	D3D11_DSV_DIMENSION viewDimension) {
	if (!device.m_device) {
		ERROR("DepthStencilView", "init", "Device is null.");
		return E_POINTER;
	}
	if (!depthStencil.m_texture) {
		ERROR("DepthStencilView", "init", "Texture is null.");
		return E_FAIL;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV{};
	descDSV.Format = format;
	descDSV.ViewDimension = viewDimension;

	if (viewDimension == D3D11_DSV_DIMENSION_TEXTURE2D) {
		descDSV.Texture2D.MipSlice = 0;
	}

	HRESULT hr = device.m_device->CreateDepthStencilView(
		depthStencil.m_texture,
		&descDSV,
		&m_depthStencilView
	);

	if (FAILED(hr)) {
		ERROR("DepthStencilView", "init",
			("Failed to create depth stencil view. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	return S_OK;
}

//
// La función `render` se encarga de limpiar el buffer de profundidad y de plantilla.
// Esto es necesario al comienzo de cada fotograma para que la información del anterior no interfiera.
//
void
DepthStencilView::render(DeviceContext& deviceContext) {
  //
  // Verificación de errores: se asegura de que el contexto del dispositivo y la vista no sean nulos.
  //
  if (!deviceContext.m_deviceContext) {
    ERROR("DepthStencilView", "render", "Device context is null.");
    return;
  }

  if (!m_depthStencilView) {
    ERROR("DepthStencilView", "render", "DepthStencilView is null.");
    return;
  }

  //
  // Se limpia la vista de profundidad y plantilla.
  // Se establecen los valores por defecto: 1.0f para profundidad (el punto más lejano) y 0 para plantilla.
  //
  deviceContext.m_deviceContext->ClearDepthStencilView(m_depthStencilView,
                                                      D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                                      1.0f,
                                                      0);
}

//
// La función `destroy` libera la memoria de la vista de profundidad/plantilla cuando ya no es necesaria.
// Se usa la macro SAFE_RELEASE para asegurar que se libere correctamente.
//
void
DepthStencilView::destroy() {
  SAFE_RELEASE(m_depthStencilView);
}