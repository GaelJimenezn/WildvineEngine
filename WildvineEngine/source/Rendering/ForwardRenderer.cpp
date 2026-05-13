#include "Rendering/ForwardRenderer.h"
#include <algorithm>
#include "Device.h"
#include "DeviceContext.h"
#include "Rendering/Material.h"
#include "Rendering/MaterialInstance.h"
#include "Rendering/Mesh.h"
#include "SamplerState.h"
#include "EngineUtilities/Utilities/Camera.h"
#include "EngineUtilities/Utilities/EditorViewportPass.h"
#include "EngineUtilities/Utilities/Skybox.h"

HRESULT
ForwardRenderer::init(Device& device) {
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

	hr = m_transparentDepthStencil.init(device,
		true,
		D3D11_DEPTH_WRITE_MASK_ZERO,
		D3D11_COMPARISON_LESS_EQUAL);
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
ForwardRenderer::resize(Device& device, unsigned int width, unsigned int height) {
	(void)device;
	(void)width;
	(void)height;
}

void
ForwardRenderer::updatePerFrame(const Camera& camera,
	const RenderScene& scene,
	DeviceContext& deviceContext) {
	XMStoreFloat4x4(&m_cbPerFrame.View, XMMatrixTranspose(camera.getView()));
	XMStoreFloat4x4(&m_cbPerFrame.Projection, XMMatrixTranspose(camera.getProj()));
	m_cbPerFrame.CameraPos = camera.getPosition();
	m_cbPerFrame.LightDir = EU::Vector3(0.0f, -1.0f, 0.0f);
	m_cbPerFrame.LightColor = EU::Vector3(1.0f, 1.0f, 1.0f);

	if (!scene.directionalLights.empty()) {
		const LightData& mainLight = scene.directionalLights.front();
		m_cbPerFrame.LightDir = mainLight.direction;
		m_cbPerFrame.LightColor = mainLight.color * mainLight.intensity;
	}

	m_perFrameBuffer.update(deviceContext, nullptr, 0, nullptr, &m_cbPerFrame, 0, 0);
}

void
ForwardRenderer::render(DeviceContext& deviceContext,
	const Camera& camera,
	RenderScene& scene,
	EditorViewportPass& viewportPass) {
	const float viewportClear[4] = { 0.10f, 0.10f, 0.10f, 1.0f };
	viewportPass.begin(deviceContext, viewportClear);
	viewportPass.setViewport(deviceContext);
	viewportPass.clearDepth(deviceContext);

	buildQueues(scene, camera);
	updatePerFrame(camera, scene, deviceContext);

	renderSkyboxPass(deviceContext, scene);
	renderOpaquePass(deviceContext);
	renderTransparentPass(deviceContext);
}

void
ForwardRenderer::destroy() {
	m_opaqueQueue.clear();
	m_transparentQueue.clear();
	SAFE_RELEASE(m_alphaBlendState);
	SAFE_RELEASE(m_opaqueBlendState);
	SAFE_RELEASE(m_additiveBlendState);
	SAFE_RELEASE(m_premultipliedBlendState);
	m_transparentDepthStencil.destroy();
	m_perMaterialBuffer.destroy();
	m_perObjectBuffer.destroy();
	m_perFrameBuffer.destroy();
}

void
ForwardRenderer::buildQueues(RenderScene& scene, const Camera& camera) {
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
ForwardRenderer::renderOpaquePass(DeviceContext& deviceContext) {
	m_perFrameBuffer.render(deviceContext, 0, 1, true);
	deviceContext.OMSetBlendState(m_opaqueBlendState, m_blendFactor, 0xffffffff);

	for (const RenderObject* object : m_opaqueQueue) {
		if (!object) {
			continue;
		}
		renderObject(deviceContext, *object, RenderPassType::Opaque);
	}
}

void
ForwardRenderer::renderTransparentPass(DeviceContext& deviceContext) {
	m_perFrameBuffer.render(deviceContext, 0, 1, true);

	for (const RenderObject* object : m_transparentQueue) {
		if (!object) {
			continue;
		}
		Material* material = object->materialInstance ? object->materialInstance->getMaterial() : nullptr;
		deviceContext.OMSetBlendState(resolveBlendState(material), m_blendFactor, 0xffffffff);
		renderObject(deviceContext, *object, RenderPassType::Transparent);
	}

	deviceContext.OMSetBlendState(m_opaqueBlendState, m_blendFactor, 0xffffffff);
}

void
ForwardRenderer::renderSkyboxPass(DeviceContext& deviceContext, RenderScene& scene) {
	if (!scene.skybox) {
		return;
	}
	scene.skybox->render(deviceContext);
}

void
ForwardRenderer::renderObject(DeviceContext& deviceContext,
	const RenderObject& object,
	RenderPassType passType) {
	if (!object.mesh || (!object.materialInstance && object.materialInstances.empty())) {
		return;
	}

	XMStoreFloat4x4(&m_cbPerObject.World, XMMatrixTranspose(object.world));
	m_perObjectBuffer.update(deviceContext, nullptr, 0, nullptr, &m_cbPerObject, 0, 0);
	m_perObjectBuffer.render(deviceContext, 1, 1, true);

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

		if (material->getRasterizerState()) {
			material->getRasterizerState()->render(deviceContext);
		}

		if (passType == RenderPassType::Transparent) {
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

		materialInstance->bindTextures(deviceContext);

		const MaterialParams& params = materialInstance->getParams();
		m_cbPerMaterial.BaseColor = params.baseColor;
		m_cbPerMaterial.Metallic = params.metallic;
		m_cbPerMaterial.Roughness = params.roughness;
		m_cbPerMaterial.AO = params.ao;
		m_cbPerMaterial.NormalScale = params.normalScale;
		m_cbPerMaterial.EmissiveStrength = params.emissiveStrength;
		m_cbPerMaterial.AlphaCutoff = 0.0f;
		if (material->getDomain() == MaterialDomain::Masked) {
			m_cbPerMaterial.AlphaCutoff = params.alphaCutoff;
		}
		m_perMaterialBuffer.update(deviceContext, nullptr, 0, nullptr, &m_cbPerMaterial, 0, 0);
		m_perMaterialBuffer.render(deviceContext, 2, 1, true);

		submesh.vertexBuffer.render(deviceContext, 0, 1);
		submesh.indexBuffer.render(deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);
		deviceContext.DrawIndexed(submesh.indexCount, submesh.startIndex, 0);
	}
}

HRESULT
ForwardRenderer::createBlendStates(Device& device) {
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

	renderTarget.BlendEnable = TRUE;
	renderTarget.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	renderTarget.DestBlend = D3D11_BLEND_ONE;
	renderTarget.BlendOp = D3D11_BLEND_OP_ADD;
	renderTarget.SrcBlendAlpha = D3D11_BLEND_ONE;
	renderTarget.DestBlendAlpha = D3D11_BLEND_ONE;
	renderTarget.BlendOpAlpha = D3D11_BLEND_OP_ADD;

	hr = device.m_device->CreateBlendState(&blendDesc, &m_additiveBlendState);
	if (FAILED(hr)) {
		return hr;
	}

	renderTarget.BlendEnable = TRUE;
	renderTarget.SrcBlend = D3D11_BLEND_ONE;
	renderTarget.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	renderTarget.BlendOp = D3D11_BLEND_OP_ADD;
	renderTarget.SrcBlendAlpha = D3D11_BLEND_ONE;
	renderTarget.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	renderTarget.BlendOpAlpha = D3D11_BLEND_OP_ADD;

	return device.m_device->CreateBlendState(&blendDesc, &m_premultipliedBlendState);
}

ID3D11BlendState*
ForwardRenderer::resolveBlendState(const Material* material) const {
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



