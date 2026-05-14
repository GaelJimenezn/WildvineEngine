#include "DeviceContext.h"

void
DeviceContext::destroy() {
	SAFE_RELEASE(m_deviceContext);
}

void
DeviceContext::RSSetViewports(unsigned int NumViewports,
	const D3D11_VIEWPORT* pViewports) {
	if (!pViewports) {
		ERROR("DeviceContext", "RSSetViewports", "pViewports is nullptr");
		return;
	}
	m_deviceContext->RSSetViewports(NumViewports, pViewports);
}

void
DeviceContext::PSSetShaderResources(unsigned int StartSlot,
	unsigned int NumViews,
	ID3D11ShaderResourceView* const* ppShaderResourceViews) {
	if (!ppShaderResourceViews) {
		ERROR("DeviceContext", "PSSetShaderResources", "ppShaderResourceViews is nullptr");
		return;
	}
	m_deviceContext->PSSetShaderResources(StartSlot, NumViews, ppShaderResourceViews);
}

void
DeviceContext::IASetInputLayout(ID3D11InputLayout* pInputLayout) {
	if (!pInputLayout) {
		ERROR("DeviceContext", "IASetInputLayout", "pInputLayout is nullptr");
		return;
	}
	m_deviceContext->IASetInputLayout(pInputLayout);
}

void
DeviceContext::VSSetShader(ID3D11VertexShader* pVertexShader,
	ID3D11ClassInstance* const* ppClassInstances,
	unsigned int NumClassInstances) {
	if (!pVertexShader) {
		ERROR("DeviceContext", "VSSetShader", "pVertexShader is nullptr");
		return;
	}
	m_deviceContext->VSSetShader(pVertexShader, ppClassInstances, NumClassInstances);
}

void
DeviceContext::PSSetShader(ID3D11PixelShader* pPixelShader,
	ID3D11ClassInstance* const* ppClassInstances,
	unsigned int NumClassInstances) {
	if (!pPixelShader) {
		ERROR("DeviceContext", "PSSetShader", "pPixelShader is nullptr");
		return;
	}
	m_deviceContext->PSSetShader(pPixelShader, ppClassInstances, NumClassInstances);
}

void
DeviceContext::UpdateSubresource(ID3D11Resource* pDstResource,
	unsigned int DstSubresource,
	const D3D11_BOX* pDstBox,
	const void* pSrcData,
	unsigned int SrcRowPitch,
	unsigned int SrcDepthPitch) {
	if (!pDstResource || !pSrcData) {
		ERROR("DeviceContext", "UpdateSubresource",
			"Invalid arguments: pDstResource or pSrcData is nullptr");
		return;
	}
	m_deviceContext->UpdateSubresource(pDstResource,
		DstSubresource,
		pDstBox,
		pSrcData,
		SrcRowPitch,
		SrcDepthPitch);
}

void
DeviceContext::IASetVertexBuffers(unsigned int StartSlot,
	unsigned int NumBuffers,
	ID3D11Buffer* const* ppVertexBuffers,
	const unsigned int* pStrides,
	const unsigned int* pOffsets) {
	if (!ppVertexBuffers || !pStrides || !pOffsets) {
		ERROR("DeviceContext", "IASetVertexBuffers",
			"Invalid arguments: ppVertexBuffers, pStrides, or pOffsets is nullptr");
		return;
	}
	m_deviceContext->IASetVertexBuffers(StartSlot,
		NumBuffers,
		ppVertexBuffers,
		pStrides,
		pOffsets);
}

void
DeviceContext::IASetIndexBuffer(ID3D11Buffer* pIndexBuffer,
	DXGI_FORMAT Format,
	unsigned int Offset) {
	if (!pIndexBuffer) {
		ERROR("DeviceContext", "IASetIndexBuffer", "pIndexBuffer is nullptr");
		return;
	}
	m_deviceContext->IASetIndexBuffer(pIndexBuffer, Format, Offset);
}

void
DeviceContext::PSSetSamplers(unsigned int StartSlot,
	unsigned int NumSamplers,
	ID3D11SamplerState* const* ppSamplers) {
	if (!ppSamplers) {
		ERROR("DeviceContext", "PSSetSamplers", "ppSamplers is nullptr");
		return;
	}
	m_deviceContext->PSSetSamplers(StartSlot, NumSamplers, ppSamplers);
}

void
DeviceContext::RSSetState(ID3D11RasterizerState* pRasterizerState) {
	if (!pRasterizerState) {
		ERROR("DeviceContext", "RSSetState", "pRasterizerState is nullptr");
		return;
	}
	m_deviceContext->RSSetState(pRasterizerState);
}

void
DeviceContext::OMSetBlendState(ID3D11BlendState* pBlendState,
	const float BlendFactor[4],
	unsigned int SampleMask) {
	if (!pBlendState) {
		ERROR("DeviceContext", "OMSetBlendState", "pBlendState is nullptr");
		return;
	}
	m_deviceContext->OMSetBlendState(pBlendState, BlendFactor, SampleMask);
}

void
DeviceContext::OMSetRenderTargets(unsigned int NumViews,
	ID3D11RenderTargetView* const* ppRenderTargetViews,
	ID3D11DepthStencilView* pDepthStencilView) {
	// Validar los parámetros
	if (!ppRenderTargetViews && !pDepthStencilView) {
		ERROR("DeviceContext", "OMSetRenderTargets",
			"Both ppRenderTargetViews and pDepthStencilView are nullptr");
		return;
	}

	if (NumViews > 0 && !ppRenderTargetViews) {
		ERROR("DeviceContext", "OMSetRenderTargets",
			"ppRenderTargetViews is nullptr, but NumViews > 0");
		return;
	}

	// Asignar los render targets y el depth stencil
	m_deviceContext->OMSetRenderTargets(NumViews, ppRenderTargetViews, pDepthStencilView);
}

void
DeviceContext::IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY Topology) {
	// Validar el parámetro Topology
	if (Topology == D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED) {
		ERROR("DeviceContext", "IASetPrimitiveTopology",
			"Topology is D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED");
		return;
	}

	// Asignar la topología al Input Assembler
	m_deviceContext->IASetPrimitiveTopology(Topology);
}

void
DeviceContext::ClearRenderTargetView(ID3D11RenderTargetView* pRenderTargetView,
	const float ColorRGBA[4]) {
	// Validar parámetros
	if (!pRenderTargetView) {
		ERROR("DeviceContext", "ClearRenderTargetView", "pRenderTargetView is nullptr");
		return;
	}
	if (!ColorRGBA) {
		ERROR("DeviceContext", "ClearRenderTargetView", "ColorRGBA is nullptr");
		return;
	}

	// Limpiar el render target
	m_deviceContext->ClearRenderTargetView(pRenderTargetView, ColorRGBA);
}

void
DeviceContext::ClearDepthStencilView(ID3D11DepthStencilView* pDepthStencilView,
	unsigned int ClearFlags,
	float Depth,
	UINT8 Stencil) {
	// Validar parámetros
	if (!pDepthStencilView) {
		ERROR("DeviceContext", "ClearDepthStencilView",
			"pDepthStencilView is nullptr");
		return;
	}

	// Validar banderas de limpieza
	if ((ClearFlags & (D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL)) == 0) {
		ERROR("DeviceContext", "ClearDepthStencilView",
			"Invalid ClearFlags: must include D3D11_CLEAR_DEPTH or D3D11_CLEAR_STENCIL");
		return;
	}

	// Limpiar el depth stencil
	m_deviceContext->ClearDepthStencilView(pDepthStencilView, ClearFlags, Depth, Stencil);
}

void
DeviceContext::VSSetConstantBuffers(unsigned int StartSlot,
	unsigned int NumBuffers,
	ID3D11Buffer* const* ppConstantBuffers) {
	// Validar parámetros
	if (!ppConstantBuffers) {
		ERROR("DeviceContext", "VSSetConstantBuffers", "ppConstantBuffers is nullptr");
		return;
	}

	// Asignar los constant buffers al vertex shader
	m_deviceContext->VSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers);
}

void
DeviceContext::PSSetConstantBuffers(unsigned int StartSlot,
	unsigned int NumBuffers,
	ID3D11Buffer* const* ppConstantBuffers) {
	// Validar parámetros
	if (!ppConstantBuffers) {
		ERROR("DeviceContext", "PSSetConstantBuffers", "ppConstantBuffers is nullptr");
		return;
	}

	// Asignar los constant buffers al pixel shader
	m_deviceContext->PSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers);
}

void
DeviceContext::DrawIndexed(unsigned int IndexCount,
	unsigned int StartIndexLocation,
	int BaseVertexLocation) {
	// Validar parámetros
	if (IndexCount == 0) {
		ERROR("DeviceContext", "DrawIndexed", "IndexCount is zero");
		return;
	}

	// Ejecutar el dibujo
	m_deviceContext->DrawIndexed(IndexCount, StartIndexLocation, BaseVertexLocation);
}
