#include "BaseApp.h"
#include "ResourceManager.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace {

static std::string toLowerCopy(std::string s) {
	for (char& c : s) if (c >= 'A' && c <= 'Z') c = (char)(c + 32);
	return s;
}

static std::string stripExt(const std::string& f) {
	size_t d = f.find_last_of('.');
	return (d == std::string::npos) ? f : f.substr(0, d);
}

static std::string fileBaseName(const std::string& path) {
	size_t s = path.find_last_of("/\\");
	std::string n = (s == std::string::npos) ? path : path.substr(s + 1);
	return stripExt(n);
}

static bool endsWith(const std::string& s, const std::string& suf) {
	return s.size() >= suf.size() && s.compare(s.size() - suf.size(), suf.size(), suf) == 0;
}

static bool containsStr(const std::string& s, const std::string& sub) {
	return s.find(sub) != std::string::npos;
}

static ExtensionType extFromName(const std::string& lower) {
	if (endsWith(lower, ".jpg") || endsWith(lower, ".jpeg")) return JPG;
	if (endsWith(lower, ".dds")) return DDS;
	return PNG;
}

static std::vector<std::string> listImageFiles(const std::string& dir) {
	std::vector<std::string> out;
	std::string pat = dir + "\\*";
	WIN32_FIND_DATAA fd;
	HANDLE h = FindFirstFileA(pat.c_str(), &fd);
	if (h == INVALID_HANDLE_VALUE) return out;
	do {
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
		std::string lo = toLowerCopy(fd.cFileName);
		if (endsWith(lo, ".png") || endsWith(lo, ".jpg") || endsWith(lo, ".jpeg") ||
			endsWith(lo, ".dds") || endsWith(lo, ".tga"))
			out.push_back(fd.cFileName);
	} while (FindNextFileA(h, &fd));
	FindClose(h);
	return out;
}

static std::vector<std::string> listSubfolders(const std::string& dir) {
	std::vector<std::string> out;
	std::string pat = dir + "\\*";
	WIN32_FIND_DATAA fd;
	HANDLE h = FindFirstFileA(pat.c_str(), &fd);
	if (h == INVALID_HANDLE_VALUE) return out;
	do {
		if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
		std::string n = fd.cFileName;
		if (n == "." || n == "..") continue;
		out.push_back(n);
	} while (FindNextFileA(h, &fd));
	FindClose(h);
	return out;
}

class TransformCommand : public ICommand {
public:
	TransformCommand(EU::TSharedPointer<Actor> actor,
		const GizmoEditState& before,
		const GizmoEditState& after)
		: m_actor(actor), m_before(before), m_after(after) {}
	void undo() override { apply(m_before); }
	void redo() override { apply(m_after); }
	const char* name() const override { return "Transform"; }
private:
	void apply(const GizmoEditState& s) {
		if (m_actor.isNull()) return;
		EU::TSharedPointer<Transform> t = m_actor->getComponent<Transform>();
		if (!t) return;
		t->setPosition(s.position);
		t->setRotation(s.rotation);
		t->setScale(s.scale);
	}
	EU::TSharedPointer<Actor> m_actor;
	GizmoEditState m_before;
	GizmoEditState m_after;
};

class SpawnActorCommand : public ICommand {
public:
	SpawnActorCommand(BaseApp* app, EU::TSharedPointer<Actor> actor)
		: m_app(app), m_actor(actor) {}
	void undo() override { if (m_app) m_app->removeActorFromScene(m_actor); }
	void redo() override { if (m_app) m_app->addActorToScene(m_actor); }
	const char* name() const override { return "Spawn Actor"; }
private:
	BaseApp* m_app;
	EU::TSharedPointer<Actor> m_actor;
};

class DeleteActorCommand : public ICommand {
public:
	DeleteActorCommand(BaseApp* app, EU::TSharedPointer<Actor> actor)
		: m_app(app), m_actor(actor) {}
	void undo() override { if (m_app) m_app->addActorToScene(m_actor); }
	void redo() override { if (m_app) m_app->removeActorFromScene(m_actor); }
	const char* name() const override { return "Delete Actor"; }
private:
	BaseApp* m_app;
	EU::TSharedPointer<Actor> m_actor;
};

} // namespace

HRESULT
BaseApp::awake() {
	HRESULT hr = S_OK;
	m_sceneGraph.init();
	MESSAGE("Main", "Awake", "Application awake successfully.");
	return hr;
}

EU::TSharedPointer<Actor>
BaseApp::createLightActor(LightType type, const std::string& baseName) {
	// Solo reusar la directional si ya existe una Y se pide crear otra directional
	if (type == LightType::Directional && !m_directionalLightActor.isNull()) {
		return m_directionalLightActor;
	}

	EU::TSharedPointer<Actor> lightActor = EU::MakeShared<Actor>(m_device);
	if (lightActor.isNull()) {
		return lightActor;
	}

	lightActor->setName(baseName);

	EU::TSharedPointer<LightComponent> lightComponent = lightActor->getComponent<LightComponent>();
	if (!lightComponent) {
		lightComponent = EU::MakeShared<LightComponent>();
		lightActor->addComponent(lightComponent);
	}

	LightData& light = lightComponent->getLightData();
	light.type = type;
	light.color = EU::Vector3(1.0f, 1.0f, 1.0f);
	light.intensity = 3.0f;
	light.direction = EU::Vector3(-0.35f, -1.0f, 0.25f);
	light.position = EU::Vector3(0.0f, 3.0f, 0.0f);

	// Defaults per type
	if (type == LightType::Point) {
		light.range = 10.0f;
		light.intensity = 5.0f;
		light.spotAngle = 0.0f;
	} else if (type == LightType::Spot) {
		light.range = 15.0f;
		light.intensity = 5.0f;
		light.spotAngle = 30.0f; // grados
	} else if (type == LightType::Rect) {
		light.range = 10.0f;
		light.intensity = 5.0f;
		light.spotAngle = 0.0f;
	} else {
		// Directional
		light.range = 0.0f;
		light.spotAngle = 0.0f;
		light.position = EU::Vector3(-4.0f, 6.0f, -4.0f);
	}

	lightComponent->setCastShadow(type == LightType::Directional);

	m_actors.push_back(lightActor);
	m_sceneGraph.addEntity(lightActor.get());

	EU::TSharedPointer<Transform> transform = lightActor->getComponent<Transform>();
	if (transform) {
		transform->setTransform(
			light.position,
			EU::Vector3(0.0f, 0.0f, 0.0f),
			EU::Vector3(1.0f, 1.0f, 1.0f));
		transform->rebuildMatrixFromVectors();
	}

	if (type == LightType::Directional && m_directionalLightActor.isNull()) {
		m_directionalLightActor = lightActor;
	}
	selectActor(lightActor);
	return lightActor;
}

void
BaseApp::enforceDefaultSceneLayout() {
	if (!m_cyberGun.isNull()) {
		m_cyberGun->setName("SM_CyberGun");
	}

	if (!m_directionalLightActor.isNull()) {
		m_directionalLightActor->setName("DirectionalLight");
		EU::TSharedPointer<LightComponent> lightComponent =
			m_directionalLightActor->getComponent<LightComponent>();

		if (!lightComponent) {
			lightComponent = EU::MakeShared<LightComponent>();
			m_directionalLightActor->addComponent(lightComponent);
		}

		LightData& light = lightComponent->getLightData();
		light.type = LightType::Directional;
		if (light.intensity <= 0.0f) light.intensity = 3.0f;
		if (light.direction.x == 0.0f && light.direction.y == 0.0f && light.direction.z == 0.0f) {
			light.direction = EU::Vector3(-0.35f, -1.0f, 0.25f);
		}
		light.range = 0.0f;
		light.spotAngle = 0.0f;
		lightComponent->setCastShadow(true);
	}

	Actor* modelActor = m_cyberGun.isNull() ? nullptr : m_cyberGun.get();
	Actor* lightActor = m_directionalLightActor.isNull() ? nullptr : m_directionalLightActor.get();

	for (int i = static_cast<int>(m_actors.size()) - 1; i >= 0; --i) {
		Actor* actor = m_actors[i].isNull() ? nullptr : m_actors[i].get();
		if (actor != modelActor && actor != lightActor) {
			if (actor) m_sceneGraph.removeEntity(actor);
			m_actors.erase(m_actors.begin() + i);
		}
	}

	if (m_gui.selectedActorIndex >= static_cast<int>(m_actors.size())) {
		m_gui.selectedActorIndex = m_actors.empty() ? -1 : 0;
	}
}

void
BaseApp::frameDefaultSceneCamera() {
	if (m_model == nullptr || m_cyberGun.isNull()) {
		m_camera.lookAt(EU::Vector3(0.0f, 3.0f, -12.0f), EU::Vector3(0.0f, 0.0f, 0.0f));
		return;
	}

	EU::Vector3 minBounds(
		(std::numeric_limits<float>::max)(),
		(std::numeric_limits<float>::max)(),
		(std::numeric_limits<float>::max)());
	EU::Vector3 maxBounds(
		-(std::numeric_limits<float>::max)(),
		-(std::numeric_limits<float>::max)(),
		-(std::numeric_limits<float>::max)());

	bool hasVertices = false;
	for (const MeshComponent& meshComponent : m_model->GetMeshes()) {
		for (const SimpleVertex& vertex : meshComponent.m_vertex) {
			const EU::Vector3& p = vertex.Position;
			if (p.x < minBounds.x) minBounds.x = p.x;
			if (p.y < minBounds.y) minBounds.y = p.y;
			if (p.z < minBounds.z) minBounds.z = p.z;
			if (p.x > maxBounds.x) maxBounds.x = p.x;
			if (p.y > maxBounds.y) maxBounds.y = p.y;
			if (p.z > maxBounds.z) maxBounds.z = p.z;
			hasVertices = true;
		}
	}

	if (!hasVertices) {
		m_camera.lookAt(EU::Vector3(0.0f, 3.0f, -12.0f), EU::Vector3(0.0f, 0.0f, 0.0f));
		return;
	}

	EU::Vector3 center(
		(minBounds.x + maxBounds.x) * 0.5f,
		(minBounds.y + maxBounds.y) * 0.5f,
		(minBounds.z + maxBounds.z) * 0.5f);

	EU::Vector3 extents(
		(maxBounds.x - minBounds.x) * 0.5f,
		(maxBounds.y - minBounds.y) * 0.5f,
		(maxBounds.z - minBounds.z) * 0.5f);

	EU::TSharedPointer<Transform> transform = m_cyberGun->getComponent<Transform>();
	if (transform) {
		const EU::Vector3& position = transform->getPosition();
		const EU::Vector3& scale = transform->getScale();
		center = EU::Vector3(
			position.x + center.x * scale.x,
			position.y + center.y * scale.y,
			position.z + center.z * scale.z);
		extents = EU::Vector3(
			std::abs(extents.x * scale.x),
			std::abs(extents.y * scale.y),
			std::abs(extents.z * scale.z));
	}

	const float computedRadius = std::sqrt(
		extents.x * extents.x + extents.y * extents.y + extents.z * extents.z);
	const float radius = computedRadius > 1.0f ? computedRadius : 1.0f;
	const float fovY = m_camera.getFovY() > 0.35f ? m_camera.getFovY() : 0.35f;
	const float computedDistance = (radius / std::tan(fovY * 0.5f)) * 1.35f;
	const float distance = computedDistance > 8.0f ? computedDistance : 8.0f;
	const float computedHeight = radius * 0.18f;
	const float height = computedHeight > 1.5f ? computedHeight : 1.5f;

	EU::Vector3 cameraPosition(center.x, center.y + height, center.z - distance);
	m_camera.lookAt(cameraPosition, center);
}

void
BaseApp::selectActor(EU::TSharedPointer<Actor> actor) {
	if (actor.isNull()) {
		m_gui.selectedActorIndex = -1;
		return;
	}
	Actor* target = actor.get();
	for (int i = 0; i < static_cast<int>(m_actors.size()); ++i) {
		if (!m_actors[i].isNull() && m_actors[i].get() == target) {
			m_gui.selectedActorIndex = i;
			return;
		}
	}
}

void
BaseApp::syncLightActors() {
	EU::TSharedPointer<Actor> firstDirectional;

	for (auto& actor : m_actors) {
		if (actor.isNull()) continue;
		EU::TSharedPointer<LightComponent> lightComponent = actor->getComponent<LightComponent>();
		if (!lightComponent) continue;

		LightData& light = lightComponent->getLightData();
		EU::TSharedPointer<Transform> transform = actor->getComponent<Transform>();
		if (transform) light.position = transform->getPosition();

		if (light.type == LightType::Directional && firstDirectional.isNull()) {
			firstDirectional = actor;
			m_constantBufferStruct.LightDir = light.direction;
			m_constantBufferStruct.LightColor = light.color * light.intensity;
		}
	}

	m_directionalLightActor = firstDirectional;
}

HRESULT
BaseApp::loadPbrTexture(Texture& texture, const std::string& path, const char* debugName) {
	HRESULT hr = texture.init(m_device, path, PNG);
	if (FAILED(hr)) {
		ERROR("Main", "LoadPbrTexture",
			("Failed to initialize " + std::string(debugName) + " texture: " + path +
				". HRESULT: " + std::to_string(hr)).c_str());
	}
	return hr;
}

int
BaseApp::run(HINSTANCE hInst, int nCmdShow) {
	if (FAILED(m_window.init(hInst, nCmdShow, WndProc, this))) {
		ERROR("Main", "Run", "Failed to initialize window.");
		return 0;
	}
	if (FAILED(awake())) {
		ERROR("Main", "Run", "Failed to awake application.");
		return 0;
	}
	if (FAILED(init())) {
		ERROR("Main", "Run", "Failed to initialize device and device context.");
		return 0;
	}

	m_gui.init(m_window, m_device, m_deviceContext);
	m_guiInitialized = true;

	MSG msg = {};
	LARGE_INTEGER freq, prev;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&prev);

	while (WM_QUIT != msg.message) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			LARGE_INTEGER curr;
			QueryPerformanceCounter(&curr);
			float deltaTime = static_cast<float>(curr.QuadPart - prev.QuadPart) / freq.QuadPart;
			prev = curr;
			update(deltaTime);
			render();
		}
	}
	return (int)msg.wParam;
}

HRESULT
BaseApp::init() {
	HRESULT hr = S_OK;

	// 1. SwapChain
	hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window);
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice", ("Failed to initialize SwapChain. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	// 2. Render Target View
	hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
	if (FAILED(hr)) { ERROR("Main", "InitDevice", "Failed RTV."); return hr; }

	// 3. Depth Stencil Texture
	hr = m_depthStencil.init(m_device, m_window.m_width, m_window.m_height,
		DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, 4, 0);
	if (FAILED(hr)) { ERROR("Main", "InitDevice", "Failed DepthStencil."); return hr; }

	// 4. Depth Stencil View
	hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
	if (FAILED(hr)) { ERROR("Main", "InitDevice", "Failed DSV."); return hr; }

	// 5. Viewport
	hr = m_viewport.init(m_window);
	if (FAILED(hr)) { ERROR("Main", "InitDevice", "Failed Viewport."); return hr; }
	m_d3dReady = true;

	// --------------------------------------------------------------------------
	// SKYBOX
	// --------------------------------------------------------------------------
	std::array<std::string, 6> faces = {
		"Skybox/cubemap_0.png", "Skybox/cubemap_1.png", "Skybox/cubemap_2.png",
		"Skybox/cubemap_3.png", "Skybox/cubemap_4.png", "Skybox/cubemap_5.png"
	};
	m_skyboxTex.CreateCubemap(m_device, m_deviceContext, faces, false);

	// --------------------------------------------------------------------------
	// ACTOR PRINCIPAL
	// --------------------------------------------------------------------------
	m_cyberGun = EU::MakeShared<Actor>(m_device);
	if (!m_cyberGun.isNull()) {
		m_model = new Model3D("Assets/Models/CyberGun.fbx", ModelType::FBX);
		if (!m_model || !m_model->load("Assets/Models/CyberGun.fbx")) {
			ERROR("Main", "InitDevice", "Failed to load CyberGun model.");
			return E_FAIL;
		}

		struct PbrTextureRequest {
			Texture* texture;
			const char* path;
			const char* debugName;
		};

		const PbrTextureRequest textureRequests[] = {
			{ &m_AlbedoSRV, "Assets/Textures/CyberGun/base.tga", "Albedo" },
			{ &m_MetallicSRV, "Assets/Textures/CyberGun/metallic.tga", "Metallic" },
			{ &m_RoughnessSRV, "Assets/Textures/CyberGun/roughness.tga", "Roughness" },
			{ &m_AOSRV, "Assets/Textures/CyberGun/ao.tga", "Ambient Occlusion" },
			{ &m_NormalSRV, "Assets/Textures/CyberGun/normal.tga", "Normal" }
		};

		for (const PbrTextureRequest& request : textureRequests) {
			hr = loadPbrTexture(*request.texture, request.path, request.debugName);
			if (FAILED(hr)) return hr;
		}

		std::vector<MeshComponent> cyberGunMeshes = m_model->GetMeshes();

		m_cyberGun->setName("SM_CyberGun");
		m_actors.push_back(m_cyberGun);

		EU::TSharedPointer<Transform> transform = m_cyberGun->getComponent<Transform>();
		if (transform) {
			transform->setTransform(
				EU::Vector3(0.0f, 0.0f, 0.0f),
				EU::Vector3(0.0f, 0.0f, 0.0f),
				EU::Vector3(1.0f, 1.0f, 1.0f));
			transform->rebuildMatrixFromVectors();
		}

		// Mesh de render
		m_cyberGunRenderMesh.destroy();
		for (const MeshComponent& meshComponent : cyberGunMeshes) {
			Submesh submesh{};
			hr = submesh.vertexBuffer.init(m_device, meshComponent, D3D11_BIND_VERTEX_BUFFER);
			if (FAILED(hr)) { ERROR("Main", "InitDevice", "Failed vertex buffer."); return hr; }
			hr = submesh.indexBuffer.init(m_device, meshComponent, D3D11_BIND_INDEX_BUFFER);
			if (FAILED(hr)) { ERROR("Main", "InitDevice", "Failed index buffer."); return hr; }
			submesh.indexCount = meshComponent.m_numIndex;
			submesh.startIndex = 0;
			submesh.materialSlot = 0;
			m_cyberGunRenderMesh.getSubmeshes().push_back(std::move(submesh));
		}

		// AABB local del modelo (para picking)
		m_modelLocalMin = EU::Vector3(1e9f, 1e9f, 1e9f);
		m_modelLocalMax = EU::Vector3(-1e9f, -1e9f, -1e9f);
		for (const MeshComponent& mc : cyberGunMeshes) {
			for (const SimpleVertex& v : mc.m_vertex) {
				m_modelLocalMin.x = fminf(m_modelLocalMin.x, v.Position.x);
				m_modelLocalMin.y = fminf(m_modelLocalMin.y, v.Position.y);
				m_modelLocalMin.z = fminf(m_modelLocalMin.z, v.Position.z);
				m_modelLocalMax.x = fmaxf(m_modelLocalMax.x, v.Position.x);
				m_modelLocalMax.y = fmaxf(m_modelLocalMax.y, v.Position.y);
				m_modelLocalMax.z = fmaxf(m_modelLocalMax.z, v.Position.z);
			}
		}
	}
	else {
		ERROR("Main", "InitDevice", "Failed to create CyberGun Actor.");
		return E_FAIL;
	}

	// --------------------------------------------------------------------------
	// SCENE GRAPH
	// --------------------------------------------------------------------------
	for (auto& actor : m_actors) {
		m_sceneGraph.addEntity(actor.get());
	}

	// --------------------------------------------------------------------------
	// SHADER BASE
	// --------------------------------------------------------------------------
	LayoutBuilder builder;
	builder.Add("POSITION", DXGI_FORMAT_R32G32B32_FLOAT)
		.Add("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT)
		.Add("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
		.Add("BITANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
		.Add("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

	hr = m_shaderProgram.init(m_device, "PBRShader.hlsl", builder);
	if (FAILED(hr)) { ERROR("Main", "InitDevice", "Failed ShaderProgram."); return hr; }

	hr = m_constantBuffer.init(m_device, sizeof(CBMain));
	if (FAILED(hr)) { ERROR("Main", "InitDevice", "Failed constant buffer."); return hr; }

	// --------------------------------------------------------------------------
	// CAMERA / LIGHT
	// --------------------------------------------------------------------------
	m_camera.setLens(XM_PIDIV4, m_window.m_width / (float)m_window.m_height, 0.01f, 100.0f);
	m_camera.setPosition(0.0f, 3.0f, -6.0f);

	m_constantBufferStruct.LightColor = EU::Vector3(1.0f, 1.0f, 1.0f);
	m_constantBufferStruct.LightDir = EU::Vector3(-0.20f, -1.0f, 1.0f);

	m_skybox.init(m_device, &m_deviceContext, m_skyboxTex);

	// --------------------------------------------------------------------------
	// ESTADOS POR DEFECTO
	// --------------------------------------------------------------------------
	hr = m_defaultRasterizer.init(m_device, D3D11_FILL_SOLID, D3D11_CULL_BACK, false, true);
	if (FAILED(hr)) { ERROR("Main", "InitDevice", "Failed Rasterizer."); return hr; }

	hr = m_defaultDepthStencil.init(m_device, true, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS);
	if (FAILED(hr)) { ERROR("Main", "InitDevice", "Failed DepthStencilState."); return hr; }

	hr = m_defaultSampler.init(m_device);
	if (FAILED(hr)) { ERROR("Main", "InitDevice", "Failed SamplerState."); return hr; }

	// --------------------------------------------------------------------------
	// MATERIALES
	// --------------------------------------------------------------------------
	m_pbrMaterial.setShader(&m_shaderProgram);
	m_pbrMaterial.setRasterizerState(&m_defaultRasterizer);
	m_pbrMaterial.setDepthStencilState(&m_defaultDepthStencil);
	m_pbrMaterial.setSamplerState(&m_defaultSampler);
	m_pbrMaterial.setDomain(MaterialDomain::Opaque);
	m_pbrMaterial.setBlendMode(BlendMode::Opaque);

	m_transparentPbrMaterial.setShader(&m_shaderProgram);
	m_transparentPbrMaterial.setRasterizerState(&m_defaultRasterizer);
	m_transparentPbrMaterial.setDepthStencilState(&m_defaultDepthStencil);
	m_transparentPbrMaterial.setSamplerState(&m_defaultSampler);
	m_transparentPbrMaterial.setDomain(MaterialDomain::Transparent);
	m_transparentPbrMaterial.setBlendMode(BlendMode::Alpha);

	m_cyberGunMaterial.setMaterial(&m_pbrMaterial);
	m_cyberGunMaterial.setAlbedo(&m_AlbedoSRV);
	m_cyberGunMaterial.setNormal(&m_NormalSRV);
	m_cyberGunMaterial.setMetallic(&m_MetallicSRV);
	m_cyberGunMaterial.setRoughness(&m_RoughnessSRV);
	m_cyberGunMaterial.setAO(&m_AOSRV);
	m_cyberGunMaterial.getParams().baseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_cyberGunMaterial.getParams().metallic = 1.0f;
	m_cyberGunMaterial.getParams().roughness = 1.0f;
	m_cyberGunMaterial.getParams().ao = 1.0f;
	m_cyberGunMaterial.getParams().normalScale = 1.0f;
	m_cyberGunMaterial.getParams().emissiveStrength = 1.0f;
	m_cyberGunMaterial.getParams().alphaCutoff = 0.5f;

	// --------------------------------------------------------------------------
	// MESH RENDERER COMPONENT
	// --------------------------------------------------------------------------
	EU::TSharedPointer<MeshRendererComponent> meshRenderer =
		m_cyberGun->getComponent<MeshRendererComponent>();
	if (!meshRenderer) {
		meshRenderer = EU::MakeShared<MeshRendererComponent>();
		m_cyberGun->addComponent(meshRenderer);
	}
	meshRenderer->setMesh(&m_cyberGunRenderMesh);
	meshRenderer->setMaterialInstance(&m_cyberGunMaterial);
	meshRenderer->setVisible(true);
	meshRenderer->setCastShadow(true);

	// --------------------------------------------------------------------------
	// LUZ DIRECCIONAL COMO ACTOR
	// --------------------------------------------------------------------------
	m_directionalLightActor = createLightActor(LightType::Directional, "DirectionalLight");

	loadScene(getDefaultScenePath());
	enforceDefaultSceneLayout();
	frameDefaultSceneCamera();

	// --------------------------------------------------------------------------
	// VIEWPORT PASS / RENDER PIPELINE (Deferred)
	// --------------------------------------------------------------------------
	hr = m_editorViewportPass.init(m_device, 1280, 720);
	if (FAILED(hr)) { ERROR("Main", "InitDevice", "Failed EditorViewportPass."); return hr; }

	hr = m_renderPipeline.init(m_device, RendererType::Deferred);
	if (FAILED(hr)) { ERROR("Main", "InitDevice", "Failed RenderPipeline."); return hr; }

	buildTextureThumbnails();

	return S_OK;
}

void
BaseApp::update(float deltaTime) {
	handleEditorViewportResize();

	if (!m_initialStateCaptured) {
		captureInitialState();
		m_initialStateCaptured = true;
	}

	// GUI
	m_gui.update(m_viewport, m_window);
	m_gui.drawViewportPanel(m_editorViewportPass.getSRV());

	if (!m_actors.empty() && m_gui.selectedActorIndex >= 0 &&
		m_gui.selectedActorIndex < (int)m_actors.size()) {
		if (m_gui.shouldShowInspector()) {
			m_gui.inspectorGeneral(m_actors[m_gui.selectedActorIndex]);
		}
		m_gui.editTransform(m_camera, m_window, m_actors[m_gui.selectedActorIndex]);
	}

	if (m_gui.shouldShowOutliner()) {
		m_gui.outliner(m_actors);
	}

	m_gui.drawGBufferDebugPanel(
		m_renderPipeline.getGBufferAlbedoMetallicSRV(),
		m_renderPipeline.getGBufferNormalRoughnessSRV(),
		m_renderPipeline.getGBufferWorldAoSRV(),
		m_renderPipeline.getGBufferEmissiveAlphaSRV());

	if (m_gui.shouldShowRenderDebug()) {
		m_gui.drawRenderDebugPanel(
			m_renderPipeline.getPreShadowSRV(),
			m_editorViewportPass.getSRV(),
			m_renderPipeline.getShadowMapSRV());
	}

	if (m_gui.shouldShowToolbox()) {
		m_gui.drawToolboxPanel();
	}

	m_gui.drawViewportGrid(m_camera);
	m_gui.drawLightingPanel(&m_constantBufferStruct.LightDir.x, &m_constantBufferStruct.LightColor.x);
	m_gui.drawStatsPanel(deltaTime, m_lastDrawCalls);
	m_gui.drawConsolePanel();
	m_gui.drawTexturePreview();
	m_gui.drawContentBrowser(m_thumbnails);

	// Instanciar modelo desde el Content Browser
	if (m_gui.m_assetSpawnRequested) {
		m_gui.m_assetSpawnRequested = false;
		EU::TSharedPointer<Actor> a = loadModelActor(m_gui.m_assetSpawnPath);
		if (!a.isNull()) {
			addActorToScene(a);
			m_commands.push(std::unique_ptr<ICommand>(new SpawnActorCommand(this, a)));
			m_gui.selectedActorIndex = (int)m_actors.size() - 1;
			MESSAGE("BaseApp", "loadModelActor", "Modelo instanciado desde Content");
		}
	}

	// Aplicar textura desde el Content Browser
	if (m_gui.m_textureDropRequested) {
		m_gui.m_textureDropRequested = false;
		if (m_gui.selectedActorIndex >= 0 && m_gui.selectedActorIndex < (int)m_actors.size()) {
			EU::TSharedPointer<Actor> selected = m_actors[m_gui.selectedActorIndex];
			if (!selected.isNull()) {
				EU::TSharedPointer<MeshRendererComponent> mr = selected->getComponent<MeshRendererComponent>();
				if (mr && mr->getMaterialInstance()) {
					Texture* newTex = new Texture();
					HRESULT hr = loadPbrTexture(*newTex, m_gui.m_textureDropPath, "DynamicTexture");
					if (SUCCEEDED(hr)) {
						m_dynamicTextures.push_back(std::unique_ptr<Texture>(newTex));
						mr->getMaterialInstance()->setAlbedo(newTex);
						MESSAGE("BaseApp", "update", "Textura aplicada al actor seleccionado");
					} else {
						delete newTex;
						ERROR("BaseApp", "update", "Fallo al cargar la textura");
					}
				} else {
					MESSAGE("BaseApp", "update", "Actor sin material");
				}
			}
		} else {
			MESSAGE("BaseApp", "update", "No hay actor seleccionado");
		}
	}

	// Recargar miniaturas si se importó contenido
	if (m_gui.m_importContentRequested) {
		m_gui.m_importContentRequested = false;
		buildTextureThumbnails();
	}

	if (m_gui.consumeResetRequest()) {
		resetSceneToDefaults();
	}

	if (m_gui.consumeSaveSceneRequest()) {
		saveScene(getDefaultScenePath());
	}

	LightType requestedLightType = LightType::Directional;
	if (m_gui.consumeCreateLightRequest(requestedLightType)) {
		std::string name = "Light";
		if (requestedLightType == LightType::Point) name = "PointLight";
		if (requestedLightType == LightType::Spot) name = "SpotLight";
		if (requestedLightType == LightType::Directional) name = "DirectionalLight";
		if (requestedLightType == LightType::Rect) name = "RectLight";

		EU::TSharedPointer<Actor> newLight = createLightActor(requestedLightType, name);
		if (!newLight.isNull()) {
			m_commands.push(std::unique_ptr<ICommand>(new SpawnActorCommand(this, newLight)));
		}
	}

	// --- Undo/Redo: registrar movimientos del gizmo ---
	{
		bool usingGizmo = m_gui.m_isUsingGizmo;
		int sel = m_gui.selectedActorIndex;

		if (usingGizmo && !m_prevGizmoUsing) {
			m_gizmoEditActorIndex = sel;
			m_gizmoEditing = captureGizmoState(sel, m_gizmoBefore);
		}
		else if (!usingGizmo && m_prevGizmoUsing && m_gizmoEditing) {
			GizmoEditState after;
			if (captureGizmoState(m_gizmoEditActorIndex, after)) {
				const float eps = 1e-4f;
				bool changed =
					fabsf(after.position.x - m_gizmoBefore.position.x) > eps ||
					fabsf(after.position.y - m_gizmoBefore.position.y) > eps ||
					fabsf(after.position.z - m_gizmoBefore.position.z) > eps ||
					fabsf(after.rotation.x - m_gizmoBefore.rotation.x) > eps ||
					fabsf(after.rotation.y - m_gizmoBefore.rotation.y) > eps ||
					fabsf(after.rotation.z - m_gizmoBefore.rotation.z) > eps ||
					fabsf(after.scale.x - m_gizmoBefore.scale.x) > eps ||
					fabsf(after.scale.y - m_gizmoBefore.scale.y) > eps ||
					fabsf(after.scale.z - m_gizmoBefore.scale.z) > eps;
				if (changed && m_gizmoEditActorIndex >= 0 &&
					m_gizmoEditActorIndex < (int)m_actors.size()) {
					EU::TSharedPointer<Actor> actor = m_actors[m_gizmoEditActorIndex];
					m_commands.push(std::unique_ptr<ICommand>(
						new TransformCommand(actor, m_gizmoBefore, after)));
				}
			}
			m_gizmoEditing = false;
		}
		m_prevGizmoUsing = usingGizmo;
	}

	// --- Atajos Undo/Redo ---
	{
		ImGuiIO& io = ImGui::GetIO();
		bool ctrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
		static bool zPrev = false, yPrev = false;
		bool zNow = (GetAsyncKeyState('Z') & 0x8000) != 0;
		bool yNow = (GetAsyncKeyState('Y') & 0x8000) != 0;

		bool menuUndo = m_gui.consumeUndoRequest();
		bool menuRedo = m_gui.consumeRedoRequest();
		bool doUndo = menuUndo || (ctrl && zNow && !zPrev && !io.WantTextInput);
		bool doRedo = menuRedo || (ctrl && yNow && !yPrev && !io.WantTextInput);

		if (doUndo) { m_commands.undo(); MESSAGE("BaseApp", "undo", "Deshacer"); }
		if (doRedo) { m_commands.redo(); MESSAGE("BaseApp", "redo", "Rehacer"); }

		zPrev = zNow; yPrev = yNow;
	}

	// --- Atajos Duplicar / Copiar / Pegar / Borrar ---
	{
		ImGuiIO& io = ImGui::GetIO();
		bool ctrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
		bool typing = io.WantTextInput;
		static bool dPrev = false, cPrev = false, vPrev = false, delPrev = false;
		bool dNow = (GetAsyncKeyState('D') & 0x8000) != 0;
		bool cNow = (GetAsyncKeyState('C') & 0x8000) != 0;
		bool vNow = (GetAsyncKeyState('V') & 0x8000) != 0;
		bool delNow = (GetAsyncKeyState(VK_DELETE) & 0x8000) != 0;

		bool doDup = (!typing && ctrl && dNow && !dPrev) || m_gui.m_duplicateRequested;
		bool doCopy = (!typing && ctrl && cNow && !cPrev) || m_gui.m_copyRequested;
		bool doPaste = (!typing && ctrl && vNow && !vPrev) || m_gui.m_pasteRequested;
		bool doDel = (!typing && delNow && !delPrev) || m_gui.m_deleteRequested;
		bool doSavePrefab = m_gui.m_savePrefabRequested;
		bool doLoadPrefab = m_gui.m_loadPrefabRequested;

		m_gui.m_duplicateRequested = false;
		m_gui.m_copyRequested = false;
		m_gui.m_pasteRequested = false;
		m_gui.m_deleteRequested = false;
		m_gui.m_savePrefabRequested = false;
		m_gui.m_loadPrefabRequested = false;

		if (doCopy) { MESSAGE("BaseApp", "input", "Copy detectado"); copySelected(); }
		if (doDup) { MESSAGE("BaseApp", "input", "Duplicate detectado"); duplicateSelected(); }
		if (doPaste) { MESSAGE("BaseApp", "input", "Paste detectado"); pasteClipboard(); }
		if (doDel) { MESSAGE("BaseApp", "input", "Delete detectado"); deleteSelected(); }
		if (doSavePrefab) savePrefabSelected();
		if (doLoadPrefab) loadPrefab();

		dPrev = dNow; cPrev = cNow; vPrev = vNow; delPrev = delNow;
	}

	// --- Navegacion de camara ---
	if (!m_gui.m_isUsingGizmo) {
		ImGuiIO& io = ImGui::GetIO();
		if (m_gui.m_viewportHovered) {
			if (io.MouseWheel != 0.0f) m_camera.walk(io.MouseWheel * 0.7f);

			const bool rightHeld = ImGui::IsMouseDown(ImGuiMouseButton_Right);

			if (rightHeld) {
				// Rotar camara con mouse
				m_camera.yaw(io.MouseDelta.x * 0.004f);
				m_camera.pitch(io.MouseDelta.y * 0.004f);

				// WASD + QE como Unreal Engine (solo activo con click derecho)
				const float speed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) ? 0.15f : 0.05f;
				if (GetAsyncKeyState('W') & 0x8000) m_camera.walk( speed);
				if (GetAsyncKeyState('S') & 0x8000) m_camera.walk(-speed);
				if (GetAsyncKeyState('A') & 0x8000) m_camera.strafe(-speed);
				if (GetAsyncKeyState('D') & 0x8000) m_camera.strafe( speed);
				if (GetAsyncKeyState('E') & 0x8000) {
					EU::Vector3 p = m_camera.getPosition(); p.y += speed; m_camera.setPosition(p);
				}
				if (GetAsyncKeyState('Q') & 0x8000) {
					EU::Vector3 p = m_camera.getPosition(); p.y -= speed; m_camera.setPosition(p);
				}
			}

			if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
				m_camera.strafe(-io.MouseDelta.x * 0.02f);
				EU::Vector3 p = m_camera.getPosition();
				p.y += io.MouseDelta.y * 0.02f;
				m_camera.setPosition(p);
			}
		}
	}

	// --- Focus (F) y Fit ---
	{
		EU::TSharedPointer<Actor> selected;
		if (!m_actors.empty() && m_gui.selectedActorIndex >= 0 &&
			m_gui.selectedActorIndex < (int)m_actors.size()) {
			selected = m_actors[m_gui.selectedActorIndex];
		}
		static bool fDown = false;
		bool fNow = (GetAsyncKeyState('F') & 0x8000) != 0;
		if (m_gui.m_viewportHovered && fNow && !fDown) focusCameraOnActor(selected);
		fDown = fNow;
		if (m_gui.consumeFocusRequest()) focusCameraOnActor(selected);
		if (m_gui.consumeFitRequest())   fitCameraToScene();
	}

	// --- Picking (click izquierdo) ---
	if (m_gui.m_viewportHovered && !m_gui.m_isUsingGizmo &&
		ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver()) {
		pickActorFromMouse();
	}

	// --- Outline del objeto seleccionado (borde 3D) ---
	if (!m_actors.empty() && m_gui.selectedActorIndex >= 0 &&
		m_gui.selectedActorIndex < (int)m_actors.size()) {
		EU::TSharedPointer<Actor> sel = m_actors[m_gui.selectedActorIndex];
		if (!sel.isNull()) {
			EU::TSharedPointer<Transform> t = sel->getComponent<Transform>();
			EU::Vector3 mn, mx;
			if (t && getActorAABB(sel, mn, mx)) {
				m_gui.drawSelectionOutline(m_camera, mn, mx, t->worldMatrix);
			}
		}
	}

	// Resize estable del viewport
	unsigned int desiredW = static_cast<unsigned int>(m_gui.m_viewportSize.x);
	unsigned int desiredH = static_cast<unsigned int>(m_gui.m_viewportSize.y);
	const unsigned int kMinViewportSize = 64;
	if (desiredW < kMinViewportSize) desiredW = kMinViewportSize;
	if (desiredH < kMinViewportSize) desiredH = kMinViewportSize;
	if (desiredW != m_lastRequestedViewportWidth || desiredH != m_lastRequestedViewportHeight) {
		m_lastRequestedViewportWidth = desiredW;
		m_lastRequestedViewportHeight = desiredH;
		m_viewportResizeStableFrames = 0;
	}
	else {
		m_viewportResizeStableFrames++;
	}
	const int kStableFramesRequired = 2;
	if (m_viewportResizeStableFrames >= kStableFramesRequired) {
		if (desiredW != m_editorViewportPass.getWidth() ||
			desiredH != m_editorViewportPass.getHeight()) {
			m_editorViewportResizePending = true;
			m_pendingViewportWidth = desiredW;
			m_pendingViewportHeight = desiredH;
		}
	}

	m_camera.updateViewMatrix();

	// Actualizar aspect ratio de la camara al tamano real del viewport editor
	if (m_gui.m_viewportSize.x > 64.0f && m_gui.m_viewportSize.y > 64.0f) {
		float viewportAspect = m_gui.m_viewportSize.x / m_gui.m_viewportSize.y;
		m_camera.setLens(XM_PIDIV4, viewportAspect, 0.01f, 100.0f);
	}

	XMStoreFloat4x4(&m_constantBufferStruct.View, XMMatrixTranspose(m_camera.getView()));
	XMStoreFloat4x4(&m_constantBufferStruct.Projection, XMMatrixTranspose(m_camera.getProj()));
	m_constantBufferStruct.CameraPos = m_camera.getPosition();

	syncLightActors();

	m_skybox.update(m_deviceContext, m_camera);
	m_constantBuffer.update(m_deviceContext, nullptr, 0, nullptr, &m_constantBufferStruct, 0, 0);
	m_sceneGraph.update(deltaTime, m_deviceContext);
}

void
BaseApp::render() {
	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	m_deviceContext.m_drawCallCount = 0;

	m_renderScene.clear();
	m_sceneGraph.gatherRenderScene(m_renderScene, m_camera);
	m_renderScene.skybox = &m_skybox;

	m_renderPipeline.setShadowFactorDebugEnabled(m_gui.m_visualizeDeferredShadowFactor);
	m_renderPipeline.render(m_deviceContext, m_camera, m_renderScene, m_editorViewportPass);

	m_lastDrawCalls = m_deviceContext.m_drawCallCount;

	m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);
	m_viewport.render(m_deviceContext);
	m_depthStencilView.render(m_deviceContext);
	m_gui.render();
	m_swapChain.present();
}

void
BaseApp::destroy() {
	if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();
	m_sceneGraph.destroy();
	m_renderPipeline.destroy();
	m_editorViewportPass.destroy();
	m_cyberGunRenderMesh.destroy();
	m_AlbedoSRV.destroy();
	m_MetallicSRV.destroy();
	m_NormalSRV.destroy();
	m_RoughnessSRV.destroy();
	m_AOSRV.destroy();
	m_EmissiveSRV.destroy();
	m_defaultRasterizer.destroy();
	m_defaultDepthStencil.destroy();
	m_defaultSampler.destroy();
	m_shaderProgram.destroy();
	m_depthStencil.destroy();
	m_depthStencilView.destroy();
	m_renderTargetView.destroy();
	m_swapChain.destroy();
	m_backBuffer.destroy();
	if (m_guiInitialized) {
		m_gui.destroy();
		m_guiInitialized = false;
	}
	delete m_model;
	m_model = nullptr;
	for (auto& lm : m_loadedModels) {
		if (lm) {
			lm->mesh.destroy();
			lm->albedo.destroy(); lm->normal.destroy(); lm->metallic.destroy();
			lm->roughness.destroy(); lm->ao.destroy();
		}
	}
	m_loadedModels.clear();
	for (auto& tex : m_thumbTextures) tex.destroy();
	m_thumbTextures.clear();

	m_deviceContext.destroy();
	m_device.destroy();
}

LRESULT
BaseApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam)) {
		return true;
	}
	switch (message) {
	case WM_CREATE: {
		CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCreate->lpCreateParams);
	}
	return 0;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
	}
	return 0;
	case WM_SIZE:
	{
		if (wParam == SIZE_MINIMIZED) return 0;
		UINT newW = LOWORD(lParam);
		UINT newH = HIWORD(lParam);
		if (newW == 0 || newH == 0) return 0;
		BaseApp* app = reinterpret_cast<BaseApp*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (app) app->onResize(newW, newH);
		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void
BaseApp::onResize(unsigned int newW, unsigned int newH) {
	if (!m_d3dReady) {
		m_window.m_width = (int)newW;
		m_window.m_height = (int)newH;
		return;
	}
	if (!m_deviceContext.m_deviceContext || !m_swapChain.m_swapChain) return;
	if (newW == 0 || newH == 0) return;

	m_window.m_width = (int)newW;
	m_window.m_height = (int)newH;

	ID3D11RenderTargetView* nullRTV = nullptr;
	m_deviceContext.m_deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);

	m_renderTargetView.destroy();
	m_depthStencilView.destroy();
	m_depthStencil.destroy();
	m_backBuffer.destroy();

	HRESULT hr = m_swapChain.resizeBuffers(newW, newH);
	if (FAILED(hr)) return;
	hr = m_swapChain.getBackBuffer(m_backBuffer);
	if (FAILED(hr)) return;
	hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
	if (FAILED(hr)) return;
	hr = m_depthStencil.init(m_device, newW, newH, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, 4, 0);
	if (FAILED(hr)) return;
	hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
	if (FAILED(hr)) return;

	m_viewport.init(m_window);
	m_camera.setLens(XM_PIDIV4, newW / (float)newH, 0.01f, 100.0f);
}

void
BaseApp::handleEditorViewportResize() {
	if (!m_editorViewportResizePending) return;

	m_deviceContext.m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	ID3D11ShaderResourceView* nullSRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
	m_deviceContext.m_deviceContext->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullSRVs);

	EditorViewportPass newPass;
	HRESULT hr = newPass.init(m_device, m_pendingViewportWidth, m_pendingViewportHeight);
	if (FAILED(hr)) { m_editorViewportResizePending = false; return; }

	m_editorViewportPass.swap(newPass);
	m_renderPipeline.resize(m_device, m_pendingViewportWidth, m_pendingViewportHeight);
	m_editorViewportResizePending = false;
}

void
BaseApp::captureInitialState() {
	m_initialLightDir = m_constantBufferStruct.LightDir;
	m_initialLightColor = m_constantBufferStruct.LightColor;
	m_initialCameraPos = m_camera.getPosition();
	m_initialTransforms.clear();
	for (auto& actor : m_actors) {
		InitialTransform it{};
		if (!actor.isNull()) {
			EU::TSharedPointer<Transform> t = actor->getComponent<Transform>();
			if (t) {
				it.position = t->getPosition();
				it.rotation = t->getRotation();
				it.scale = t->getScale();
			}
		}
		m_initialTransforms.push_back(it);
	}
}

void
BaseApp::resetSceneToDefaults() {
	m_constantBufferStruct.LightDir = m_initialLightDir;
	m_constantBufferStruct.LightColor = m_initialLightColor;
	m_camera.setPosition(m_initialCameraPos.x, m_initialCameraPos.y, m_initialCameraPos.z);
	for (size_t i = 0; i < m_actors.size() && i < m_initialTransforms.size(); ++i) {
		if (m_actors[i].isNull()) continue;
		EU::TSharedPointer<Transform> t = m_actors[i]->getComponent<Transform>();
		if (t) {
			t->setPosition(m_initialTransforms[i].position);
			t->setRotation(m_initialTransforms[i].rotation);
			t->setScale(m_initialTransforms[i].scale);
		}
	}
	if (!m_directionalLightActor.isNull()) {
		EU::TSharedPointer<LightComponent> lc = m_directionalLightActor->getComponent<LightComponent>();
		if (lc) {
			lc->getLightData().direction = m_initialLightDir;
			lc->getLightData().color = m_initialLightColor;
		}
	}
	MESSAGE("BaseApp", "resetSceneToDefaults", "Escena restaurada");
}

void
BaseApp::focusCameraOnActor(const EU::TSharedPointer<Actor>& actor) {
	if (actor.isNull()) return;
	EU::TSharedPointer<Transform> t = actor->getComponent<Transform>();
	if (!t) return;
	EU::Vector3 target = t->getPosition();
	EU::Vector3 fwd = m_camera.GetForward();
	const float dist = 6.0f;
	EU::Vector3 eye(target.x - fwd.x * dist, target.y - fwd.y * dist, target.z - fwd.z * dist);
	m_camera.lookAt(eye, target);
	m_camera.setPosition(eye);
}

void
BaseApp::fitCameraToScene() {
	float minX = 1e9f, minY = 1e9f, minZ = 1e9f;
	float maxX = -1e9f, maxY = -1e9f, maxZ = -1e9f;
	int count = 0;
	for (auto& a : m_actors) {
		if (a.isNull()) continue;
		if (a->getComponent<MeshRendererComponent>().isNull()) continue;
		EU::TSharedPointer<Transform> t = a->getComponent<Transform>();
		if (!t) continue;
		EU::Vector3 p = t->getPosition();
		minX = fminf(minX, p.x); minY = fminf(minY, p.y); minZ = fminf(minZ, p.z);
		maxX = fmaxf(maxX, p.x); maxY = fmaxf(maxY, p.y); maxZ = fmaxf(maxZ, p.z);
		++count;
	}
	if (count == 0) return;
	EU::Vector3 center((minX + maxX) * 0.5f, (minY + maxY) * 0.5f, (minZ + maxZ) * 0.5f);
	float dx = maxX - minX, dy = maxY - minY, dz = maxZ - minZ;
	float radius = 0.5f * sqrtf(dx * dx + dy * dy + dz * dz) + 3.0f;
	float dist = radius / tanf(m_camera.getFovY() * 0.5f);
	EU::Vector3 fwd = m_camera.GetForward();
	EU::Vector3 eye(center.x - fwd.x * dist, center.y - fwd.y * dist, center.z - fwd.z * dist);
	m_camera.lookAt(eye, center);
	m_camera.setPosition(eye);
}

bool
BaseApp::captureGizmoState(int index, GizmoEditState& out) {
	if (index < 0 || index >= (int)m_actors.size()) return false;
	if (m_actors[index].isNull()) return false;
	EU::TSharedPointer<Transform> t = m_actors[index]->getComponent<Transform>();
	if (!t) return false;
	out.position = t->getPosition();
	out.rotation = t->getRotation();
	out.scale = t->getScale();
	return true;
}

void
BaseApp::pickActorFromMouse() {
	float vpX = m_gui.m_viewportPos.x;
	float vpY = m_gui.m_viewportPos.y;
	float vpW = m_gui.m_viewportSize.x;
	float vpH = m_gui.m_viewportSize.y;
	if (vpW < 1.0f || vpH < 1.0f) return;

	ImVec2 mouse = ImGui::GetIO().MousePos;
	float mx = mouse.x - vpX;
	float my = mouse.y - vpY;
	if (mx < 0.0f || my < 0.0f || mx > vpW || my > vpH) return;

	float ndcX = (2.0f * mx / vpW) - 1.0f;
	float ndcY = 1.0f - (2.0f * my / vpH);

	XMMATRIX view = m_camera.getView();
	XMMATRIX proj = m_camera.getProj();
	XMVECTOR det;
	XMMATRIX invVP = XMMatrixInverse(&det, view * proj);
	XMVECTOR nearP = XMVector3TransformCoord(XMVectorSet(ndcX, ndcY, 0.0f, 1.0f), invVP);
	XMVECTOR farP = XMVector3TransformCoord(XMVectorSet(ndcX, ndcY, 1.0f, 1.0f), invVP);

	XMFLOAT3 o, d;
	XMStoreFloat3(&o, nearP);
	XMVECTOR dirV = XMVector3Normalize(XMVectorSubtract(farP, nearP));
	XMStoreFloat3(&d, dirV);

	float bestT = 1e9f;
	int bestIndex = -1;

	for (int i = 0; i < (int)m_actors.size(); ++i) {
		if (m_actors[i].isNull()) continue;
		EU::TSharedPointer<MeshRendererComponent> mr = m_actors[i]->getComponent<MeshRendererComponent>();
		if (!mr || !mr->isVisible()) continue;
		EU::TSharedPointer<Transform> t = m_actors[i]->getComponent<Transform>();
		if (!t) continue;

		EU::Vector3 lmn, lmx;
		if (!getActorAABB(m_actors[i], lmn, lmx)) continue;

		XMMATRIX world = t->worldMatrix;
		float bmin[3] = { 1e9f, 1e9f, 1e9f };
		float bmax[3] = { -1e9f, -1e9f, -1e9f };
		for (int c = 0; c < 8; ++c) {
			float cx = (c & 1) ? lmx.x : lmn.x;
			float cy = (c & 2) ? lmx.y : lmn.y;
			float cz = (c & 4) ? lmx.z : lmn.z;
			XMVECTOR wc = XMVector3TransformCoord(XMVectorSet(cx, cy, cz, 1.0f), world);
			XMFLOAT3 f; XMStoreFloat3(&f, wc);
			bmin[0] = fminf(bmin[0], f.x); bmin[1] = fminf(bmin[1], f.y); bmin[2] = fminf(bmin[2], f.z);
			bmax[0] = fmaxf(bmax[0], f.x); bmax[1] = fmaxf(bmax[1], f.y); bmax[2] = fmaxf(bmax[2], f.z);
		}

		float ro[3] = { o.x, o.y, o.z };
		float rd[3] = { d.x, d.y, d.z };
		float tmin = 0.0f, tmax = 1e9f;
		bool hit = true;
		for (int a = 0; a < 3; ++a) {
			if (fabsf(rd[a]) < 1e-8f) {
				if (ro[a] < bmin[a] || ro[a] > bmax[a]) { hit = false; break; }
			}
			else {
				float inv = 1.0f / rd[a];
				float t1 = (bmin[a] - ro[a]) * inv;
				float t2 = (bmax[a] - ro[a]) * inv;
				if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
				tmin = fmaxf(tmin, t1);
				tmax = fminf(tmax, t2);
				if (tmin > tmax) { hit = false; break; }
			}
		}
		if (hit && tmin < bestT) { bestT = tmin; bestIndex = i; }
	}

	if (bestIndex >= 0) m_gui.selectedActorIndex = bestIndex;
}

void
BaseApp::addActorToScene(const EU::TSharedPointer<Actor>& actor) {
	if (actor.isNull()) return;
	m_actors.push_back(actor);
	m_sceneGraph.addEntity(actor.get());
}

void
BaseApp::removeActorFromScene(const EU::TSharedPointer<Actor>& actor) {
	if (actor.isNull()) return;
	m_sceneGraph.removeEntity(actor.get());
	for (size_t i = 0; i < m_actors.size(); ++i) {
		if (m_actors[i].get() == actor.get()) {
			m_actors.erase(m_actors.begin() + i);
			break;
		}
	}
	if (m_gui.selectedActorIndex >= (int)m_actors.size())
		m_gui.selectedActorIndex = (int)m_actors.size() - 1;
}

EU::TSharedPointer<Actor>
BaseApp::spawnPistol(const std::string& name,
	const EU::Vector3& pos, const EU::Vector3& rot, const EU::Vector3& scale) {
	EU::TSharedPointer<Actor> a = EU::MakeShared<Actor>(m_device);
	if (a.isNull()) return a;
	a->setName(name);
	EU::TSharedPointer<Transform> t = a->getComponent<Transform>();
	if (t) t->setTransform(pos, rot, scale);
	EU::TSharedPointer<MeshRendererComponent> mr = a->getComponent<MeshRendererComponent>();
	if (!mr) { mr = EU::MakeShared<MeshRendererComponent>(); a->addComponent(mr); }
	mr->setMesh(&m_cyberGunRenderMesh);
	mr->setMaterialInstance(&m_cyberGunMaterial);
	mr->setVisible(true);
	mr->setCastShadow(true);
	return a;
}

void
BaseApp::duplicateSelected() {
	int idx = m_gui.selectedActorIndex;
	if (idx < 0 || idx >= (int)m_actors.size() || m_actors[idx].isNull()) {
		MESSAGE("BaseApp", "duplicateSelected", "No hay actor seleccionado");
		return;
	}
	EU::TSharedPointer<Actor> src = m_actors[idx];
	EU::TSharedPointer<Transform> t = src->getComponent<Transform>();
	EU::Vector3 pos = t ? t->getPosition() : EU::Vector3(0, 0, 0);
	EU::Vector3 rot = t ? t->getRotation() : EU::Vector3(0, 0, 0);
	EU::Vector3 sca = t ? t->getScale() : EU::Vector3(1, 1, 1);
	pos.x += 1.5f;

	EU::TSharedPointer<Actor> a = EU::MakeShared<Actor>(m_device);
	a->setName(src->getName() + "_copy");
	EU::TSharedPointer<Transform> nt = a->getComponent<Transform>();
	if (nt) nt->setTransform(pos, rot, sca);

	EU::TSharedPointer<MeshRendererComponent> srcMr = src->getComponent<MeshRendererComponent>();
	if (srcMr) {
		EU::TSharedPointer<MeshRendererComponent> mr = EU::MakeShared<MeshRendererComponent>();
		a->addComponent(mr);
		mr->setMesh(srcMr->getMesh());
		if (srcMr->getMaterialInstance()) {
			MaterialInstance* newMat = new MaterialInstance(*srcMr->getMaterialInstance());
			m_dynamicMaterials.push_back(std::unique_ptr<MaterialInstance>(newMat));
			mr->setMaterialInstance(newMat);
		}
		mr->setVisible(srcMr->isVisible());
		mr->setCastShadow(srcMr->canCastShadow());
	}

	EU::TSharedPointer<LightComponent> srcLc = src->getComponent<LightComponent>();
	if (srcLc) {
		EU::TSharedPointer<LightComponent> lc = EU::MakeShared<LightComponent>();
		a->addComponent(lc);
		lc->getLightData() = srcLc->getLightData();
	}

	addActorToScene(a);
	m_commands.push(std::unique_ptr<ICommand>(new SpawnActorCommand(this, a)));
	m_gui.selectedActorIndex = (int)m_actors.size() - 1;
	MESSAGE("BaseApp", "duplicateSelected", "Actor duplicado");
}

void
BaseApp::deleteSelected() {
	int idx = m_gui.selectedActorIndex;
	if (idx < 0 || idx >= (int)m_actors.size() || m_actors[idx].isNull()) {
		MESSAGE("BaseApp", "deleteSelected", "No hay actor seleccionado");
		return;
	}
	EU::TSharedPointer<Actor> a = m_actors[idx];
	removeActorFromScene(a);
	m_commands.push(std::unique_ptr<ICommand>(new DeleteActorCommand(this, a)));
	MESSAGE("BaseApp", "deleteSelected", "Actor eliminado");
}

void
BaseApp::copySelected() {
	int idx = m_gui.selectedActorIndex;
	if (idx < 0 || idx >= (int)m_actors.size() || m_actors[idx].isNull()) {
		MESSAGE("BaseApp", "copySelected", "No hay actor seleccionado");
		return;
	}
	EU::TSharedPointer<Actor> src = m_actors[idx];
	EU::TSharedPointer<Transform> t = src->getComponent<Transform>();
	m_clipboard.name = src->getName();
	m_clipboard.position = t ? t->getPosition() : EU::Vector3(0, 0, 0);
	m_clipboard.rotation = t ? t->getRotation() : EU::Vector3(0, 0, 0);
	m_clipboard.scale = t ? t->getScale() : EU::Vector3(1, 1, 1);
	
	m_clipboard.hasMesh = false;
	m_clipboard.hasLight = false;
	
	EU::TSharedPointer<MeshRendererComponent> srcMr = src->getComponent<MeshRendererComponent>();
	if (srcMr) {
		m_clipboard.hasMesh = true;
		m_clipboard.mesh = srcMr->getMesh();
		if (srcMr->getMaterialInstance()) {
			m_clipboard.materialInstance = *srcMr->getMaterialInstance();
		}
	}
	EU::TSharedPointer<LightComponent> srcLc = src->getComponent<LightComponent>();
	if (srcLc) {
		m_clipboard.hasLight = true;
		m_clipboard.lightData = srcLc->getLightData();
	}

	m_hasClipboard = true;
	MESSAGE("BaseApp", "copySelected", "Actor copiado al portapapeles");
}

void
BaseApp::pasteClipboard() {
	if (!m_hasClipboard) {
		MESSAGE("BaseApp", "pasteClipboard", "Portapapeles vacio (usa Copy primero)");
		return;
	}
	EU::Vector3 pos = m_clipboard.position;
	pos.x += 1.5f;

	EU::TSharedPointer<Actor> a = EU::MakeShared<Actor>(m_device);
	a->setName(m_clipboard.name + "_paste");
	EU::TSharedPointer<Transform> nt = a->getComponent<Transform>();
	if (nt) nt->setTransform(pos, m_clipboard.rotation, m_clipboard.scale);

	if (m_clipboard.hasMesh) {
		EU::TSharedPointer<MeshRendererComponent> mr = EU::MakeShared<MeshRendererComponent>();
		a->addComponent(mr);
		mr->setMesh(m_clipboard.mesh);
		MaterialInstance* newMat = new MaterialInstance(m_clipboard.materialInstance);
		m_dynamicMaterials.push_back(std::unique_ptr<MaterialInstance>(newMat));
		mr->setMaterialInstance(newMat);
		mr->setVisible(true);
		mr->setCastShadow(true);
	}
	
	if (m_clipboard.hasLight) {
		EU::TSharedPointer<LightComponent> lc = EU::MakeShared<LightComponent>();
		a->addComponent(lc);
		lc->getLightData() = m_clipboard.lightData;
	}

	addActorToScene(a);
	m_commands.push(std::unique_ptr<ICommand>(new SpawnActorCommand(this, a)));
	m_gui.selectedActorIndex = (int)m_actors.size() - 1;
	MESSAGE("BaseApp", "pasteClipboard", "Actor pegado");
}

void
BaseApp::savePrefabSelected() {
	int idx = m_gui.selectedActorIndex;
	if (idx < 0 || idx >= (int)m_actors.size() || m_actors[idx].isNull()) return;
	EU::TSharedPointer<Actor> src = m_actors[idx];
	EU::TSharedPointer<Transform> t = src->getComponent<Transform>();
	if (!t) return;
	CreateDirectoryA("Saved", nullptr);
	std::ofstream f("Saved/actor.prefab", std::ios::trunc);
	if (!f.is_open()) { ERROR("BaseApp", "savePrefab", "No se pudo abrir el archivo"); return; }
	EU::Vector3 p = t->getPosition(), r = t->getRotation(), s = t->getScale();
	std::string n = src->getName();
	for (char& ch : n) if (ch == ' ') ch = '_';
	f << "PREFAB 1\n";
	f << "NAME " << n << "\n";
	f << "POSITION " << p.x << " " << p.y << " " << p.z << "\n";
	f << "ROTATION " << r.x << " " << r.y << " " << r.z << "\n";
	f << "SCALE " << s.x << " " << s.y << " " << s.z << "\n";
	MESSAGE("BaseApp", "savePrefab", "Prefab guardado en Saved/actor.prefab");
}

void
BaseApp::loadPrefab() {
	std::ifstream f("Saved/actor.prefab");
	if (!f.is_open()) { ERROR("BaseApp", "loadPrefab", "No existe Saved/actor.prefab"); return; }
	std::string token, name = "Prefab";
	EU::Vector3 p(0, 0, 0), r(0, 0, 0), s(1, 1, 1);
	int version = 0;
	f >> token >> version;
	while (f >> token) {
		if (token == "NAME") f >> name;
		else if (token == "POSITION") f >> p.x >> p.y >> p.z;
		else if (token == "ROTATION") f >> r.x >> r.y >> r.z;
		else if (token == "SCALE")    f >> s.x >> s.y >> s.z;
	}
	EU::TSharedPointer<Actor> a = spawnPistol(name, p, r, s);
	addActorToScene(a);
	m_commands.push(std::unique_ptr<ICommand>(new SpawnActorCommand(this, a)));
	m_gui.selectedActorIndex = (int)m_actors.size() - 1;
	MESSAGE("BaseApp", "loadPrefab", "Prefab cargado");
}

void
BaseApp::buildTextureThumbnails() {
	m_thumbTextures.clear();
	m_thumbnails.clear();
	m_thumbTextures.reserve(64);

	std::vector<std::string> folders = listSubfolders("Assets/Textures");
	folders.push_back("");
	int loaded = 0;
	for (const std::string& sub : folders) {
		std::string dir = sub.empty() ? "Assets/Textures" : ("Assets/Textures/" + sub);
		std::vector<std::string> files = listImageFiles(dir);
		for (const std::string& f : files) {
			if (loaded >= 48) break;
			std::string base = dir + "/" + stripExt(f);
			ExtensionType ext = extFromName(toLowerCopy(f));
			Texture tex;
			if (SUCCEEDED(tex.init(m_device, base, ext))) {
				m_thumbTextures.push_back(tex);
				AssetThumb th;
				th.name = f;
				th.srv = m_thumbTextures.back().m_textureFromImg;
				m_thumbnails.push_back(th);
				++loaded;
			}
		}
	}
	MESSAGE("BaseApp", "buildTextureThumbnails", "Thumbnails de texturas cargados");
}

void
BaseApp::loadModelTextures(LoadedModel& lm, const std::string& folder) {
	std::vector<std::string> files = listImageFiles(folder);
	for (const std::string& f : files) {
		std::string lower = toLowerCopy(f);
		std::string baseNoExt = toLowerCopy(stripExt(f));
		std::string path = folder + "/" + stripExt(f);
		ExtensionType ext = extFromName(lower);

		int slot = -1;
		if (containsStr(lower, "basecolor") || containsStr(lower, "albedo") || containsStr(lower, "diffuse")) slot = 0;
		else if (containsStr(lower, "normal")) slot = 1;
		else if (containsStr(lower, "roughness")) slot = 3;
		else if (containsStr(lower, "metallic") || containsStr(lower, "metalness")) slot = 2;
		else if (containsStr(lower, "occlusion")) slot = 4;
		else if (endsWith(baseNoExt, "_bc") || endsWith(baseNoExt, "_d") || endsWith(baseNoExt, "_alb")) slot = 0;
		else if (endsWith(baseNoExt, "_n") || endsWith(baseNoExt, "_nrm")) slot = 1;
		else if (endsWith(baseNoExt, "_r") || endsWith(baseNoExt, "_rgh")) slot = 3;
		else if (endsWith(baseNoExt, "_m") || endsWith(baseNoExt, "_met")) slot = 2;
		else if (endsWith(baseNoExt, "_ao") || endsWith(baseNoExt, "_o")) slot = 4;

		HRESULT hr;
		switch (slot) {
		case 0: hr = lm.albedo.init(m_device, path, ext);    if (SUCCEEDED(hr)) lm.materialInstance.setAlbedo(&lm.albedo); break;
		case 1: hr = lm.normal.init(m_device, path, ext);    if (SUCCEEDED(hr)) lm.materialInstance.setNormal(&lm.normal); break;
		case 2: hr = lm.metallic.init(m_device, path, ext);  if (SUCCEEDED(hr)) lm.materialInstance.setMetallic(&lm.metallic); break;
		case 3: hr = lm.roughness.init(m_device, path, ext); if (SUCCEEDED(hr)) lm.materialInstance.setRoughness(&lm.roughness); break;
		case 4: hr = lm.ao.init(m_device, path, ext);        if (SUCCEEDED(hr)) lm.materialInstance.setAO(&lm.ao); break;
		default: break;
		}
	}
	if (!lm.albedo.m_textureFromImg) {
		lm.materialInstance.setAlbedo(&m_AlbedoSRV);
		MESSAGE("BaseApp", "loadModelTextures", "Sin albedo en la carpeta; usando textura fallback");
	}
}

EU::TSharedPointer<Actor>
BaseApp::loadModelActor(const std::string& modelPath) {
	std::string lower = toLowerCopy(modelPath);
	ModelType type = FBX;
	if (endsWith(lower, ".obj")) type = OBJ;
	else if (endsWith(lower, ".glb") || endsWith(lower, ".gltf")) type = GLTF;

	Model3D model(modelPath, type);
	const std::vector<MeshComponent>& meshes = model.GetMeshes();
	if (meshes.empty()) {
		ERROR("BaseApp", "loadModelActor", "El modelo no tiene mallas");
		return EU::TSharedPointer<Actor>();
	}

	std::unique_ptr<LoadedModel> lm(new LoadedModel());

	HRESULT hr;
	for (const MeshComponent& mc : meshes) {
		Submesh sm{};
		hr = sm.vertexBuffer.init(m_device, mc, D3D11_BIND_VERTEX_BUFFER);
		if (FAILED(hr)) { ERROR("BaseApp", "loadModelActor", "Fallo vertex buffer"); return EU::TSharedPointer<Actor>(); }
		hr = sm.indexBuffer.init(m_device, mc, D3D11_BIND_INDEX_BUFFER);
		if (FAILED(hr)) { ERROR("BaseApp", "loadModelActor", "Fallo index buffer"); return EU::TSharedPointer<Actor>(); }
		sm.indexCount = mc.m_numIndex;
		sm.startIndex = 0;
		sm.materialSlot = 0;
		lm->mesh.getSubmeshes().push_back(std::move(sm));
	}
	lm->localMin = EU::Vector3(1e9f, 1e9f, 1e9f);
	lm->localMax = EU::Vector3(-1e9f, -1e9f, -1e9f);
	for (const MeshComponent& mc : meshes) {
		for (const SimpleVertex& v : mc.m_vertex) {
			lm->localMin.x = fminf(lm->localMin.x, v.Position.x);
			lm->localMin.y = fminf(lm->localMin.y, v.Position.y);
			lm->localMin.z = fminf(lm->localMin.z, v.Position.z);
			lm->localMax.x = fmaxf(lm->localMax.x, v.Position.x);
			lm->localMax.y = fmaxf(lm->localMax.y, v.Position.y);
			lm->localMax.z = fmaxf(lm->localMax.z, v.Position.z);
		}
	}

	lm->material.setShader(&m_shaderProgram);
	lm->material.setRasterizerState(&m_defaultRasterizer);
	lm->material.setDepthStencilState(&m_defaultDepthStencil);
	lm->material.setSamplerState(&m_defaultSampler);
	lm->material.setDomain(MaterialDomain::Opaque);
	lm->material.setBlendMode(BlendMode::Opaque);

	lm->materialInstance.setMaterial(&lm->material);
	lm->materialInstance.getParams().baseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	lm->materialInstance.getParams().metallic = 1.0f;
	lm->materialInstance.getParams().roughness = 1.0f;
	lm->materialInstance.getParams().ao = 1.0f;
	lm->materialInstance.getParams().normalScale = 1.0f;
	lm->materialInstance.getParams().emissiveStrength = 1.0f;
	lm->materialInstance.getParams().alphaCutoff = 0.5f;

	std::string modelName = fileBaseName(modelPath);
	loadModelTextures(*lm, "Assets/Textures/" + modelName);

	if (type == GLTF) {
		const auto& embTextures = model.GetEmbeddedTextures();
		for (const auto& emb : embTextures) {
			auto tex = std::make_unique<Texture>();
			HRESULT hrTex = tex->initFromMemory(m_device, emb.data.data(), emb.data.size(), emb.name);
			if (SUCCEEDED(hrTex)) {
				std::string lowerName = toLowerCopy(emb.name);
				if (lowerName.find("normal") != std::string::npos) {
					lm->materialInstance.setNormal(tex.get());
				} else if (lowerName.find("metallic") != std::string::npos || lowerName.find("orm") != std::string::npos || lowerName.find("roughness") != std::string::npos) {
					// GLTF usually combines metallic and roughness. We just assign to both if needed.
					lm->materialInstance.setMetallic(tex.get());
					lm->materialInstance.setRoughness(tex.get());
				} else if (lowerName.find("emissive") != std::string::npos) {
					lm->materialInstance.setEmissive(tex.get());
				} else {
					lm->materialInstance.setAlbedo(tex.get());
				}
				m_dynamicTextures.push_back(std::move(tex));
			}
		}
	}

	EU::TSharedPointer<Actor> a = EU::MakeShared<Actor>(m_device);
	if (a.isNull()) return a;
	a->setName(modelName);
	EU::TSharedPointer<Transform> t = a->getComponent<Transform>();
	if (t) t->setTransform(EU::Vector3(0.0f, 2.92f, 5.60f), EU::Vector3(0.0f, 0.0f, 0.0f), EU::Vector3(1.0f, 1.0f, 1.0f));
	EU::TSharedPointer<MeshRendererComponent> mr = a->getComponent<MeshRendererComponent>();
	if (!mr) { mr = EU::MakeShared<MeshRendererComponent>(); a->addComponent(mr); }
	mr->setMesh(&lm->mesh);
	mr->setMaterialInstance(&lm->materialInstance);
	mr->setVisible(true);
	mr->setCastShadow(true);

	m_loadedModels.push_back(std::move(lm));
	return a;
}

bool
BaseApp::getActorAABB(const EU::TSharedPointer<Actor>& actor, EU::Vector3& outMin, EU::Vector3& outMax) {
	if (actor.isNull()) return false;
	EU::TSharedPointer<MeshRendererComponent> mr = actor->getComponent<MeshRendererComponent>();
	if (!mr) return false;
	Mesh* mesh = mr->getMesh();
	if (!mesh) return false;

	if (mesh == &m_cyberGunRenderMesh) {
		outMin = m_modelLocalMin;
		outMax = m_modelLocalMax;
		return true;
	}
	for (auto& lm : m_loadedModels) {
		if (lm && &lm->mesh == mesh) {
			outMin = lm->localMin;
			outMax = lm->localMax;
			return true;
		}
	}
	return false;
}

std::string
BaseApp::getDefaultScenePath() const {
	CreateDirectoryA("Saved", nullptr);
	return "Saved/DefaultScene.wvscene";
}

bool
BaseApp::saveScene(const std::string& path) {
	enforceDefaultSceneLayout();

	std::ofstream stream(path, std::ios::trunc);
	if (!stream.is_open()) {
		ERROR("Main", "saveScene", ("Failed to open scene file for writing: " + path).c_str());
		return false;
	}

	stream << "WVSCENE 1\n";
	stream << "ACTOR_COUNT " << m_actors.size() << "\n";

	for (size_t actorIndex = 0; actorIndex < m_actors.size(); ++actorIndex) {
		const EU::TSharedPointer<Actor>& actor = m_actors[actorIndex];
		if (actor.isNull()) continue;

		stream << "ACTOR " << actorIndex << " " << std::quoted(actor->getName()) << "\n";

		EU::TSharedPointer<Transform> transform = actor->getComponent<Transform>();
		if (transform) {
			const EU::Vector3& position = transform->getPosition();
			const EU::Vector3& rotation = transform->getRotation();
			const EU::Vector3& scale = transform->getScale();
			stream << "POSITION " << position.x << " " << position.y << " " << position.z << "\n";
			stream << "ROTATION " << rotation.x << " " << rotation.y << " " << rotation.z << "\n";
			stream << "SCALE " << scale.x << " " << scale.y << " " << scale.z << "\n";
		}

		EU::TSharedPointer<MeshRendererComponent> meshRenderer =
			actor->getComponent<MeshRendererComponent>();
		if (meshRenderer) {
			stream << "VISIBLE " << (meshRenderer->isVisible() ? 1 : 0) << "\n";
			stream << "CAST_SHADOW " << (meshRenderer->canCastShadow() ? 1 : 0) << "\n";
		}

		EU::TSharedPointer<LightComponent> lightComponent =
			actor->getComponent<LightComponent>();
		if (lightComponent) {
			const LightData& light = lightComponent->getLightData();
			stream << "LIGHT " << static_cast<int>(light.type) << " "
				<< light.color.x << " " << light.color.y << " " << light.color.z << " "
				<< light.intensity << " "
				<< light.direction.x << " " << light.direction.y << " " << light.direction.z << " "
				<< light.range << "\n";
		}
	}

	MESSAGE("Main", "saveScene", ("Scene saved to " + path).c_str());
	return true;
}

bool
BaseApp::loadScene(const std::string& path) {
	std::ifstream stream(path);
	if (!stream.is_open()) return false;

	std::string token;
	int version = 0;
	stream >> token >> version;
	if (token != "WVSCENE" || version < 1) return false;

	int actorCount = 0;
	stream >> token >> actorCount;

	while (stream >> token) {
		if (token == "ACTOR") {
			int index;
			std::string name;
			stream >> index >> std::quoted(name);

			EU::TSharedPointer<Actor> actor;
			if (index < (int)m_actors.size()) {
				actor = m_actors[index];
			}
			if (!actor.isNull()) {
				actor->setName(name);
			}
		}
		else if (token == "POSITION") {
			float x, y, z;
			stream >> x >> y >> z;
			if (!m_actors.empty()) {
				EU::TSharedPointer<Actor>& lastActor = m_actors.back();
				if (!lastActor.isNull()) {
					EU::TSharedPointer<Transform> t = lastActor->getComponent<Transform>();
					if (t) t->setPosition(EU::Vector3(x, y, z));
				}
			}
		}
		else if (token == "ROTATION") {
			float x, y, z;
			stream >> x >> y >> z;
			if (!m_actors.empty()) {
				EU::TSharedPointer<Actor>& lastActor = m_actors.back();
				if (!lastActor.isNull()) {
					EU::TSharedPointer<Transform> t = lastActor->getComponent<Transform>();
					if (t) t->setRotation(EU::Vector3(x, y, z));
				}
			}
		}
		else if (token == "SCALE") {
			float x, y, z;
			stream >> x >> y >> z;
			if (!m_actors.empty()) {
				EU::TSharedPointer<Actor>& lastActor = m_actors.back();
				if (!lastActor.isNull()) {
					EU::TSharedPointer<Transform> t = lastActor->getComponent<Transform>();
					if (t) t->setScale(EU::Vector3(x, y, z));
				}
			}
		}
		else if (token == "LIGHT") {
			int type;
			float cx, cy, cz, intensity, dx, dy, dz, range;
			stream >> type >> cx >> cy >> cz >> intensity >> dx >> dy >> dz >> range;
			if (!m_directionalLightActor.isNull()) {
				EU::TSharedPointer<LightComponent> lc =
					m_directionalLightActor->getComponent<LightComponent>();
				if (lc) {
					LightData& light = lc->getLightData();
					light.type = static_cast<LightType>(type);
					light.color = EU::Vector3(cx, cy, cz);
					light.intensity = intensity;
					light.direction = EU::Vector3(dx, dy, dz);
					light.range = range;
				}
			}
		}
	}

	MESSAGE("Main", "loadScene", ("Scene loaded from " + path).c_str());
	return true;
}
