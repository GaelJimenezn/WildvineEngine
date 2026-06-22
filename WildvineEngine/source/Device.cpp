#include "Device.h"

//
// La función `destroy` se encarga de liberar el objeto principal de Direct3D, el ID3D11Device.
// Esto es crucial para liberar todos los recursos de la GPU cuando la aplicación termina.
//
void
Device::destroy() {
	SAFE_RELEASE(m_device);
}

//
// `CreateRenderTargetView` es una función wrapper que crea un Render Target View.
// Un Render Target View es una "vista" de un recurso (como una textura) que se puede usar como destino para renderizar.
//
HRESULT
Device::CreateRenderTargetView(ID3D11Resource* pResource,
															const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
															ID3D11RenderTargetView** ppRTView) {
	// Se valida que los punteros de entrada no sean nulos.
	if (!pResource) {
		ERROR("Device", "CreateRenderTargetView", "pResource is nullptr");
		return E_INVALIDARG;
	}
	if (!ppRTView) {
		ERROR("Device", "CreateRenderTargetView", "ppRTView is nullptr");
		return E_POINTER;
	}

	// Se llama a la función de Direct3D para crear la vista.
	HRESULT hr = m_device->CreateRenderTargetView(pResource, 
																								pDesc, 
																								ppRTView);

	// Se comprueba el resultado y se muestra un mensaje apropiado.
	if (SUCCEEDED(hr)) {
		MESSAGE("Device", "CreateRenderTargetView",
			"Render Target View created successfully!");
	}
	else {
		ERROR("Device", "CreateRenderTargetView",
			("Failed to create Render Target View. HRESULT: " + std::to_string(hr)).c_str());
	}

	return hr;
}

//
// `CreateTexture2D` crea una textura 2D en la memoria de la GPU.
// Las texturas se usan para almacenar imágenes, buffers de renderizado, y más.
//
HRESULT
Device::CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc,
												const D3D11_SUBRESOURCE_DATA* pInitialData,
												ID3D11Texture2D** ppTexture2D) {
	// Se valida que los punteros de entrada no sean nulos.
	if (!pDesc) {
		ERROR("Device", "CreateTexture2D", "pDesc is nullptr");
		return E_INVALIDARG;
	}
	if (!ppTexture2D) {
		ERROR("Device", "CreateTexture2D", "ppTexture2D is nullptr");
		return E_POINTER;
	}

	// Se llama a la función de Direct3D para crear la textura.
	HRESULT hr = m_device->CreateTexture2D(pDesc, 
																				pInitialData,
																				ppTexture2D);

	// Se comprueba el resultado y se muestra un mensaje.
	if (SUCCEEDED(hr)) {
		MESSAGE("Device", "CreateTexture2D",
			"Texture2D created successfully!");
	}
	else {
		ERROR("Device", "CreateTexture2D",
			("Failed to create Texture2D. HRESULT: " + std::to_string(hr)).c_str());
	}

	return hr;
}

//
// `CreateDepthStencilView` crea una vista para un buffer de profundidad y de plantilla.
// Este buffer es esencial para el Z-Buffering, que ayuda a determinar qué polígonos son visibles.
//
HRESULT
Device::CreateDepthStencilView(ID3D11Resource* pResource,
															const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc,
															ID3D11DepthStencilView** ppDepthStencilView) {
	// Se valida que los punteros de entrada no sean nulos.
	if (!pResource) {
		ERROR("Device", "CreateDepthStencilView", "pResource is nullptr");
		return E_INVALIDARG;
	}
	if (!ppDepthStencilView) {
		ERROR("Device", "CreateDepthStencilView", "ppDepthStencilView is nullptr");
		return E_POINTER;
	}

	// Se llama a la función de Direct3D para crear la vista.
	HRESULT hr = m_device->CreateDepthStencilView(pResource,
																								pDesc, 
																								ppDepthStencilView);

	// Se comprueba el resultado y se muestra un mensaje.
	if (SUCCEEDED(hr)) {
		MESSAGE("Device", "CreateDepthStencilView",
			"Depth Stencil View created successfully!");
	}
	else {
		ERROR("Device", "CreateDepthStencilView",
			("Failed to create Depth Stencil View. HRESULT: " + std::to_string(hr)).c_str());
	}

	return hr;
}

//
// `CreateVertexShader` crea un sombreador de vértices.
// Este programa se ejecuta en la GPU para procesar cada vértice de los modelos 3D.
//
HRESULT
Device::CreateVertexShader(const void* pShaderBytecode,
													unsigned int BytecodeLength,
													ID3D11ClassLinkage* pClassLinkage,
													ID3D11VertexShader** ppVertexShader) {
	// Se valida que los punteros de entrada no sean nulos.
	if (!pShaderBytecode) {
		ERROR("Device", "CreateVertexShader", "pShaderBytecode is nullptr");
		return E_INVALIDARG;
	}
	if (!ppVertexShader) {
		ERROR("Device", "CreateVertexShader", "ppVertexShader is nullptr");
		return E_POINTER;
	}

	// Se llama a la función de Direct3D para crear el sombreador.
	HRESULT hr = m_device->CreateVertexShader(pShaderBytecode,
																						BytecodeLength,
																						pClassLinkage,
																						ppVertexShader);

	// Se comprueba el resultado y se muestra un mensaje.
	if (SUCCEEDED(hr)) {
		MESSAGE("Device", "CreateVertexShader",
			"Vertex Shader created successfully!");
	}
	else {
		ERROR("Device", "CreateVertexShader",
			("Failed to create Vertex Shader. HRESULT: " + std::to_string(hr)).c_str());
	}

	return hr;
}

//
// `CreateInputLayout` crea una descripción del formato de los datos de los vértices.
// Esto le dice a la tarjeta gráfica cómo interpretar la información de los vértices (posición, color, etc.).
//
HRESULT
Device::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs,
													unsigned int NumElements,
													const void* pShaderBytecodeWithInputSignature,
													unsigned int BytecodeLength,
													ID3D11InputLayout** ppInputLayout) {
	// Se valida que los punteros de entrada no sean nulos.
	if (!pInputElementDescs) {
		ERROR("Device", "CreateInputLayout", "pInputElementDescs is nullptr");
		return E_INVALIDARG;
	}
	if (!ppInputLayout) {
		ERROR("Device", "CreateInputLayout", "ppInputLayout is nullptr");
		return E_POINTER;
	}

	// Se llama a la función de Direct3D para crear el layout.
	HRESULT
		hr = m_device->CreateInputLayout(pInputElementDescs,
																		NumElements,
																		pShaderBytecodeWithInputSignature,
																		BytecodeLength,
																		ppInputLayout);

	// Se comprueba el resultado y se muestra un mensaje.
	if (SUCCEEDED(hr)) {
		MESSAGE("Device", "CreateInputLayout",
			"Input Layout created successfully!");
	}
	else {
		ERROR("Device", "CreateInputLayout",
			("Failed to create Input Layout. HRESULT: " + std::to_string(hr)).c_str());
	}

	return hr;
}

//
// `CreatePixelShader` crea un sombreador de píxeles.
// Este programa se ejecuta en la GPU para determinar el color de cada píxel de un objeto.
//
HRESULT
Device::CreatePixelShader(const void* pShaderBytecode,
													unsigned int BytecodeLength,
													ID3D11ClassLinkage* pClassLinkage,
													ID3D11PixelShader** ppPixelShader) {
	// Se valida que los punteros de entrada no sean nulos.
	if (!pShaderBytecode) {
		ERROR("Device", "CreatePixelShader", "pShaderBytecode is nullptr");
		return E_INVALIDARG;
	}
	if (!ppPixelShader) {
		ERROR("Device", "CreatePixelShader", "ppPixelShader is nullptr");
		return E_POINTER;
	}

	// Se llama a la función de Direct3D para crear el sombreador.
	HRESULT hr = m_device->CreatePixelShader(pShaderBytecode,
																					BytecodeLength,
																					pClassLinkage,
																					ppPixelShader);

	// Se comprueba el resultado y se muestra un mensaje.
	if (SUCCEEDED(hr)) {
		MESSAGE("Device", "CreatePixelShader",
			"Pixel Shader created successfully!");
	}
	else {
		ERROR("Device", "CreatePixelShader",
			("Failed to create Pixel Shader. HRESULT: " + std::to_string(hr)).c_str());
	}

	return hr;
}

//
// `CreateSamplerState` crea un estado de muestreo.
// Los estados de muestreo controlan cómo se lee la información de las texturas (por ejemplo, cómo se filtra).
//
HRESULT
Device::CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc,
	ID3D11SamplerState** ppSamplerState) {
	// Se valida que los punteros de entrada no sean nulos.
	if (!pSamplerDesc) {
		ERROR("Device", "CreateSamplerState", "pSamplerDesc is nullptr");
		return E_INVALIDARG;
	}
	if (!ppSamplerState) {
		ERROR("Device", "CreateSamplerState", "ppSamplerState is nullptr");
		return E_POINTER;
	}

	// Se llama a la función de Direct3D para crear el estado de muestreo.
	HRESULT hr = m_device->CreateSamplerState(pSamplerDesc, ppSamplerState);

	// Se comprueba el resultado y se muestra un mensaje.
	if (SUCCEEDED(hr)) {
		MESSAGE("Device", "CreateSamplerState",
			"Sampler State created successfully!");
	}
	else {
		ERROR("Device", "CreateSamplerState",
			("Failed to create Sampler State. HRESULT: " + std::to_string(hr)).c_str());
	}

	return hr;
}

//
// `CreateBuffer` crea un buffer en la memoria de la GPU.
// Los buffers se usan para almacenar vértices, índices, constantes, y otros datos que necesita la GPU.
//
HRESULT
Device::CreateBuffer(const D3D11_BUFFER_DESC* pDesc,
										const D3D11_SUBRESOURCE_DATA* pInitialData,
										ID3D11Buffer** ppBuffer) {
	// Se valida que los punteros de entrada no sean nulos.
	if (!pDesc) {
		ERROR("Device", "CreateBuffer", "pDesc is nullptr");
		return E_INVALIDARG;
	}
	if (!ppBuffer) {
		ERROR("Device", "CreateBuffer", "ppBuffer is nullptr");
		return E_POINTER;
	}

	// Se llama a la función de Direct3D para crear el buffer.
	HRESULT hr = m_device->CreateBuffer(pDesc, pInitialData, ppBuffer);

	// Se comprueba el resultado y se muestra un mensaje.
	if (SUCCEEDED(hr)) {
		MESSAGE("Device", "CreateBuffer",
			"Buffer created successfully!");
	}
	else {
		ERROR("Device", "CreateBuffer",
			("Failed to create Buffer. HRESULT: " + std::to_string(hr)).c_str());

	}
	return hr;
}