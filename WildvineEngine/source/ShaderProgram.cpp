#include "ShaderProgram.h"
#include "Device.h"
#include "DeviceContext.h"
#include "EngineUtilities\Utilities\LayoutBuilder.h"


HRESULT
ShaderProgram::init(Device& device,
                    const std::string& fileName,
                    LayoutBuilder layoutBuilder) {
  if (!device.m_device) {
    ERROR("ShaderProgram", "init", "Device is null.");
    return E_POINTER;
  }
  if (fileName.empty()) {
    ERROR("ShaderProgram", "init", "File name is empty.");
    return E_INVALIDARG;
  }
 
  m_shaderFileName = fileName;
  // Create the Vertex Shader
  HRESULT hr = CreateShader(device, ShaderType::VERTEX_SHADER);
  if (FAILED(hr)) {
    ERROR("ShaderProgram", "init", "Failed to create vertex shader.");
    return hr;
  }

  // Create the Input Layout
  hr = CreateInputLayout(device, layoutBuilder);
  if (FAILED(hr)) {
    ERROR("ShaderProgram", "init", "Failed to create input layout.");
    return hr;
  }

  // Create the Pixel Shader
  hr = CreateShader(device, ShaderType::PIXEL_SHADER);
  if (FAILED(hr)) {
    ERROR("ShaderProgram", "init", "Failed to create pixel shader.");
    return hr;
  }

  return hr;
}

HRESULT
ShaderProgram::CreateInputLayout(Device& device,
                                 LayoutBuilder layoutBuilder) {
  if (!m_vertexShaderData) {
    ERROR("ShaderProgram", "CreateInputLayout", "Vertex shader data is null.");
    return E_POINTER;
  }
  if (!device.m_device) {
    ERROR("ShaderProgram", "CreateInputLayout", "Device is null.");
    return E_POINTER;
  }

  auto& layout = layoutBuilder.Get();

  HRESULT hr = m_inputLayout.init(device, layout.data(), layout.size(), m_vertexShaderData);
  SAFE_RELEASE(m_vertexShaderData);

  if (FAILED(hr)) {
    ERROR("ShaderProgram", "CreateInputLayout", "Failed to create input layout.");
    return hr;
  }

  return hr;
}

HRESULT
ShaderProgram::CreateShader(Device& device, ShaderType type) {
  if (!device.m_device) {
    ERROR("ShaderProgram", "CreateShader", "Device is null");
    return E_POINTER;
  }
  if (m_shaderFileName.empty()) {
    ERROR("ShaderProgram", "CreateShader", "Shader file name is empty");
    return E_INVALIDARG;
  }

  HRESULT hr = S_OK;
  ID3DBlob* shaderData = nullptr;

  const char* shaderEntryPoint = (type == ShaderType::PIXEL_SHADER) ? "PS" : "VS";
  const char* shaderModel = (type == ShaderType::PIXEL_SHADER) ? "ps_5_0" : "vs_5_0";

  //Compilacion del shader de el archivo
  hr = CompileShaderFromFile(m_shaderFileName.data(),
    shaderEntryPoint,
    shaderModel,
    &shaderData);

  if (FAILED(hr)) {
    ERROR("ShaderProgram", "CreateShader",
      "Failed to compile shader from file: %s", m_shaderFileName.c_str());
    return hr;
  }

  //Creacion de objeto de shader
  if (type == PIXEL_SHADER) {
    hr = device.CreatePixelShader(shaderData->GetBufferPointer(),
                                  shaderData->GetBufferSize(),
                                  nullptr,
                                  &m_PixelShader);
  }
  else {
    hr = device.CreateVertexShader(shaderData->GetBufferPointer(),
                                  shaderData->GetBufferSize(),
                                  nullptr,
                                  &m_VertexShader);
  }

  if (FAILED(hr)) {
    ERROR("ShaderProgram", "CreateShader",
      "Failed to create shader object from compiled data. ");
    shaderData->Release();
    return hr;
  }

  //Store the compiled shader data
  if (type == PIXEL_SHADER) {
    SAFE_RELEASE(m_pixelShaderData);
    m_pixelShaderData = shaderData;
  }
  else {
    SAFE_RELEASE(m_vertexShaderData);
    m_vertexShaderData = shaderData;
  }

return S_OK;
}

HRESULT
ShaderProgram::CreateShader(Device& device,
  ShaderType type,
  const std::string& fileName) {

  if (!device.m_device) {
    ERROR("ShaderProgram", "init", "Device is null");
    return E_POINTER;
  }
  if (fileName.empty()) {
    ERROR("ShaderProgram", "init", "File name is empty. ");
    return E_INVALIDARG;
  }
  m_shaderFileName = fileName;

  HRESULT hr = CreateShader(device, type);

  if (FAILED(hr)) {
    ERROR("ShaderProgram", "CreateShader",
      "Failed to Create shader from file: %s", m_shaderFileName.c_str());
    return hr;
  }

  return S_OK;
}

HRESULT
ShaderProgram::CompileShaderFromFile(char* szFileName,
                                    LPCSTR szEntryPoint,
                                    LPCSTR szShaderModel,
                                    ID3DBlob** ppBlobOut) {
  HRESULT hr = S_OK;

  DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined (DEBUG) || defined (_DEBUG)

  dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
  ID3DBlob* pErrorBlob = nullptr;
  hr = D3DX11CompileFromFile(szFileName, 
                            nullptr, 
                            nullptr, 
                            szEntryPoint, 
                            szShaderModel,
                            dwShaderFlags, 
                            0, 
                            nullptr,
                            ppBlobOut,
                            &pErrorBlob,
                            nullptr);

  if (FAILED(hr)) {
    if (pErrorBlob) {
      // 1. Imprimimos el error crudo directamente en la consola de Visual Studio
      OutputDebugStringA("\n================ ERROR DE HLSL ================\n");
      OutputDebugStringA(static_cast<const char*>(pErrorBlob->GetBufferPointer()));
      OutputDebugStringA("\n===============================================\n");

      ERROR("ShaderProgram", "CompileShaderFromFile", "Failed to compile shader (revisa la consola arriba)");

      pErrorBlob->Release();
    }
    else {
      ERROR("ShaderProgram", "CompileShaderFromFile", "Failed to compile shader. No error message available");
    }
    return hr;
  }

  SAFE_RELEASE(pErrorBlob)

  return S_OK;
}

void
ShaderProgram::render(DeviceContext& deviceContext) {
  if (!m_VertexShader || !m_PixelShader || !m_inputLayout.m_inputLayout) {
    ERROR("ShaderProgram", "render", "Shader or InputLayout not initialized");
    return;
  }

  m_inputLayout.render(deviceContext);
  deviceContext.m_deviceContext->VSSetShader(m_VertexShader, nullptr, 0);
  deviceContext.m_deviceContext->PSSetShader(m_PixelShader, nullptr, 0);
}

void
ShaderProgram::render(DeviceContext& deviceContext, ShaderType type) {
  if (!deviceContext.m_deviceContext) {
    ERROR("RenderTargetView", "render", "DeviceContext is nullptr");
    return;
  }
  switch (type) {
  case VERTEX_SHADER:
    deviceContext.m_deviceContext->VSSetShader(m_VertexShader, nullptr, 0);
    break;
  case PIXEL_SHADER:
    deviceContext.m_deviceContext->PSSetShader(m_PixelShader, nullptr, 0);
    break;
  default:
    break;
  }
}

void
ShaderProgram::destroy() {
  SAFE_RELEASE(m_VertexShader);
  m_inputLayout.destroy();
  SAFE_RELEASE(m_PixelShader);
  SAFE_RELEASE(m_vertexShaderData);
  SAFE_RELEASE(m_pixelShaderData);
}