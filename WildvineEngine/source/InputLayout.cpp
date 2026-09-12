#include "InputLayout.h"
#include "Device.h"
#include "DeviceContext.h"

HRESULT
InputLayout::init(Device &device,
                  const D3D11_INPUT_ELEMENT_DESC *layoutDesc,
                  UINT layoutCount,
                  ID3DBlob *vertexShaderData) {
  if (!layoutDesc || layoutCount == 0) {
    ERROR("InputLayout", "init", "Layout descriptor is empty.");
    return E_INVALIDARG;
  }

  if (!vertexShaderData) {
    ERROR("InputLayout", "init", "VertexShaderData is nullptr.");
    return E_POINTER;
  }

  HRESULT hr = device.CreateInputLayout(layoutDesc,
                                        layoutCount,
                                        vertexShaderData->GetBufferPointer(),
                                        vertexShaderData->GetBufferSize(),
                                        &m_inputLayout);

  if (FAILED(hr)) {
    ERROR("InputLayout",
          "init",
          ("Failed to create InputLayout. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  return S_OK;
}

void
InputLayout::update() {
  // Método vacío, se puede utilizar en caso de necesitar cambios dinámicos en el layout
}

void
InputLayout::render(DeviceContext &deviceContext) {
  if (!m_inputLayout) {
    ERROR("InputLayout", "render", "InputLayout is nullptr");
    return;
  }

  deviceContext.m_deviceContext->IASetInputLayout(m_inputLayout);
}

void
InputLayout::destroy() {
  SAFE_RELEASE(m_inputLayout);
}