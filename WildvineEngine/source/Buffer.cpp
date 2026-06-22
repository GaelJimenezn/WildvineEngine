#include "Buffer.h"
#include "Device.h"
#include "DeviceContext.h"


HRESULT
Buffer::init(Device& device, const MeshComponent& mesh, unsigned int bindFlag) {
	if (!device.m_device) {
		ERROR("ShaderProgram", "init", "Device is null.");
		return E_POINTER;
	}
	if ((bindFlag & D3D11_BIND_VERTEX_BUFFER) && mesh.m_vertex.empty() && mesh.m_skyVertex.empty()) {
		ERROR("Buffer", "init", "Vertex buffer is empty");
		return E_INVALIDARG;
	}
	if ((bindFlag & D3D11_BIND_INDEX_BUFFER) && mesh.m_index.empty()) {
		ERROR("Buffer", "init", "Index buffer is empty");
		return E_INVALIDARG;
	}

	D3D11_BUFFER_DESC desc = {};
	D3D11_SUBRESOURCE_DATA data = {};

	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	m_bindFlag = bindFlag;
  desc.BindFlags = (D3D11_BIND_FLAG)bindFlag;

	if (bindFlag & D3D11_BIND_VERTEX_BUFFER) {
		if (mesh.m_skyVertex.size() > 0 && mesh.m_vertex.size() == 0) {
			m_stride = sizeof(SkyboxVertex);
			desc.ByteWidth = m_stride * static_cast<unsigned int>(mesh.m_skyVertex.size());
			data.pSysMem = mesh.m_skyVertex.data();

		}
		else {
			m_stride = sizeof(SimpleVertex);
			desc.ByteWidth = m_stride * static_cast<unsigned int>(mesh.m_vertex.size());
			data.pSysMem = mesh.m_vertex.data();
		}
	}
	else if (bindFlag & D3D11_BIND_INDEX_BUFFER) {
		m_stride = sizeof(unsigned int);
		desc.ByteWidth = m_stride * static_cast<unsigned int>(mesh.m_index.size());
		desc.BindFlags = (D3D11_BIND_FLAG)bindFlag;
		data.pSysMem = mesh.m_index.data();
	}

	return createBuffer(device, desc, &data);
}

HRESULT
Buffer::init(Device& device, unsigned int ByteWidth) {
	if (!device.m_device) {
		ERROR("ShaderProgram", "init", "Device is null.");
		return E_POINTER;
	}
	if (ByteWidth == 0) {
		ERROR("Buffer", "init", "ByteWidth is zero");
		return E_INVALIDARG;
	}
	m_stride = ByteWidth;

	D3D11_BUFFER_DESC desc = {};
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = ByteWidth;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	m_bindFlag = desc.BindFlags;

	return createBuffer(device, desc, nullptr);
}

void
Buffer::update(DeviceContext& deviceContext,
							ID3D11Resource* pDstResource,
							unsigned int DstSubresource,
							const D3D11_BOX* pDstBox,
							const void* pSrcData,
							unsigned int SrcRowPitch,
							unsigned int SrcDepthPitch) {
	if (!m_buffer) {
		ERROR("ShaderProgram", "update", "m_buffer is null.");
		return;
	}
	if (!pSrcData) {
		ERROR("ShaderProgram", "update", "pSrcData is null.");
		return;
	}
	deviceContext.m_deviceContext->UpdateSubresource(m_buffer,
		DstSubresource,
		pDstBox,
		pSrcData,
		SrcRowPitch,
		SrcDepthPitch);


}

void
Buffer::render(DeviceContext& deviceContext,
							unsigned int StartSlot,
							unsigned int NumBuffers,
							bool setPixelShader,
							DXGI_FORMAT format) {
	if (!deviceContext.m_deviceContext) {
		ERROR("RenderTargetView", "render", "DeviceContext is nullptr.");
		return;
	}
	if (!m_buffer) {
		ERROR("Buffer", "render", "m_buffer is null.");
		return;
	}

	switch (m_bindFlag) {
	case D3D11_BIND_VERTEX_BUFFER:
		deviceContext.m_deviceContext->IASetVertexBuffers(StartSlot, NumBuffers, &m_buffer, &m_stride, &m_offset);
		break;
	case D3D11_BIND_CONSTANT_BUFFER:
		deviceContext.m_deviceContext->VSSetConstantBuffers(StartSlot, NumBuffers, &m_buffer);
		if (setPixelShader) {
			deviceContext.m_deviceContext->PSSetConstantBuffers(StartSlot, NumBuffers, &m_buffer);
		}
		break;
	case D3D11_BIND_INDEX_BUFFER:
		deviceContext.m_deviceContext->IASetIndexBuffer(m_buffer, format, m_offset);
		break;
	default:
		ERROR("Buffer", "render", "Unsupported BindFlag");
		break;
	}
}

void
Buffer::destroy() {
	SAFE_RELEASE(m_buffer);
}

HRESULT
Buffer::createBuffer(Device& device,
	D3D11_BUFFER_DESC& desc,
	D3D11_SUBRESOURCE_DATA* initData) {
	if (!device.m_device) {
		ERROR("Buffer", "createBuffer", "Device is nullptr");
		return E_POINTER;
	}

	HRESULT hr = device.CreateBuffer(&desc, initData, &m_buffer);
	if (FAILED(hr)) {
		ERROR("Buffer", "createBuffer", "Failed to create buffer");
		return hr;
	}
	return S_OK;
}