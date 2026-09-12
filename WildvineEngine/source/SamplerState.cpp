#include "SamplerState.h"
#include "Device.h"
#include "DeviceContext.h"

HRESULT
SamplerState::init(Device &device) {
  if (!device.m_device) {
    ERROR("SamplerState", "init", "Device is nullptr");
    return E_POINTER;
  }

  // Create the sample state
  D3D11_SAMPLER_DESC sampDesc = {};
  sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  sampDesc.MinLOD = 0;
  sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

  HRESULT hr = device.CreateSamplerState(&sampDesc, &m_sampler);
  if (FAILED(hr)) {
    ERROR("SamplerState", "init", "Failed to create SamplerState");
    return hr;
  }

  return S_OK;
}

void
SamplerState::update() {
  // No hay logica de actualizacion
}

void
SamplerState::render(DeviceContext &deviceContext,
                     unsigned int StartSlot,
                     unsigned int NumSamplers) {

  if (!m_sampler) {
    ERROR("SamplerState", "render", "SamplerState is nullptr");
    return;
  }

  deviceContext.PSSetSamplers(StartSlot, NumSamplers, &m_sampler);
}

void
SamplerState::destroy() {
  if (m_sampler) {
    SAFE_RELEASE(m_sampler);
  }
}