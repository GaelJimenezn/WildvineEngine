#include "DepthStencilState.h"
#include "Device.h"
#include "DeviceContext.h"

HRESULT
DepthStencilState::init(Device& device,
	bool depthEnable,
	D3D11_DEPTH_WRITE_MASK writeMask,
	D3D11_COMPARISON_FUNC depthFunc) {
	if (!device.m_device) {
		ERROR("ShaderProgram", "init", "Device is null.");
		return E_POINTER;
	}
	D3D11_DEPTH_STENCIL_DESC desc{};
	desc.DepthEnable = depthEnable;
	desc.DepthWriteMask = writeMask;
	desc.DepthFunc = depthFunc;
	desc.StencilEnable = false;

	HRESULT hr = device.m_device->CreateDepthStencilState(&desc, &m_depthStencilState);
	if (FAILED(hr)) {
		ERROR("DepthStencilState", "init", "Failed to create DepthStencilState");
		return hr;
	}
	return hr;
}

void
DepthStencilState::update() {

}

void
DepthStencilState::render(DeviceContext& deviceContext,
	unsigned int stencilRef,
	bool reset) {
	if (!deviceContext.m_deviceContext) {
		ERROR("RenderTargetView", "render", "DeviceContext is nullptr.");
		return;
	}
	if (!m_depthStencilState) {
		ERROR("DepthStencilState", "render", "DepthStencilState is nullptr");
		return;
	}

	if (!reset) {
		deviceContext.m_deviceContext->OMSetDepthStencilState(m_depthStencilState, stencilRef);
	}
	else {
		deviceContext.m_deviceContext->OMSetDepthStencilState(nullptr, stencilRef);
	}
}

void
DepthStencilState::destroy() {
	SAFE_RELEASE(m_depthStencilState);
}
