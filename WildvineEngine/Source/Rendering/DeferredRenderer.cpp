/**
 * @file DeferredRenderer.cpp
 * @brief Implementa la logica de DeferredRenderer dentro del subsistema Rendering.
 * @ingroup rendering
 */
#include "Rendering/DeferredRenderer.h"
#include <algorithm>
#include <cmath>
#include "Device.h"
#include "DeviceContext.h"
#include "EngineUtilities/Utilities/Camera.h"
#include "EngineUtilities/Utilities/LayoutBuilder.h"
#include "EngineUtilities/Utilities/Skybox.h"
#include "Rendering/Material.h"
#include "Rendering/MaterialInstance.h"
#include "Rendering/Mesh.h"

namespace {
	constexpr unsigned int kGBufferTargetCount = 4;

	struct RenderTargetViewAccess {
		ID3D11RenderTargetView* m_renderTargetView = nullptr;
	};

	struct EditorViewportPassAccess {
		Texture m_colorTexture;
		Texture m_colorSRV;
		RenderTargetView m_rtv;
		Texture m_depthTexture;
		DepthStencilView m_dsv;
		unsigned int m_width = 1;
		unsigned int m_height = 1;
	};

	ID3D11RenderTargetView* ResolveViewportRTV(EditorViewportPass& pass) {
		EditorViewportPassAccess* access = reinterpret_cast<EditorViewportPassAccess*>(&pass);
		RenderTargetViewAccess* rtvAccess = reinterpret_cast<RenderTargetViewAccess*>(&access->m_rtv);
		return rtvAccess->m_renderTargetView;
	}

	ID3D11DepthStencilView* ResolveViewportDSV(EditorViewportPass& pass) {
		EditorViewportPassAccess* access = reinterpret_cast<EditorViewportPassAccess*>(&pass);
		return access->m_dsv.m_depthStencilView;
	}

	ID3D11RenderTargetView* ResolveRTV(RenderTargetView& view) {
		RenderTargetViewAccess* access = reinterpret_cast<RenderTargetViewAccess*>(&view);
		return access->m_renderTargetView;
	}

	const LightData*
		findPrimaryShadowLight(const RenderScene& scene) {
		for (const LightData& light : scene.directionalLights) {
			if (light.type == LightType::Directional) {
				return &light;
			}
		}

		return scene.directionalLights.empty() ? nullptr : &scene.directionalLights.front();
	}

	void
		writeLightToFrameBuffer(CBPerFrame& buffer, int lightIndex, const LightData& light) {
		const float range = light.range > 0.0f ? light.range : 10.0f;
		const EU::Vector3 lightColor = light.color * light.intensity;
		buffer.LightPositionsRanges[lightIndex] = XMFLOAT4(light.position.x, light.position.y, light.position.z, range);
		buffer.LightColorsTypes[lightIndex] = XMFLOAT4(lightColor.x, lightColor.y, lightColor.z, static_cast<float>(static_cast<int>(light.type)));
		buffer.LightDirectionsIntensities[lightIndex] = XMFLOAT4(light.direction.x, light.direction.y, light.direction.z, light.intensity);
	}
}

HRESULT
DeferredRenderer::init(Device& device) {
	HRESULT hr = m_perFrameBuffer.init(device, sizeof(CBPerFrame));
	if (FAILED(hr)) {
		return hr;
	}

	hr = m_perObjectBuffer.init(device, sizeof(CBPerObject));
	if (FAILED(hr)) {
		return hr;
	}

	hr = m_perMaterialBuffer.init(device, sizeof(CBPerMaterial));
	if (FAILED(hr)) {
		return hr;
	}

	hr = m_lightingDebugBuffer.init(device, sizeof(DeferredLightingDebugData));
	if (FAILED(hr)) {
		return hr;
	}

	hr = m_transparentDepthStencil.init(device,
		true,
		D3D11_DEPTH_WRITE_MASK_ZERO,
		D3D11_COMPARISON_LESS_EQUAL);
	if (FAILED(hr)) {
		return hr;
	}

	hr = m_disabledDepthStencil.init(device,
		false,
		D3D11_DEPTH_WRITE_MASK_ZERO,
		D3D11_COMPARISON_ALWAYS);
	if (FAILED(hr)) {
		return hr;
	}

	hr = m_shadowDepthStencil.init(device,
		true,
		D3D11_DEPTH_WRITE_MASK_ALL,
		D3D11_COMPARISON_LESS);
	if (FAILED(hr)) {
		return hr;
	}

	hr = createShadowResources(device);
	if (FAILED(hr)) {
		return hr;
	}

	hr = m_preShadowDebugPass.init(device, m_renderWidth, m_renderHeight);
	if (FAILED(hr)) {
		return hr;
	}

	hr = createGBufferResources(device, m_renderWidth, m_renderHeight);
	if (FAILED(hr)) {
		return hr;
	}

	hr = createLightingResources(device);
	if (FAILED(hr)) {
		return hr;
	}

	hr = createFullScreenQuad(device);
	if (FAILED(hr)) {
		return hr;
	}

	hr = createBlendStates(device);
	if (FAILED(hr)) {
		return hr;
	}

	return S_OK;
}

void
DeferredRenderer::resize(Device& device, unsigned int width, unsigned int height) {
	if (width < 64) width = 64;
	if (height < 64) height = 64;

	m_renderWidth = width;
	m_renderHeight = height;
	m_preShadowDebugPass.resize(device, width, height);
	createGBufferResources(device, width, height);
}

void
DeferredRenderer::render(DeviceContext& deviceContext,
	const Camera& camera,
	RenderScene& scene,
	EditorViewportPass& viewportPass) {
	buildQueues(scene, camera);
	updatePerFrame(camera, scene, deviceContext);

	renderSceneToTarget(deviceContext, scene, m_preShadowDebugPass, false);
	renderShadowPass(deviceContext);
	renderSceneToTarget(deviceContext, scene, viewportPass, true);
}

void
DeferredRenderer::destroy() {
	m_opaqueQueue.clear();
	m_transparentQueue.clear();

	SAFE_RELEASE(m_alphaBlendState);
	SAFE_RELEASE(m_opaqueBlendState);
	SAFE_RELEASE(m_additiveBlendState);
	SAFE_RELEASE(m_premultipliedBlendState);

	m_fullscreenIndexBuffer.destroy();
	m_fullscreenVertexBuffer.destroy();

	m_gBufferEmissiveAlphaRTV.destroy();
	m_gBufferEmissiveAlphaSRV.destroy();
	m_gBufferEmissiveAlphaTexture.destroy();
	m_gBufferWorldAoRTV.destroy();
	m_gBufferWorldAoSRV.destroy();
	m_gBufferWorldAoTexture.destroy();
	m_gBufferNormalRoughnessRTV.destroy();
	m_gBufferNormalRoughnessSRV.destroy();
	m_gBufferNormalRoughnessTexture.destroy();
	m_gBufferAlbedoMetallicRTV.destroy();
	m_gBufferAlbedoMetallicSRV.destroy();
	m_gBufferAlbedoMetallicTexture.destroy();

	m_fullscreenRasterizer.destroy();
	m_lightingSampler.destroy();
	m_deferredLightingShader.destroy();
	m_gBufferShader.destroy();

	m_transparentDepthStencil.destroy();
	m_disabledDepthStencil.destroy();
	m_shadowDepthStencil.destroy();
	m_perMaterialBuffer.destroy();
	m_lightingDebugBuffer.destroy();
	m_perObjectBuffer.destroy();
	m_perFrameBuffer.destroy();

	m_shadowRasterizer.destroy();
	m_shadowShader.destroy();
	m_shadowDSV.destroy();
	m_shadowDepthSRV.destroy();
	m_shadowDepthTexture.destroy();
	m_preShadowDebugPass.destroy();
}

void
DeferredRenderer::buildQueues(RenderScene& scene, const Camera& camera) {
	(void)camera;
	m_opaqueQueue.clear();
	m_transparentQueue.clear();

	for (auto& object : scene.opaqueObjects) {
		m_opaqueQueue.push_back(&object);
	}

	for (auto& object : scene.transparentObjects) {
		m_transparentQueue.push_back(&object);
	}

	std::sort(m_opaqueQueue.begin(), m_opaqueQueue.end(),
		[](const RenderObject* lhs, const RenderObject* rhs) {
			if (lhs->materialInstance != rhs->materialInstance) {
				return lhs->materialInstance < rhs->materialInstance;
			}
			return lhs->distanceToCamera < rhs->distanceToCamera;
		});

	std::sort(m_transparentQueue.begin(), m_transparentQueue.end(),
		[](const RenderObject* lhs, const RenderObject* rhs) {
			return lhs->distanceToCamera > rhs->distanceToCamera;
		});
}

void
DeferredRenderer::updatePerFrame(const Camera& camera,
	const RenderScene& scene,
	DeviceContext& deviceContext) {
	updateLightMatrices(camera, scene);
	XMStoreFloat4x4(&m_cbPerFrame.View, XMMatrixTranspose(camera.getView()));
	XMStoreFloat4x4(&m_cbPerFrame.Projection, XMMatrixTranspose(camera.getProj()));
	m_cbPerFrame.CameraPos = camera.getPosition();
	m_cbPerFrame.LightDir = EU::Vector3(0.0f, -1.0f, 0.0f);
	m_cbPerFrame.LightColor = EU::Vector3(1.0f, 1.0f, 1.0f);
	m_cbPerFrame.LightPosition = EU::Vector3(0.0f, 3.0f, 0.0f);
	m_cbPerFrame.LightRange = 10.0f;
	m_cbPerFrame.LightType = static_cast<int>(LightType::Directional);
	m_cbPerFrame.LightCount = 0;
	for (int lightIndex = 0; lightIndex < kMaxSceneLights; ++lightIndex) {
		m_cbPerFrame.LightPositionsRanges[lightIndex] = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		m_cbPerFrame.LightColorsTypes[lightIndex] = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		m_cbPerFrame.LightDirectionsIntensities[lightIndex] = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
	}

	if (!scene.directionalLights.empty()) {
		const LightData* primaryShadowLight = findPrimaryShadowLight(scene);
		int lightCount = 0;
		if (primaryShadowLight) {
			writeLightToFrameBuffer(m_cbPerFrame, lightCount++, *primaryShadowLight);
		}
		for (const LightData& light : scene.directionalLights) {
			if (&light == primaryShadowLight || lightCount >= kMaxSceneLights) {
				continue;
			}
			writeLightToFrameBuffer(m_cbPerFrame, lightCount++, light);
		}
		m_cbPerFrame.LightCount = lightCount;

		const LightData& mainLight = primaryShadowLight ? *primaryShadowLight : scene.directionalLights[0];
		m_cbPerFrame.LightDir = mainLight.direction;
		m_cbPerFrame.LightColor = mainLight.color * mainLight.intensity;
		m_cbPerFrame.LightPosition = mainLight.position;
		m_cbPerFrame.LightRange = mainLight.range > 0.0f ? mainLight.range : 10.0f;
		m_cbPerFrame.LightType = static_cast<int>(mainLight.type);
	}

	m_perFrameBuffer.update(deviceContext, nullptr, 0, nullptr, &m_cbPerFrame, 0, 0);
}

void
DeferredRenderer::updateLightMatrices(const Camera& camera, const RenderScene& scene) {
	EU::Vector3 lightDir = EU::Vector3(0.0f, -1.0f, 0.0f);
	const LightData* primaryShadowLight = findPrimaryShadowLight(scene);
	if (primaryShadowLight) {
		lightDir = primaryShadowLight->direction;
	}

	XMVECTOR lightDirVec = XMVector3Normalize(XMVectorSet(lightDir.x, lightDir.y, lightDir.z, 0.0f));
	XMVECTOR cameraPos = XMVectorSet(camera.getPosition().x, camera.getPosition().y, camera.getPosition().z, 1.0f);
	XMVECTOR lightTarget = cameraPos;
	XMVECTOR lightEye = XMVectorSubtract(lightTarget, XMVectorScale(lightDirVec, 35.0f));
	XMVECTOR worldUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	if (fabsf(XMVectorGetX(XMVector3Dot(lightDirVec, worldUp))) > 0.98f) {
		worldUp = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	}

	XMMATRIX lightView = XMMatrixLookAtLH(lightEye, lightTarget, worldUp);
	XMMATRIX lightProjection = XMMatrixOrthographicLH(40.0f, 40.0f, 1.0f, 80.0f);
	XMStoreFloat4x4(&m_cbPerFrame.LightViewProjection, XMMatrixTranspose(lightView * lightProjection));
}

void
DeferredRenderer::renderSceneToTarget(DeviceContext& deviceContext,
	RenderScene& scene,
	EditorViewportPass& targetPass,
	bool applyShadows) {
	const float clearColor[4] = { 0.10f, 0.10f, 0.10f, 1.0f };

	targetPass.begin(deviceContext, clearColor);
	targetPass.setViewport(deviceContext);

	m_applyShadows = applyShadows;
	bindGBufferTargets(deviceContext, ResolveViewportDSV(targetPass));
	renderGeometryPass(deviceContext);

	bindFinalTarget(deviceContext, ResolveViewportRTV(targetPass), ResolveViewportDSV(targetPass));
	renderLightingPass(deviceContext);
	renderSkyboxPass(deviceContext, scene);
	renderTransparentPass(deviceContext);
}

void
DeferredRenderer::bindGBufferTargets(DeviceContext& deviceContext, ID3D11DepthStencilView* depthStencilView) {
	clearDeferredSRVs(deviceContext);

	ID3D11RenderTargetView* renderTargets[kGBufferTargetCount] = {
		ResolveRTV(m_gBufferAlbedoMetallicRTV),
		ResolveRTV(m_gBufferNormalRoughnessRTV),
		ResolveRTV(m_gBufferWorldAoRTV),
		ResolveRTV(m_gBufferEmissiveAlphaRTV)
	};

	const float clear0[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	const float clear1[4] = { 0.5f, 0.5f, 1.0f, 0.0f };
	const float clear2[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	const float clear3[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	deviceContext.ClearRenderTargetView(renderTargets[0], clear0);
	deviceContext.ClearRenderTargetView(renderTargets[1], clear1);
	deviceContext.ClearRenderTargetView(renderTargets[2], clear2);
	deviceContext.ClearRenderTargetView(renderTargets[3], clear3);
	deviceContext.ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	deviceContext.OMSetRenderTargets(kGBufferTargetCount, renderTargets, depthStencilView);
	deviceContext.OMSetBlendState(m_opaqueBlendState, m_blendFactor, 0xffffffff);
}

void
DeferredRenderer::bindFinalTarget(DeviceContext& deviceContext,
	ID3D11RenderTargetView* renderTargetView,
	ID3D11DepthStencilView* depthStencilView) {
	ID3D11RenderTargetView* renderTargets[1] = { renderTargetView };
	deviceContext.OMSetRenderTargets(1, renderTargets, depthStencilView);
	deviceContext.OMSetBlendState(m_opaqueBlendState, m_blendFactor, 0xffffffff);
}

void
DeferredRenderer::clearDeferredSRVs(DeviceContext& deviceContext) {
	ID3D11ShaderResourceView* nullViews[8] = {};
	deviceContext.PSSetShaderResources(0, 8, nullViews);
}

void
DeferredRenderer::renderGeometryPass(DeviceContext& deviceContext) {
	m_perFrameBuffer.render(deviceContext, 0, 1, false);

	for (const RenderObject* object : m_opaqueQueue) {
		if (!object) {
			continue;
		}
		renderGeometryObject(deviceContext, *object);
	}
}

void
DeferredRenderer::renderGeometryObject(DeviceContext& deviceContext, const RenderObject& object) {
	if (!object.mesh || (!object.materialInstance && object.materialInstances.empty())) {
		return;
	}

	deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	std::vector<Submesh>& submeshes = object.mesh->getSubmeshes();
	for (Submesh& submesh : submeshes) {
		MaterialInstance* materialInstance = object.materialInstance;
		if (submesh.materialSlot < object.materialInstances.size() &&
			object.materialInstances[submesh.materialSlot]) {
			materialInstance = object.materialInstances[submesh.materialSlot];
		}

		if (!materialInstance) {
			continue;
		}

		Material* material = materialInstance->getMaterial();
		if (!material) {
			continue;
		}
		if (material->getDomain() == MaterialDomain::Transparent) {
			continue;
		}

		if (material->getRasterizerState()) {
			material->getRasterizerState()->render(deviceContext);
		}

		if (material->getDepthStencilState()) {
			material->getDepthStencilState()->render(deviceContext, 0, false);
		}

		m_gBufferShader.render(deviceContext);
		if (material->getSamplerState()) {
			material->getSamplerState()->render(deviceContext, 0, 1);
		}
		XMMATRIX submeshWorld = XMLoadFloat4x4(&submesh.localTransform) * object.world;
		XMStoreFloat4x4(&m_cbPerObject.World, XMMatrixTranspose(submeshWorld));
		m_perObjectBuffer.update(deviceContext, nullptr, 0, nullptr, &m_cbPerObject, 0, 0);
		m_perObjectBuffer.render(deviceContext, 1, 1, false);

		materialInstance->bindTextures(deviceContext);

		const MaterialParams& params = materialInstance->getParams();
		m_cbPerMaterial.BaseColor = params.baseColor;
		m_cbPerMaterial.Metallic = params.metallic;
		m_cbPerMaterial.Roughness = params.roughness;
		m_cbPerMaterial.AO = params.ao;
		m_cbPerMaterial.NormalScale = params.normalScale;
		m_cbPerMaterial.EmissiveStrength = params.emissiveStrength;
		m_cbPerMaterial.AlphaCutoff = material->getDomain() == MaterialDomain::Masked ? params.alphaCutoff : 0.0f;
		m_perMaterialBuffer.update(deviceContext, nullptr, 0, nullptr, &m_cbPerMaterial, 0, 0);
		m_perMaterialBuffer.render(deviceContext, 2, 1, true);

		submesh.vertexBuffer.render(deviceContext, 0, 1);
		submesh.indexBuffer.render(deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);
		deviceContext.DrawIndexed(submesh.indexCount, submesh.startIndex, 0);
	}
}

void
DeferredRenderer::renderLightingPass(DeviceContext& deviceContext) {
	clearDeferredSRVs(deviceContext);

	ID3D11ShaderResourceView* gBufferResources[4] = {
		m_gBufferAlbedoMetallicSRV.m_textureFromImg,
		m_gBufferNormalRoughnessSRV.m_textureFromImg,
		m_gBufferWorldAoSRV.m_textureFromImg,
		m_gBufferEmissiveAlphaSRV.m_textureFromImg
	};

	deviceContext.PSSetShaderResources(0, 4, gBufferResources);
	if (m_applyShadows && m_shadowDepthSRV.m_textureFromImg) {
		deviceContext.PSSetShaderResources(6, 1, &m_shadowDepthSRV.m_textureFromImg);
	}
	else {
		ID3D11ShaderResourceView* nullShadowSRV[1] = { nullptr };
		deviceContext.PSSetShaderResources(6, 1, nullShadowSRV);
	}
	m_disabledDepthStencil.render(deviceContext, 0, false);
	m_fullscreenRasterizer.render(deviceContext);
	m_lightingSampler.render(deviceContext, 0, 1);
	m_deferredLightingShader.render(deviceContext);
	m_perFrameBuffer.render(deviceContext, 0, 1, true);
	m_lightingDebugData.DebugViewMode = m_shadowFactorDebugEnabled ? 1 : m_deferredDebugViewMode;
	m_lightingDebugData.ShadowStrength = 1.0f;
	m_lightingDebugBuffer.update(deviceContext, nullptr, 0, nullptr, &m_lightingDebugData, 0, 0);
	m_lightingDebugBuffer.render(deviceContext, 1, 1, true);

	deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_fullscreenVertexBuffer.render(deviceContext, 0, 1);
	m_fullscreenIndexBuffer.render(deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);
	deviceContext.OMSetBlendState(m_opaqueBlendState, m_blendFactor, 0xffffffff);
	deviceContext.DrawIndexed(6, 0, 0);

	clearDeferredSRVs(deviceContext);
}

void
DeferredRenderer::renderSkyboxPass(DeviceContext& deviceContext, RenderScene& scene) {
	if (!scene.skybox) {
		return;
	}
	scene.skybox->render(deviceContext);
}

void
DeferredRenderer::renderTransparentPass(DeviceContext& deviceContext) {
	m_perFrameBuffer.render(deviceContext, 0, 1, true);
	if (m_applyShadows && m_shadowDepthSRV.m_textureFromImg) {
		deviceContext.PSSetShaderResources(6, 1, &m_shadowDepthSRV.m_textureFromImg);
	}
	else {
		ID3D11ShaderResourceView* nullShadowSRV[1] = { nullptr };
		deviceContext.PSSetShaderResources(6, 1, nullShadowSRV);
	}

	for (const RenderObject* object : m_transparentQueue) {
		if (!object) {
			continue;
		}

		Material* material = object->materialInstance ? object->materialInstance->getMaterial() : nullptr;
		deviceContext.OMSetBlendState(resolveBlendState(material), m_blendFactor, 0xffffffff);
		renderForwardObject(deviceContext, *object, RenderPassType::Transparent);
	}

	deviceContext.OMSetBlendState(m_opaqueBlendState, m_blendFactor, 0xffffffff);
}

void
DeferredRenderer::renderForwardObject(DeviceContext& deviceContext,
	const RenderObject& object,
	RenderPassType passType) {
	if (!object.mesh || (!object.materialInstance && object.materialInstances.empty())) {
		return;
	}

	deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	std::vector<Submesh>& submeshes = object.mesh->getSubmeshes();
	for (Submesh& submesh : submeshes) {
		MaterialInstance* materialInstance = object.materialInstance;
		if (submesh.materialSlot < object.materialInstances.size() &&
			object.materialInstances[submesh.materialSlot]) {
			materialInstance = object.materialInstances[submesh.materialSlot];
		}

		if (!materialInstance) {
			continue;
		}

		Material* material = materialInstance->getMaterial();
		if (!material) {
			continue;
		}
		if (passType == RenderPassType::Transparent &&
			material->getDomain() != MaterialDomain::Transparent) {
			continue;
		}
		if (passType != RenderPassType::Transparent &&
			material->getDomain() == MaterialDomain::Transparent) {
			continue;
		}

		if (material->getRasterizerState()) {
			material->getRasterizerState()->render(deviceContext);
		}

		if (passType == RenderPassType::Transparent) {
			deviceContext.OMSetBlendState(resolveBlendState(material), m_blendFactor, 0xffffffff);
			m_transparentDepthStencil.render(deviceContext, 0, false);
		}
		else if (material->getDepthStencilState()) {
			material->getDepthStencilState()->render(deviceContext, 0, false);
		}

		if (material->getShader()) {
			material->getShader()->render(deviceContext);
		}

		if (material->getSamplerState()) {
			material->getSamplerState()->render(deviceContext, 0, 1);
		}

		XMMATRIX submeshWorld = XMLoadFloat4x4(&submesh.localTransform) * object.world;
		XMStoreFloat4x4(&m_cbPerObject.World, XMMatrixTranspose(submeshWorld));
		m_perObjectBuffer.update(deviceContext, nullptr, 0, nullptr, &m_cbPerObject, 0, 0);
		m_perObjectBuffer.render(deviceContext, 1, 1, true);

		materialInstance->bindTextures(deviceContext);

		const MaterialParams& params = materialInstance->getParams();
		m_cbPerMaterial.BaseColor = params.baseColor;
		m_cbPerMaterial.Metallic = params.metallic;
		m_cbPerMaterial.Roughness = params.roughness;
		m_cbPerMaterial.AO = params.ao;
		m_cbPerMaterial.NormalScale = params.normalScale;
		m_cbPerMaterial.EmissiveStrength = params.emissiveStrength;
		m_cbPerMaterial.AlphaCutoff = material->getDomain() == MaterialDomain::Masked ? params.alphaCutoff : 0.0f;
		m_perMaterialBuffer.update(deviceContext, nullptr, 0, nullptr, &m_cbPerMaterial, 0, 0);
		m_perMaterialBuffer.render(deviceContext, 2, 1, true);

		submesh.vertexBuffer.render(deviceContext, 0, 1);
		submesh.indexBuffer.render(deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);
		deviceContext.DrawIndexed(submesh.indexCount, submesh.startIndex, 0);
	}
}

void
DeferredRenderer::renderShadowPass(DeviceContext& deviceContext) {
	if (!m_shadowDSV.m_depthStencilView || !m_shadowShader.m_VertexShader) {
		return;
	}

	ID3D11ShaderResourceView* nullShadowSRV[1] = { nullptr };
	deviceContext.PSSetShaderResources(6, 1, nullShadowSRV);
	deviceContext.OMSetRenderTargets(0, nullptr, m_shadowDSV.m_depthStencilView);
	deviceContext.ClearDepthStencilView(m_shadowDSV.m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	D3D11_VIEWPORT shadowViewport{};
	shadowViewport.TopLeftX = 0.0f;
	shadowViewport.TopLeftY = 0.0f;
	shadowViewport.Width = static_cast<float>(m_shadowMapSize);
	shadowViewport.Height = static_cast<float>(m_shadowMapSize);
	shadowViewport.MinDepth = 0.0f;
	shadowViewport.MaxDepth = 1.0f;
	deviceContext.RSSetViewports(1, &shadowViewport);

	m_shadowRasterizer.render(deviceContext);
	m_shadowDepthStencil.render(deviceContext, 0, false);
	m_perFrameBuffer.render(deviceContext, 0, 1, false);

	for (const RenderObject* object : m_opaqueQueue) {
		if (!object || !object->castShadow) {
			continue;
		}
		renderShadowObject(deviceContext, *object);
	}
}

void
DeferredRenderer::renderShadowObject(DeviceContext& deviceContext, const RenderObject& object) {
	if (!object.mesh) {
		return;
	}

	m_shadowShader.render(deviceContext);
	deviceContext.m_deviceContext->PSSetShader(nullptr, nullptr, 0);
	deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	std::vector<Submesh>& submeshes = object.mesh->getSubmeshes();
	for (Submesh& submesh : submeshes) {
		MaterialInstance* materialInstance = object.materialInstance;
		if (submesh.materialSlot < object.materialInstances.size() &&
			object.materialInstances[submesh.materialSlot]) {
			materialInstance = object.materialInstances[submesh.materialSlot];
		}

		Material* material = materialInstance ? materialInstance->getMaterial() : nullptr;
		if (material && material->getDomain() == MaterialDomain::Transparent) {
			continue;
		}

		XMMATRIX submeshWorld = XMLoadFloat4x4(&submesh.localTransform) * object.world;
		XMStoreFloat4x4(&m_cbPerObject.World, XMMatrixTranspose(submeshWorld));
		m_perObjectBuffer.update(deviceContext, nullptr, 0, nullptr, &m_cbPerObject, 0, 0);
		m_perObjectBuffer.render(deviceContext, 1, 1, false);

		submesh.vertexBuffer.render(deviceContext, 0, 1);
		submesh.indexBuffer.render(deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);
		deviceContext.DrawIndexed(submesh.indexCount, submesh.startIndex, 0);
	}
}

HRESULT
DeferredRenderer::createShadowResources(Device& device) {
	HRESULT hr = m_shadowDepthTexture.init(
		device,
		m_shadowMapSize,
		m_shadowMapSize,
		DXGI_FORMAT_R24G8_TYPELESS,
		D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
	if (FAILED(hr)) {
		return hr;
	}

	hr = m_shadowDepthSRV.init(device, m_shadowDepthTexture, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
	if (FAILED(hr)) {
		return hr;
	}

	hr = m_shadowDSV.init(device, m_shadowDepthTexture, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_DSV_DIMENSION_TEXTURE2D);
	if (FAILED(hr)) {
		return hr;
	}

	LayoutBuilder builder;
	builder.Add("POSITION", DXGI_FORMAT_R32G32B32_FLOAT)
		.Add("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT)
		.Add("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
		.Add("BITANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
		.Add("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

	hr = m_shadowShader.init(device, "ShadowMap.hlsl", builder);
	if (FAILED(hr)) {
		return hr;
	}

	return m_shadowRasterizer.init(device, D3D11_FILL_SOLID, D3D11_CULL_BACK, false, true);
}

HRESULT
DeferredRenderer::createGBufferResources(Device& device, unsigned int width, unsigned int height) {
	m_gBufferEmissiveAlphaRTV.destroy();
	m_gBufferEmissiveAlphaSRV.destroy();
	m_gBufferEmissiveAlphaTexture.destroy();
	m_gBufferWorldAoRTV.destroy();
	m_gBufferWorldAoSRV.destroy();
	m_gBufferWorldAoTexture.destroy();
	m_gBufferNormalRoughnessRTV.destroy();
	m_gBufferNormalRoughnessSRV.destroy();
	m_gBufferNormalRoughnessTexture.destroy();
	m_gBufferAlbedoMetallicRTV.destroy();
	m_gBufferAlbedoMetallicSRV.destroy();
	m_gBufferAlbedoMetallicTexture.destroy();

	HRESULT hr = createGBufferTarget(device,
		width,
		height,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		m_gBufferAlbedoMetallicTexture,
		m_gBufferAlbedoMetallicSRV,
		m_gBufferAlbedoMetallicRTV);
	if (FAILED(hr)) {
		return hr;
	}

	hr = createGBufferTarget(device,
		width,
		height,
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		m_gBufferNormalRoughnessTexture,
		m_gBufferNormalRoughnessSRV,
		m_gBufferNormalRoughnessRTV);
	if (FAILED(hr)) {
		return hr;
	}

	hr = createGBufferTarget(device,
		width,
		height,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		m_gBufferWorldAoTexture,
		m_gBufferWorldAoSRV,
		m_gBufferWorldAoRTV);
	if (FAILED(hr)) {
		return hr;
	}

	return createGBufferTarget(device,
		width,
		height,
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		m_gBufferEmissiveAlphaTexture,
		m_gBufferEmissiveAlphaSRV,
		m_gBufferEmissiveAlphaRTV);
}

HRESULT
DeferredRenderer::createGBufferTarget(Device& device,
	unsigned int width,
	unsigned int height,
	DXGI_FORMAT format,
	Texture& texture,
	Texture& srv,
	RenderTargetView& rtv) {
	HRESULT hr = texture.init(device,
		width,
		height,
		format,
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
		1,
		0);
	if (FAILED(hr)) {
		return hr;
	}

	hr = rtv.init(device, texture, D3D11_RTV_DIMENSION_TEXTURE2D, format);
	if (FAILED(hr)) {
		return hr;
	}

	return srv.init(device, texture, format);
}

HRESULT
DeferredRenderer::createLightingResources(Device& device) {
	LayoutBuilder geometryBuilder;
	geometryBuilder.Add("POSITION", DXGI_FORMAT_R32G32B32_FLOAT)
		.Add("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT)
		.Add("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
		.Add("BITANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
		.Add("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

	HRESULT hr = m_gBufferShader.init(device, "DeferredGBuffer.hlsl", geometryBuilder);
	if (FAILED(hr)) {
		return hr;
	}

	LayoutBuilder fullscreenBuilder;
	fullscreenBuilder.Add("POSITION", DXGI_FORMAT_R32G32B32_FLOAT)
		.Add("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT)
		.Add("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
		.Add("BITANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
		.Add("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

	hr = m_deferredLightingShader.init(device, "DeferredLighting.hlsl", fullscreenBuilder);
	if (FAILED(hr)) {
		return hr;
	}

	hr = m_lightingSampler.init(device);
	if (FAILED(hr)) {
		return hr;
	}

	return m_fullscreenRasterizer.init(device, D3D11_FILL_SOLID, D3D11_CULL_NONE, false, false);
}

HRESULT
DeferredRenderer::createFullScreenQuad(Device& device) {
	MeshComponent mesh;
	mesh.m_vertex = {
		{ EU::Vector3(-1.0f, -1.0f, 0.0f), EU::Vector3(), EU::Vector3(), EU::Vector3(), EU::Vector2(0.0f, 1.0f) },
		{ EU::Vector3(-1.0f,  1.0f, 0.0f), EU::Vector3(), EU::Vector3(), EU::Vector3(), EU::Vector2(0.0f, 0.0f) },
		{ EU::Vector3(1.0f,  1.0f, 0.0f), EU::Vector3(), EU::Vector3(), EU::Vector3(), EU::Vector2(1.0f, 0.0f) },
		{ EU::Vector3(1.0f, -1.0f, 0.0f), EU::Vector3(), EU::Vector3(), EU::Vector3(), EU::Vector2(1.0f, 1.0f) }
	};
	mesh.m_index = { 0, 1, 2, 0, 2, 3 };
	mesh.m_numVertex = static_cast<int>(mesh.m_vertex.size());
	mesh.m_numIndex = static_cast<int>(mesh.m_index.size());

	HRESULT hr = m_fullscreenVertexBuffer.init(device, mesh, D3D11_BIND_VERTEX_BUFFER);
	if (FAILED(hr)) {
		return hr;
	}

	return m_fullscreenIndexBuffer.init(device, mesh, D3D11_BIND_INDEX_BUFFER);
}

HRESULT
DeferredRenderer::createBlendStates(Device& device) {
	if (!device.m_device) {
		return E_POINTER;
	}

	D3D11_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;

	D3D11_RENDER_TARGET_BLEND_DESC& renderTarget = blendDesc.RenderTarget[0];
	renderTarget.BlendEnable = FALSE;
	renderTarget.SrcBlend = D3D11_BLEND_ONE;
	renderTarget.DestBlend = D3D11_BLEND_ZERO;
	renderTarget.BlendOp = D3D11_BLEND_OP_ADD;
	renderTarget.SrcBlendAlpha = D3D11_BLEND_ONE;
	renderTarget.DestBlendAlpha = D3D11_BLEND_ZERO;
	renderTarget.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	renderTarget.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HRESULT hr = device.m_device->CreateBlendState(&blendDesc, &m_opaqueBlendState);
	if (FAILED(hr)) {
		return hr;
	}

	renderTarget.BlendEnable = TRUE;
	renderTarget.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	renderTarget.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	renderTarget.BlendOp = D3D11_BLEND_OP_ADD;
	renderTarget.SrcBlendAlpha = D3D11_BLEND_ONE;
	renderTarget.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	renderTarget.BlendOpAlpha = D3D11_BLEND_OP_ADD;

	hr = device.m_device->CreateBlendState(&blendDesc, &m_alphaBlendState);
	if (FAILED(hr)) {
		return hr;
	}

	renderTarget.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	renderTarget.DestBlend = D3D11_BLEND_ONE;
	renderTarget.SrcBlendAlpha = D3D11_BLEND_ONE;
	renderTarget.DestBlendAlpha = D3D11_BLEND_ONE;

	hr = device.m_device->CreateBlendState(&blendDesc, &m_additiveBlendState);
	if (FAILED(hr)) {
		return hr;
	}

	renderTarget.SrcBlend = D3D11_BLEND_ONE;
	renderTarget.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	renderTarget.SrcBlendAlpha = D3D11_BLEND_ONE;
	renderTarget.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;

	return device.m_device->CreateBlendState(&blendDesc, &m_premultipliedBlendState);
}

ID3D11BlendState*
DeferredRenderer::resolveBlendState(const Material* material) const {
	if (!material) {
		return m_opaqueBlendState;
	}

	if (material->getDomain() != MaterialDomain::Transparent) {
		return m_opaqueBlendState;
	}

	switch (material->getBlendMode()) {
	case BlendMode::Additive:
		return m_additiveBlendState ? m_additiveBlendState : m_alphaBlendState;
	case BlendMode::PremultipliedAlpha:
		return m_premultipliedBlendState ? m_premultipliedBlendState : m_alphaBlendState;
	case BlendMode::Alpha:
	case BlendMode::Opaque:
	default:
		return m_alphaBlendState ? m_alphaBlendState : m_opaqueBlendState;
	}
}