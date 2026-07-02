#include "DeviceContext.h"

//
// La funci�n `destroy` se encarga de liberar el objeto principal de Direct3D, el ID3D11DeviceContext.
// Este contexto es el "motor" que se utiliza para enviar comandos de renderizado a la GPU.
//
void
DeviceContext::destroy() {
	SAFE_RELEASE(m_deviceContext);
}

//
// `RSSetViewports` configura el(los) viewport(s) de la etapa de rasterizaci�n.
// Un viewport define el �rea de la ventana de salida donde se renderizar� la imagen.
//
void
DeviceContext::RSSetViewports(unsigned int NumViewports,
															const D3D11_VIEWPORT* pViewports) {
	// Verificaci�n para evitar un puntero nulo.
	if (!pViewports) {
		ERROR("DeviceContext", "RSSetViewports", "pViewports is nullptr");
		return;
	}

	// Se llama a la funci�n nativa de Direct3D.
	m_deviceContext->RSSetViewports(NumViewports, 
																	pViewports);
}

//
// `PSSetShaderResources` asigna recursos (como texturas) al Pixel Shader.
// La GPU lee estos recursos para aplicar detalles de color y textura a los p�xeles.
//
void
DeviceContext::PSSetShaderResources(unsigned int StartSlot,
																		unsigned int NumViews,
																		ID3D11ShaderResourceView* 
																		const* ppShaderResourceViews) {
	// Verificaci�n para evitar un puntero nulo.
	if (!ppShaderResourceViews) {
		ERROR("DeviceContext", "PSSetShaderResources", "ppShaderResourceViews is nullptr");
		return;
	}

	// Se llama a la funci�n nativa de Direct3D.
	m_deviceContext->PSSetShaderResources(StartSlot, NumViews, ppShaderResourceViews);
}

//
// `IASetInputLayout` asigna un dise�o de entrada para la etapa de Ensamblador de Entrada (IA).
// Este dise�o le dice a la GPU c�mo interpretar el formato de los datos del b�fer de v�rtices.
//
void
DeviceContext::IASetInputLayout(ID3D11InputLayout* pInputLayout) {
	// Verificaci�n para evitar un puntero nulo.
	if (!pInputLayout) {
		ERROR("DeviceContext", "IASetInputLayout", "pInputLayout is nullptr");
		return;
	}

	// Se llama a la funci�n nativa de Direct3D.
	m_deviceContext->IASetInputLayout(pInputLayout);
}

//
// `VSSetShader` asigna un Vertex Shader activo.
// Este sombreador procesa las posiciones de los v�rtices y otros atributos antes de la rasterizaci�n.
//
void
DeviceContext::VSSetShader(ID3D11VertexShader* pVertexShader,
													ID3D11ClassInstance* const* ppClassInstances,
													unsigned int NumClassInstances) {
	// Verificaci�n para evitar un puntero nulo.
	if (!pVertexShader) {
		ERROR("DeviceContext", "VSSetShader", "pVertexShader is nullptr");
		return;
	}
	// Se llama a la funci�n nativa de Direct3D.
	m_deviceContext->VSSetShader(pVertexShader, 
																ppClassInstances, 
																NumClassInstances);
}

//
// `PSSetShader` asigna un Pixel Shader activo.
// Este sombreador determina el color final de cada p�xel en la pantalla.
//
void
DeviceContext::PSSetShader(ID3D11PixelShader* pPixelShader,
													ID3D11ClassInstance* const* ppClassInstances,
													unsigned int NumClassInstances) {
	// Verificaci�n para evitar un puntero nulo.
	if (!pPixelShader) {
		ERROR("DeviceContext", "PSSetShader", "pPixelShader is nullptr");
		return;
	}

	// Se llama a la funci�n nativa de Direct3D.
	m_deviceContext->PSSetShader(pPixelShader, 
															ppClassInstances, 
															NumClassInstances);
}

//
// `UpdateSubresource` actualiza los datos de un recurso (como un b�fer o una textura) en la GPU.
// Se utiliza para copiar nuevos datos de la memoria del CPU a la memoria de la GPU.
//
void
DeviceContext::UpdateSubresource(ID3D11Resource* pDstResource,
																	unsigned int DstSubresource,
																	const D3D11_BOX* pDstBox,
																	const void* pSrcData,
																	unsigned int SrcRowPitch,
																	unsigned int SrcDepthPitch) {
	// Verificaci�n para evitar punteros nulos.
	if (!pDstResource || !pSrcData) {
		ERROR("DeviceContext", "UpdateSubresource",
			"Invalid arguments: pDstResource or pSrcData is nullptr");
		return;
	}
	// Se llama a la funci�n nativa de Direct3D.
	m_deviceContext->UpdateSubresource(pDstResource,
																		DstSubresource,
																		pDstBox,
																		pSrcData,
																		SrcRowPitch,
																		SrcDepthPitch);
}

//
// `IASetVertexBuffers` asigna b�feres de v�rtices a la etapa de Ensamblador de Entrada.
// Estos b�feres contienen los datos de los v�rtices (posiciones, normales, coordenadas de textura, etc.).
//
void
DeviceContext::IASetVertexBuffers(unsigned int StartSlot,
																	unsigned int NumBuffers,
																	ID3D11Buffer* const* ppVertexBuffers,
																	const unsigned int* pStrides,
																	const unsigned int* pOffsets) {
	// Verificaci�n para evitar punteros nulos.
	if (!ppVertexBuffers || !pStrides || !pOffsets) {
		ERROR("DeviceContext", "IASetVertexBuffers",
			"Invalid arguments: ppVertexBuffers, pStrides, or pOffsets is nullptr");
		return;
	}
	// Se llama a la funci�n nativa de Direct3D.
	m_deviceContext->IASetVertexBuffers(StartSlot,
																			NumBuffers,
																			ppVertexBuffers,
																			pStrides,
																			pOffsets);
}

//
// `IASetIndexBuffer` asigna un b�fer de �ndices a la etapa de Ensamblador de Entrada.
// Un b�fer de �ndices especifica el orden en que se usan los v�rtices para formar primitivas (tri�ngulos, l�neas).
//
void
DeviceContext::IASetIndexBuffer(ID3D11Buffer* pIndexBuffer,
																DXGI_FORMAT Format,
																unsigned int Offset) {
	// Verificaci�n para evitar un puntero nulo.
	if (!pIndexBuffer) {
		ERROR("DeviceContext", "IASetIndexBuffer", "pIndexBuffer is nullptr");
		return;
	}
	// Se llama a la funci�n nativa de Direct3D.
	m_deviceContext->IASetIndexBuffer(pIndexBuffer, 
																		Format, 
																		Offset);
}

//
// `PSSetSamplers` asigna estados de muestreo al Pixel Shader.
// Los samplers controlan c�mo se "muestrean" o leen los datos de las texturas.
//
void
DeviceContext::PSSetSamplers(unsigned int StartSlot,
														unsigned int NumSamplers,
														ID3D11SamplerState* const* ppSamplers) {
	// Verificaci�n para evitar un puntero nulo.
	if (!ppSamplers) {
		ERROR("DeviceContext", "PSSetSamplers", "ppSamplers is nullptr");
		return;
	}
	// Se llama a la funci�n nativa de Direct3D.
	m_deviceContext->PSSetSamplers(StartSlot, NumSamplers, ppSamplers);
}

//
// `RSSetState` asigna un estado de rasterizaci�n.
// Esto controla el comportamiento de la etapa de rasterizaci�n, como el modo de relleno de pol�gonos.
//
void
DeviceContext::RSSetState(ID3D11RasterizerState* pRasterizerState) {
	// Verificaci�n para evitar un puntero nulo.
	if (!pRasterizerState) {
		ERROR("DeviceContext", "RSSetState", "pRasterizerState is nullptr");
		return;
	}
	// Se llama a la funci�n nativa de Direct3D.
	m_deviceContext->RSSetState(pRasterizerState);
}

//
// `OMSetBlendState` asigna un estado de mezcla de salida (Output Merger).
// Esto controla c�mo se mezclan los p�xeles renderizados con los p�xeles que ya existen en el render target.
//
void
DeviceContext::OMSetBlendState(ID3D11BlendState* pBlendState,
	const float BlendFactor[4],
	unsigned int SampleMask) {
	// Verificaci�n para evitar un puntero nulo.
	if (!pBlendState) {
		ERROR("DeviceContext", "OMSetBlendState", "pBlendState is nullptr");
		return;
	}
	// Se llama a la funci�n nativa de Direct3D.
	m_deviceContext->OMSetBlendState(pBlendState, 
																	BlendFactor, 
																	SampleMask);
}

//
// `OMSetRenderTargets` asigna uno o m�s render targets y un buffer de profundidad/plantilla.
// Esto establece d�nde la GPU debe renderizar el resultado de la escena.
//
void
DeviceContext::OMSetRenderTargets(unsigned int NumViews,
																	ID3D11RenderTargetView* const* ppRenderTargetViews,
																	ID3D11DepthStencilView* pDepthStencilView) {
	// Verificaciones de punteros para evitar errores.
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

	// Se llama a la funci�n nativa de Direct3D.
	m_deviceContext->OMSetRenderTargets(NumViews, 
																			ppRenderTargetViews, 
																			pDepthStencilView);
}

//
// `IASetPrimitiveTopology` establece el tipo de primitivas (tri�ngulos, puntos, l�neas) que se dibujar�n.
//
void
DeviceContext::IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY Topology) {
	// Verificaci�n para evitar un tipo de topolog�a indefinido.
	if (Topology == D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED) {
		ERROR("DeviceContext", "IASetPrimitiveTopology",
			"Topology is D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED");
		return;
	}

	// Se llama a la funci�n nativa de Direct3D.
	m_deviceContext->IASetPrimitiveTopology(Topology);
}

//
// `ClearRenderTargetView` limpia el render target con un color s�lido.
// Esto se hace t�picamente al comienzo de cada fotograma para borrar el contenido anterior.
//
void
DeviceContext::ClearRenderTargetView(ID3D11RenderTargetView* pRenderTargetView,
																		const float ColorRGBA[4]) {
	// Verificaciones de punteros para evitar errores.
	if (!pRenderTargetView) {
		ERROR("DeviceContext", "ClearRenderTargetView", "pRenderTargetView is nullptr");
		return;
	}
	if (!ColorRGBA) {
		ERROR("DeviceContext", "ClearRenderTargetView", "ColorRGBA is nullptr");
		return;
	}

	// Se llama a la funci�n nativa de Direct3D.
	m_deviceContext->ClearRenderTargetView(pRenderTargetView, 
																				ColorRGBA);
}

//
// `ClearDepthStencilView` limpia un buffer de profundidad y/o de plantilla.
// Esto es necesario para que el Z-Buffering funcione correctamente en cada fotograma.
//
void
DeviceContext::ClearDepthStencilView(ID3D11DepthStencilView* pDepthStencilView,
	unsigned int ClearFlags,
	float Depth,
	UINT8 Stencil) {
	// Verificaciones de punteros y banderas para evitar errores.
	if (!pDepthStencilView) {
		ERROR("DeviceContext", "ClearDepthStencilView",
			"pDepthStencilView is nullptr");
		return;
	}

	if ((ClearFlags & (D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL)) == 0) {
		ERROR("DeviceContext", "ClearDepthStencilView",
			"Invalid ClearFlags: must include D3D11_CLEAR_DEPTH or D3D11_CLEAR_STENCIL");
		return;
	}

	// Se llama a la funci�n nativa de Direct3D.
	m_deviceContext->ClearDepthStencilView(pDepthStencilView,
																				ClearFlags, Depth, 
																				Stencil);
}

//
// `VSSetConstantBuffers` asigna b�feres de constantes al Vertex Shader.
// Los b�feres de constantes se utilizan para pasar datos de la CPU (como matrices de transformaci�n) a los sombreadores.
//
void
DeviceContext::VSSetConstantBuffers(unsigned int StartSlot,
	unsigned int NumBuffers,
	ID3D11Buffer* const* ppConstantBuffers) {
	// Verificaci�n para evitar un puntero nulo.
	if (!ppConstantBuffers) {
		ERROR("DeviceContext", "VSSetConstantBuffers", "ppConstantBuffers is nullptr");
		return;
	}

	// Se llama a la funci�n nativa de Direct3D.
	m_deviceContext->VSSetConstantBuffers(StartSlot, 
																				NumBuffers,
																				ppConstantBuffers);
}

//
// `PSSetConstantBuffers` asigna b�feres de constantes al Pixel Shader.
// Se usa para pasar datos como la posici�n de las luces, el color del material, etc.
//
void
DeviceContext::PSSetConstantBuffers(unsigned int StartSlot,
	unsigned int NumBuffers,
	ID3D11Buffer* const* ppConstantBuffers) {
	// Verificaci�n para evitar un puntero nulo.
	if (!ppConstantBuffers) {
		ERROR("DeviceContext", "PSSetConstantBuffers", "ppConstantBuffers is nullptr");
		return;
	}
	// Se llama a la funci�n nativa de Direct3D.
	m_deviceContext->PSSetConstantBuffers(StartSlot, 
																				NumBuffers, 
																				ppConstantBuffers);
}

//
// `DrawIndexed` es la funci�n de dibujo principal.
// Le dice a la GPU que dibuje primitivas usando los b�feres de v�rtices e �ndices actualmente asignados.
//
void
DeviceContext::DrawIndexed(unsigned int IndexCount,
													unsigned int StartIndexLocation,
													int BaseVertexLocation) {
	// Verificaci�n para evitar un dibujo sin �ndices.
	if (IndexCount == 0) {
		ERROR("DeviceContext", "DrawIndexed", "IndexCount is zero");
		return;
	}

	// Se llama a la funci�n nativa de Direct3D.
	m_deviceContext->DrawIndexed(IndexCount, 
															StartIndexLocation, 
															BaseVertexLocation);
	++m_drawCallCount;
}