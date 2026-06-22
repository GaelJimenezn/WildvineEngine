/**
 * @file MaterialInstance.cpp
 * @brief Implementa la logica de MaterialInstance dentro del subsistema Rendering.
 * @ingroup rendering
 */
#include "Rendering/MaterialInstance.h"
#include "DeviceContext.h"
#include "Texture.h"

void
MaterialInstance::bindTextures(DeviceContext& deviceContext) const {
	ID3D11ShaderResourceView* nullTextures[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	deviceContext.PSSetShaderResources(0, 6, nullTextures);

	if (m_albedo) {
		m_albedo->render(deviceContext, 0, 1);
	}
	if (m_normal) {
		m_normal->render(deviceContext, 1, 1);
	}
	if (m_metallic) {
		m_metallic->render(deviceContext, 2, 1);
	}
	if (m_roughness) {
		m_roughness->render(deviceContext, 3, 1);
	}
	if (m_ao) {
		m_ao->render(deviceContext, 4, 1);
	}
	if (m_emissive) {
		m_emissive->render(deviceContext, 5, 1);
	}
}