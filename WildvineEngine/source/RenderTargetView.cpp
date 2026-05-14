#include "RenderTargetView.h"
#include "Device.h"
#include "Texture.h"
#include "DeviceContext.h"
#include "DepthStencilView.h"

HRESULT
RenderTargetView::init(Device& device, Texture& backBuffer, DXGI_FORMAT Format) {
	if (!device.m_device) {
		ERROR("RenderTargetView", "init", "Device is nullptr.");
		return E_POINTER;
	}
	if (!backBuffer.m_texture) {
		ERROR("RenderTargetView", "init", "Texture is nullptr.");
		return E_POINTER;
	}
	if (Format == DXGI_FORMAT_UNKNOWN) {
		ERROR("RenderTargetView", "init", "Format is DXGI_FORMAT_UNKNOWN.");
		return E_INVALIDARG;
	}

	// Config the description for the render target view
	D3D11_RENDER_TARGET_VIEW_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.Format = Format;
	desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;

	// Create the render target view
	HRESULT hr = device.m_device->CreateRenderTargetView(backBuffer.m_texture,
		&desc,
		&m_renderTargetView);
	if (FAILED(hr)) {
		ERROR("RenderTargetView", "init",
			("Failed to create render target view. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	return S_OK;
}

HRESULT
RenderTargetView::init(Device& device,
	Texture& inTex,
	D3D11_RTV_DIMENSION ViewDimension,
	DXGI_FORMAT Format) {
	if (!device.m_device) {
		ERROR("RenderTargetView", "init", "Device is nullptr.");
		return E_POINTER;
	}
	if (!inTex.m_texture) {
		ERROR("RenderTargetView", "init", "Texture is nullptr.");
		return E_POINTER;
	}
	if (Format == DXGI_FORMAT_UNKNOWN) {
		ERROR("RenderTargetView", "init", "Format is DXGI_FORMAT_UNKNOWN.");
		return E_INVALIDARG;
	}

	// Config the description for the render target view
	D3D11_RENDER_TARGET_VIEW_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.Format = Format;
	desc.ViewDimension = ViewDimension;

	// Create the render target view
	HRESULT hr = device.m_device->CreateRenderTargetView(inTex.m_texture,
		&desc,
		&m_renderTargetView);

	if (FAILED(hr)) {
		ERROR("RenderTargetView", "init",
			("Failed to create render target view. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	return S_OK;
}

void
RenderTargetView::render(DeviceContext& deviceContext,
	DepthStencilView& depthStencilView,
	unsigned int numViews,
	const float ClearColor[4]) {
	if (!deviceContext.m_deviceContext) {
		ERROR("RenderTargetView", "render", "DeviceContext is nullptr.");
		return;
	}
	if (!m_renderTargetView) {
		ERROR("RenderTargetView", "render", "RenderTargetView is nullptr.");
		return;
	}

	// Clear the render target view
	deviceContext.m_deviceContext->ClearRenderTargetView(m_renderTargetView, ClearColor);

	// Config render target view and depth stencil view
	deviceContext.m_deviceContext->OMSetRenderTargets(numViews,
		&m_renderTargetView,
		depthStencilView.m_depthStencilView);
}

void
RenderTargetView::render(DeviceContext& deviceContext, unsigned int numViews) {
	if (!deviceContext.m_deviceContext) {
		ERROR("RenderTargetView", "render", "DeviceContext is nullptr.");
		return;
	}
	if (!m_renderTargetView) {
		ERROR("RenderTargetView", "render", "RenderTargetView is nullptr.");
		return;
	}
	// Config render target view
	deviceContext.m_deviceContext->OMSetRenderTargets(numViews,
		&m_renderTargetView,
		nullptr);
}

void RenderTargetView::destroy() {
	SAFE_RELEASE(m_renderTargetView);
}
