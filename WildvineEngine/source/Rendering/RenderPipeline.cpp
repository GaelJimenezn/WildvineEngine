/**
 * @file RenderPipeline.cpp
 * @brief Implementa el orquestador de renderers (Forward/Deferred).
 * @ingroup rendering
 */
#include "Rendering/RenderPipeline.h"

HRESULT
RenderPipeline::init(Device& device, RendererType initialRenderer) {
	m_activeRendererType = initialRenderer;
	HRESULT hr = ensureRendererInitialized(initialRenderer, device);
	if (FAILED(hr)) {
		return hr;
	}
	m_activeRenderer = resolveRenderer(initialRenderer);
	return S_OK;
}

HRESULT
RenderPipeline::setRendererType(RendererType rendererType, Device& device) {
	if (rendererType == m_activeRendererType && m_activeRenderer) {
		return S_OK;
	}

	HRESULT hr = ensureRendererInitialized(rendererType, device);
	if (FAILED(hr)) {
		return hr;
	}

	m_activeRendererType = rendererType;
	m_activeRenderer = resolveRenderer(rendererType);
	return S_OK;
}

void
RenderPipeline::resize(Device& device, unsigned int width, unsigned int height) {
	if (m_activeRenderer) {
		m_activeRenderer->resize(device, width, height);
	}
}

void
RenderPipeline::render(DeviceContext& deviceContext,
	const Camera& camera,
	RenderScene& scene,
	EditorViewportPass& viewportPass) {
	if (m_activeRenderer) {
		m_activeRenderer->render(deviceContext, camera, scene, viewportPass);
	}
}

void
RenderPipeline::destroy() {
	if (m_forwardInitialized) {
		m_forwardRenderer.destroy();
		m_forwardInitialized = false;
	}
	if (m_deferredInitialized) {
		m_deferredRenderer.destroy();
		m_deferredInitialized = false;
	}
	m_activeRenderer = nullptr;
}

const char*
RenderPipeline::getActiveRendererName() const {
	const ISceneRenderer* renderer = resolveRenderer(m_activeRendererType);
	return renderer ? renderer->getDebugName() : "None";
}

ID3D11ShaderResourceView*
RenderPipeline::getShadowMapSRV() const {
	return m_activeRenderer ? m_activeRenderer->getShadowMapSRV() : nullptr;
}

ID3D11ShaderResourceView*
RenderPipeline::getPreShadowSRV() const {
	return m_activeRenderer ? m_activeRenderer->getPreShadowSRV() : nullptr;
}

ID3D11ShaderResourceView*
RenderPipeline::getGBufferAlbedoMetallicSRV() const {
	return m_activeRenderer ? m_activeRenderer->getGBufferAlbedoMetallicSRV() : nullptr;
}

ID3D11ShaderResourceView*
RenderPipeline::getGBufferNormalRoughnessSRV() const {
	return m_activeRenderer ? m_activeRenderer->getGBufferNormalRoughnessSRV() : nullptr;
}

ID3D11ShaderResourceView*
RenderPipeline::getGBufferWorldAoSRV() const {
	return m_activeRenderer ? m_activeRenderer->getGBufferWorldAoSRV() : nullptr;
}

ID3D11ShaderResourceView*
RenderPipeline::getGBufferEmissiveAlphaSRV() const {
	return m_activeRenderer ? m_activeRenderer->getGBufferEmissiveAlphaSRV() : nullptr;
}

void
RenderPipeline::setShadowFactorDebugEnabled(bool enabled) {
	if (m_activeRenderer) {
		m_activeRenderer->setShadowFactorDebugEnabled(enabled);
	}
}

HRESULT
RenderPipeline::ensureRendererInitialized(RendererType rendererType, Device& device) {
	if (rendererType == RendererType::Forward) {
		if (!m_forwardInitialized) {
			HRESULT hr = m_forwardRenderer.init(device);
			if (FAILED(hr)) {
				return hr;
			}
			m_forwardInitialized = true;
		}
	}
	else {
		if (!m_deferredInitialized) {
			HRESULT hr = m_deferredRenderer.init(device);
			if (FAILED(hr)) {
				return hr;
			}
			m_deferredInitialized = true;
		}
	}
	return S_OK;
}

ISceneRenderer*
RenderPipeline::resolveRenderer(RendererType rendererType) {
	if (rendererType == RendererType::Forward) {
		return &m_forwardRenderer;
	}
	return &m_deferredRenderer;
}

const ISceneRenderer*
RenderPipeline::resolveRenderer(RendererType rendererType) const {
	if (rendererType == RendererType::Forward) {
		return &m_forwardRenderer;
	}
	return &m_deferredRenderer;
}
