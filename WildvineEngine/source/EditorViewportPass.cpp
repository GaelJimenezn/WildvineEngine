#include "EngineUtilities\Utilities\EditorViewportPass.h"
#include "Device.h"
#include "DeviceContext.h"

// Inicializa el viewport creando todos los recursos necesarios (color + depth)
HRESULT
EditorViewportPass::init(Device &device, unsigned int width, unsigned int height) {
  return createResources(device, width, height);
}

// Redimensiona el viewport, recreando recursos solo si realmente cambió el tamaño
HRESULT
EditorViewportPass::resize(Device &device, unsigned int width, unsigned int height) {
  // Evita tamaños inválidos o demasiado pequeños (protección mínima)
  if (width < 64)
    width = 64;
  if (height < 64)
    height = 64;

  // Si el tamaño no cambió y los recursos siguen válidos, no hace nada
  if (width == m_width && height == m_height && isValid())
    return S_OK;

  // Recrea todos los recursos con el nuevo tamaño
  return createResources(device, width, height);
}

// Crea todos los recursos GPU necesarios para renderizar el viewport offscreen
HRESULT
EditorViewportPass::createResources(Device &device,
                                    unsigned int width,
                                    unsigned int height) {
  // Libera cualquier recurso previo antes de crear nuevos
  destroy();

  // Seguridad básica contra valores inválidos
  if (width == 0)
    width = 1;
  if (height == 0)
    height = 1;

  m_width = width;
  m_height = height;

  HRESULT
  hr = S_OK;

  // 1) Textura de color donde se renderiza la escena (offscreen render target)
  hr = m_colorTexture.init(device,
                           width,
                           height,
                           DXGI_FORMAT_R8G8B8A8_UNORM,
                           D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
                           1,
                           0);

  if (FAILED(hr))
    return hr;

  // 2) Render Target View para poder escribir en la textura desde el pipeline
  hr = m_rtv.init(
      device, m_colorTexture, D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM);

  if (FAILED(hr))
    return hr;

  // 3) Shader Resource View separado para poder leer la textura (ej. en ImGui)
  hr = m_colorSRV.init(device, m_colorTexture, DXGI_FORMAT_R8G8B8A8_UNORM);

  if (FAILED(hr))
    return hr;

  // 4) Textura de profundidad para test de profundidad durante el render
  hr = m_depthTexture.init(device,
                           width,
                           height,
                           DXGI_FORMAT_D24_UNORM_S8_UINT,
                           D3D11_BIND_DEPTH_STENCIL,
                           1,
                           0);

  if (FAILED(hr))
    return hr;

  // 5) Depth Stencil View para usar la textura de profundidad en el pipeline
  hr = m_dsv.init(device,
                  m_depthTexture,
                  DXGI_FORMAT_D24_UNORM_S8_UINT,
                  D3D11_DSV_DIMENSION_TEXTURE2D);

  if (FAILED(hr))
    return hr;

  return S_OK;
}

// Inicia el pass de render: limpia y establece render target + depth
void
EditorViewportPass::begin(DeviceContext &deviceContext, const float clearColor[4]) {
  m_rtv.render(deviceContext, m_dsv, 1, clearColor);
}

// Intercambia todos los recursos con otro viewport (útil para double buffering o
// ping-pong)
void
EditorViewportPass::swap(EditorViewportPass &other) {
  std::swap(m_colorTexture, other.m_colorTexture);
  std::swap(m_colorSRV, other.m_colorSRV);
  std::swap(m_rtv, other.m_rtv);
  std::swap(m_depthTexture, other.m_depthTexture);
  std::swap(m_dsv, other.m_dsv);
  std::swap(m_width, other.m_width);
  std::swap(m_height, other.m_height);
}

// Limpia únicamente el buffer de profundidad (útil entre passes)
void
EditorViewportPass::clearDepth(DeviceContext &deviceContext) {
  m_dsv.render(deviceContext);
}

// Configura el viewport en el pipeline (dimensiones donde se dibuja)
void
EditorViewportPass::setViewport(DeviceContext &deviceContext) {
  D3D11_VIEWPORT vp{};
  vp.TopLeftX = 0.0f;
  vp.TopLeftY = 0.0f;
  vp.Width = static_cast<float>(m_width);
  vp.Height = static_cast<float>(m_height);
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;

  deviceContext.m_deviceContext->RSSetViewports(1, &vp);
}

void
EditorViewportPass::bind(DeviceContext &deviceContext) {
  ID3D11RenderTargetView *rtv = m_rtv.getNativeView();
  deviceContext.m_deviceContext->OMSetRenderTargets(1, &rtv, m_dsv.getNativeView());
}

// Libera todos los recursos GPU asociados a este viewport
void
EditorViewportPass::destroy() {
  m_dsv.destroy();
  m_depthTexture.destroy();
  m_colorSRV.destroy();
  m_rtv.destroy();
  m_colorTexture.destroy();

  // Reset a valores por defecto seguros
  m_width = 1;
  m_height = 1;
}
