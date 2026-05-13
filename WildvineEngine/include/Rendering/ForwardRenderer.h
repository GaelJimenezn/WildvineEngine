#pragma once
#include "Prerequisites.h"
#include "Buffer.h"
#include "DepthStencilState.h"
#include "Rendering/RenderScene.h"
#include "Rendering/RenderTypes.h"

class Device;
class DeviceContext;
class Camera;
class EditorViewportPass;
class Material;

class
ForwardRenderer {
public:
	HRESULT init(Device& device);
	void resize(Device& device, unsigned int width, unsigned int height);
	void updatePerFrame(const Camera& camera, const RenderScene& scene, DeviceContext& deviceContext);
	void render(DeviceContext& deviceContext,
		const Camera& camera,
		RenderScene& scene,
		EditorViewportPass& viewportPass);
	void destroy();

private:
	void buildQueues(RenderScene& scene, const Camera& camera);
	void renderOpaquePass(DeviceContext& deviceContext);
	void renderTransparentPass(DeviceContext& deviceContext);
	void renderSkyboxPass(DeviceContext& deviceContext, RenderScene& scene);
	void renderObject(DeviceContext& deviceContext, const RenderObject& object, RenderPassType passType);
	HRESULT createBlendStates(Device& device);
	ID3D11BlendState* resolveBlendState(const Material* material) const;

private:
	Buffer m_perFrameBuffer;
	Buffer m_perObjectBuffer;
	Buffer m_perMaterialBuffer;
	DepthStencilState m_transparentDepthStencil;
	ID3D11BlendState* m_alphaBlendState = nullptr;
	ID3D11BlendState* m_opaqueBlendState = nullptr;
	ID3D11BlendState* m_additiveBlendState = nullptr;
	ID3D11BlendState* m_premultipliedBlendState = nullptr;
	float m_blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	CBPerFrame m_cbPerFrame{};
	CBPerObject m_cbPerObject{};
	CBPerMaterial m_cbPerMaterial{};

	std::vector<const RenderObject*> m_opaqueQueue;
	std::vector<const RenderObject*> m_transparentQueue;
};
