#include "BaseApp.h"
#include "ResourceManager.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>

HRESULT
BaseApp::awake() {
	HRESULT hr = S_OK;

	// Inicializacion de dlls y elementos externos al motor.
	m_sceneGraph.init();

	// Log Success Message
	MESSAGE("Main", "Awake", "Application awake successfully.");
	return hr;
}

int
BaseApp::run(HINSTANCE hInst, int nCmdShow) {
	// 1) Initialize Window
	if (FAILED(m_window.init(hInst, nCmdShow, WndProc, this))) {
		ERROR("Main", "Run", "Failed to initialize window.");
		return 0;
	}
	// 2) Awake Application
	if (FAILED(awake())) {
		ERROR("Main", "Run", "Failed to awake application.");
		return 0;
	}
	// 3) Initialize Device and Device Context
	if (FAILED(init())) {
		ERROR("Main", "Run", "Failed to initialize device and device context.");
		return 0;
	}
	// 4) Initialize GUI
	m_gui.init(m_window, m_device, m_deviceContext);
	m_guiInitialized = true;

	// Main message loop
	MSG msg = {};
	LARGE_INTEGER freq, prev;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&prev);
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
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

	// Crear swapchain
	hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window);

	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize SwpaChian. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	// Crear render target view
	hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);

	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize RenderTargetView. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	// Crear textura de depth stencil
	hr = m_depthStencil.init(m_device,
		m_window.m_width,
		m_window.m_height,
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		D3D11_BIND_DEPTH_STENCIL,
		4,
		16);

	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize DepthStencil. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	// Crear el depth stencil view
	hr = m_depthStencilView.init(m_device,
		m_depthStencil,
		DXGI_FORMAT_D24_UNORM_S8_UINT);

	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize DepthStencilView. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	// Crear el m_viewport
	hr = m_viewport.init(m_window);

	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize Viewport. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}
	m_d3dReady = true;

	// Load Resources -> Skybox
	std::array<std::string, 6> faces = {
		"Skybox/cubemap_0.png", 
		"Skybox/cubemap_1.png",
		"Skybox/cubemap_2.png",
		"Skybox/cubemap_3.png",
		"Skybox/cubemap_4.png",
		"Skybox/cubemap_5.png"
	};
	m_skyboxTex.CreateCubemap(m_device, m_deviceContext, faces, false);

	// Set CyberGun Actor
	m_cyberGun = EU::MakeShared<Actor>(m_device);

	if (!m_cyberGun.isNull()) {
		m_model = new Model3D("Assets/Models/CyberGun.fbx", ModelType::FBX);
		if (!m_model || !m_model->load("Assets/Models/CyberGun.fbx")) {
			ERROR("Main", "InitDevice", "Failed to load CyberGun model.");
			return E_FAIL;
		}

		// Cargando texturas desde Assets/Textures/CyberGun/
		hr = m_AlbedoSRV.init(m_device, "Assets/Textures/CyberGun/base.tga", PNG);
		if (FAILED(hr)) {
			ERROR("Main", "InitDevice", ("Failed to initialize CyberGun Albedo. HRESULT: " + std::to_string(hr)).c_str());
			return hr;
		}
		hr = m_MetallicSRV.init(m_device, "Assets/Textures/CyberGun/metallic.tga", PNG);
		if (FAILED(hr)) {
			ERROR("Main", "InitDevice", ("Failed to initialize CyberGun Metallic. HRESULT: " + std::to_string(hr)).c_str());
			return hr;
		}
		hr = m_RoughnessSRV.init(m_device, "Assets/Textures/CyberGun/roughness.tga", PNG);
		if (FAILED(hr)) {
			ERROR("Main", "InitDevice", ("Failed to initialize CyberGun Roughness. HRESULT: " + std::to_string(hr)).c_str());
			return hr;
		}
		hr = m_AOSRV.init(m_device, "Assets/Textures/CyberGun/ao.tga", PNG);
		if (FAILED(hr)) {
			ERROR("Main", "InitDevice", ("Failed to initialize CyberGun AO. HRESULT: " + std::to_string(hr)).c_str());
			return hr;
		}
		hr = m_NormalSRV.init(m_device, "Assets/Textures/CyberGun/normal.tga", PNG);
		if (FAILED(hr)) {
			ERROR("Main", "InitDevice", ("Failed to initialize CyberGun Normal. HRESULT: " + std::to_string(hr)).c_str());
			return hr;
		}
		HRESULT emissiveHr = m_EmissiveSRV.init(m_device, "Assets/Textures/CyberGun/Emissive.tga", PNG);
		if (FAILED(emissiveHr)) {
			MESSAGE("Main", "InitDevice", "CyberGun emissive texture not found. Continuing without emissive map.");
		}

		m_cyberGun->setName("CyberGun");
		m_actors.push_back(m_cyberGun);

		m_cyberGun->getComponent<Transform>()->setTransform(EU::Vector3(2.0f, -1.90f, 11.60f),
			EU::Vector3(-0.60f, 3.0f, -0.20f),
			EU::Vector3(1.0f, 1.0f, 1.0f));
	}
	else {
		ERROR("Main", "InitDevice", "Failed to create cyber Gun Actor.");
		return E_FAIL;
	}

	// Store the Actors in the Scene Graph
	for (auto& actor : m_actors) {
		m_sceneGraph.addEntity(actor.get());
	}

	LayoutBuilder builder;
	builder.Add("POSITION", DXGI_FORMAT_R32G32B32_FLOAT)
				 .Add("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT)
				 .Add("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
				 .Add("BITANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
				 .Add("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

	// Create the Shader Program
	hr = m_shaderProgram.init(m_device, "PBRShader.hlsl", builder);
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice", ("Failed to initialize ShaderProgram. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	// Create the constant buffers
	hr = m_constantBuffer.init(m_device, sizeof(CBMain));
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice", ("Failed to initialize m_constantBuffer Buffer. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}
	
	m_camera.setLens(XM_PIDIV4, m_window.m_width / (float)m_window.m_height, 0.01f, 100.0f);
	m_camera.setPosition(0.0f, 3.0f, -6.0f);

	m_constantBufferStruct.LightColor = EU::Vector3(1.0f, 1.0f, 1.0f);
	m_constantBufferStruct.LightDir = EU::Vector3(-0.20f, -1.0f, 1.0f);

	// Initialize the Skybox pass
	m_skybox.init(m_device, &m_deviceContext, m_skyboxTex);

	// Initialize default states
	hr = m_defaultRasterizer.init(m_device, D3D11_FILL_SOLID, D3D11_CULL_BACK, false, true);
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice", ("Failed to initialize default Rasterizer. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}
	hr = m_defaultDepthStencil.init(m_device, true, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS);
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice", ("Failed to initialize default DepthStencilState. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}
	hr = m_defaultSampler.init(m_device);
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice", ("Failed to initialize default SamplerState. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	m_pbrMaterial.setShader(&m_shaderProgram);
	m_pbrMaterial.setRasterizerState(&m_defaultRasterizer);
	m_pbrMaterial.setDepthStencilState(&m_defaultDepthStencil);
	m_pbrMaterial.setSamplerState(&m_defaultSampler);
	m_pbrMaterial.setDomain(MaterialDomain::Opaque);
	m_pbrMaterial.setBlendMode(BlendMode::Opaque);

	m_cyberGunMaterial.setMaterial(&m_pbrMaterial);
	m_cyberGunMaterial.setAlbedo(&m_AlbedoSRV);
	m_cyberGunMaterial.setNormal(&m_NormalSRV);
	m_cyberGunMaterial.setMetallic(&m_MetallicSRV);
	m_cyberGunMaterial.setRoughness(&m_RoughnessSRV);
	m_cyberGunMaterial.setAO(&m_AOSRV);
	if (m_EmissiveSRV.m_textureFromImg) {
		m_cyberGunMaterial.setEmissive(&m_EmissiveSRV);
	}
	m_cyberGunMaterial.getParams().baseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_cyberGunMaterial.getParams().metallic = 1.0f;
	m_cyberGunMaterial.getParams().roughness = 1.0f;
	m_cyberGunMaterial.getParams().ao = 1.0f;
	m_cyberGunMaterial.getParams().normalScale = 1.0f;
	m_cyberGunMaterial.getParams().emissiveStrength = 1.0f;
	m_cyberGunMaterial.getParams().alphaCutoff = 0.5f;

	m_cyberGunRenderMesh.destroy();
	for (const MeshComponent& meshComponent : m_model->GetMeshes()) {
		Submesh submesh{};
		hr = submesh.vertexBuffer.init(m_device, meshComponent, D3D11_BIND_VERTEX_BUFFER);
		if (FAILED(hr)) {
			ERROR("Main", "InitDevice", ("Failed to initialize CyberGun vertex buffer. HRESULT: " + std::to_string(hr)).c_str());
			return hr;
		}

		hr = submesh.indexBuffer.init(m_device, meshComponent, D3D11_BIND_INDEX_BUFFER);
		if (FAILED(hr)) {
			ERROR("Main", "InitDevice", ("Failed to initialize CyberGun index buffer. HRESULT: " + std::to_string(hr)).c_str());
			return hr;
		}

		submesh.indexCount = meshComponent.m_numIndex;
		submesh.materialSlot = 0;
		m_cyberGunRenderMesh.getSubmeshes().push_back(std::move(submesh));
	}

	EU::TSharedPointer<MeshRendererComponent> meshRenderer = m_cyberGun->getComponent<MeshRendererComponent>();
	if (!meshRenderer) {
		meshRenderer = EU::MakeShared<MeshRendererComponent>();
		m_cyberGun->addComponent(meshRenderer);
	}
	meshRenderer->setMesh(&m_cyberGunRenderMesh);
	meshRenderer->setMaterialInstance(&m_cyberGunMaterial);
	meshRenderer->setVisible(true);
	meshRenderer->setCastShadow(true);

	m_directionalLightActor = EU::MakeShared<Actor>(m_device);
	if (!m_directionalLightActor.isNull()) {
		m_directionalLightActor->setName("DirectionalLight");
		EU::TSharedPointer<LightComponent> lightComponent = m_directionalLightActor->getComponent<LightComponent>();
		if (!lightComponent) {
			lightComponent = EU::MakeShared<LightComponent>();
			m_directionalLightActor->addComponent(lightComponent);
		}
		lightComponent->getLightData().type = LightType::Directional;
		lightComponent->getLightData().direction = m_constantBufferStruct.LightDir;
		lightComponent->getLightData().color = m_constantBufferStruct.LightColor;
		lightComponent->getLightData().intensity = 1.0f;
		lightComponent->setCastShadow(false);
		m_sceneGraph.addEntity(m_directionalLightActor.get());
	}

	loadScene(getDefaultScenePath());

	hr = m_editorViewportPass.init(m_device, 1280, 720);
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice", ("Failed to initialize EditorViewportPass. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	hr = m_forwardRenderer.init(m_device);
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice", ("Failed to initialize ForwardRenderer. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	return S_OK;
}

void BaseApp::update(float deltaTime) {
	static float t = 0.0f;
	if (m_swapChain.m_driverType == D3D_DRIVER_TYPE_REFERENCE)
	{
		t += (float)XM_PI * 0.0125f;
	}
	else
	{
		static DWORD dwTimeStart = 0;
		DWORD dwTimeCur = GetTickCount();
		if (dwTimeStart == 0) dwTimeStart = dwTimeCur;
		t = (dwTimeCur - dwTimeStart) / 1000.0f;
	}
	
	m_gui.update(m_viewport, m_window);
	m_gui.drawViewportPanel(m_editorViewportPass.getSRV());
	m_gui.outliner(m_actors);
	
	EU::TSharedPointer<Actor> selectedActor;
	if (m_gui.selectedActorIndex >= 0 && m_gui.selectedActorIndex < static_cast<int>(m_actors.size())) {
		selectedActor = m_actors[m_gui.selectedActorIndex];
	}
	
	m_gui.inspectorGeneral(selectedActor);
	m_gui.editTransform(m_camera, m_window, selectedActor);
	
	if (m_gui.consumeSaveSceneRequest()) {
		saveScene(getDefaultScenePath());
	}

	unsigned int desiredW = static_cast<unsigned int>(m_gui.m_viewportSize.x);
	unsigned int desiredH = static_cast<unsigned int>(m_gui.m_viewportSize.y);
	const unsigned int kMinViewportSize = 64;

	if (desiredW < kMinViewportSize) desiredW = kMinViewportSize;
	if (desiredH < kMinViewportSize) desiredH = kMinViewportSize;

	if (desiredW != m_lastRequestedViewportWidth || desiredH != m_lastRequestedViewportHeight)
	{
		m_lastRequestedViewportWidth = desiredW;
		m_lastRequestedViewportHeight = desiredH;
		m_viewportResizeStableFrames = 0;
	}
	else
	{
		m_viewportResizeStableFrames++;
	}

	if (m_viewportResizeStableFrames >= 2)
	{
		if (desiredW != m_editorViewportPass.getWidth() || desiredH != m_editorViewportPass.getHeight())
		{
			m_editorViewportResizePending = true;
			m_pendingViewportWidth = desiredW;
			m_pendingViewportHeight = desiredH;
		}
	}

	m_camera.updateViewMatrix();
	XMStoreFloat4x4(&m_constantBufferStruct.View, XMMatrixTranspose(m_camera.getView()));
	XMStoreFloat4x4(&m_constantBufferStruct.Projection, XMMatrixTranspose(m_camera.getProj()));
	m_constantBufferStruct.CameraPos = m_camera.getPosition();
	
	m_gui.vec3Control("Light Direction", &m_constantBufferStruct.LightDir.x, 0.1f);
	m_gui.vec3Control("Light Color", &m_constantBufferStruct.LightColor.x, 0.1f);
	
	if (!m_directionalLightActor.isNull()) {
		EU::TSharedPointer<LightComponent> lightComponent = m_directionalLightActor->getComponent<LightComponent>();
		if (lightComponent) {
			lightComponent->getLightData().direction = m_constantBufferStruct.LightDir;
			lightComponent->getLightData().color = m_constantBufferStruct.LightColor;
		}
	}

	m_skybox.update(m_deviceContext, m_camera);
	m_sceneGraph.update(deltaTime, m_deviceContext);
}

void BaseApp::render() {
	handleEditorViewportResize();
	float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

	m_renderScene.clear();
	m_sceneGraph.gatherRenderScene(m_renderScene, m_camera);
	m_renderScene.skybox = &m_skybox;
	m_forwardRenderer.render(m_deviceContext, m_camera, m_renderScene, m_editorViewportPass);

	m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);
	m_viewport.render(m_deviceContext);
	m_depthStencilView.render(m_deviceContext);

	m_gui.render();
	m_swapChain.present();
}

void BaseApp::destroy() {
	if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();
	m_sceneGraph.destroy();
	m_editorViewportPass.destroy();
	m_forwardRenderer.destroy();
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
	m_deviceContext.destroy();
	m_device.destroy();
}

LRESULT BaseApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam)) return true;
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
	case WM_SIZE: {
		if (wParam == SIZE_MINIMIZED) return 0;
		unsigned int newW = LOWORD(lParam);
		unsigned int newH = HIWORD(lParam);
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

void BaseApp::onResize(unsigned int newW, unsigned int newH) {
	if (!m_d3dReady) {
		m_window.m_width = (int)newW;
		m_window.m_height = (int)newH;
		return;
	}
	if (!m_deviceContext.m_deviceContext || !m_swapChain.m_swapChain) return;
	m_window.m_width = (int)newW;
	m_window.m_height = (int)newH;
	ID3D11RenderTargetView* nullRTV = nullptr;
	m_deviceContext.m_deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);
	m_renderTargetView.destroy();
	m_depthStencilView.destroy();
	m_depthStencil.destroy();
	m_backBuffer.destroy();
	m_swapChain.resizeBuffers(newW, newH);
	m_swapChain.getBackBuffer(m_backBuffer);
	m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_depthStencil.init(m_device, newW, newH, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, 4, 16);
	m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
	m_viewport.init(m_window);
	m_camera.setLens(XM_PIDIV4, newW / (float)newH, 0.01f, 100.0f);
}

void BaseApp::handleEditorViewportResize() {
	if (!m_editorViewportResizePending) return;
	m_deviceContext.m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	ID3D11ShaderResourceView* nullSRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
	m_deviceContext.m_deviceContext->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullSRVs);
	EditorViewportPass newPass;
	if (SUCCEEDED(newPass.init(m_device, m_pendingViewportWidth, m_pendingViewportHeight))) {
		m_editorViewportPass.swap(newPass);
	}
	m_editorViewportResizePending = false;
}

std::string BaseApp::getDefaultScenePath() const {
	CreateDirectoryA("Saved", nullptr);
	return "Saved/DefaultScene.wvscene";
}

bool BaseApp::saveScene(const std::string& path) {
	std::ofstream stream(path, std::ios::trunc);
	if (!stream.is_open()) return false;
	stream << "WVSCENE 1\nACTOR_COUNT " << m_actors.size() << "\n";
	for (size_t i = 0; i < m_actors.size(); ++i) {
		const auto& actor = m_actors[i];
		if (actor.isNull()) continue;
		stream << "ACTOR " << i << " " << std::quoted(actor->getName()) << "\n";
		auto transform = actor->getComponent<Transform>();
		if (transform) {
			auto pos = transform->getPosition(); auto rot = transform->getRotation(); auto scl = transform->getScale();
			stream << "POSITION " << pos.x << " " << pos.y << " " << pos.z << "\n";
			stream << "ROTATION " << rot.x << " " << rot.y << " " << rot.z << "\n";
			stream << "SCALE " << scl.x << " " << scl.y << " " << scl.z << "\n";
		}
		auto meshRenderer = actor->getComponent<MeshRendererComponent>();
		if (meshRenderer) {
			stream << "VISIBLE " << (meshRenderer->isVisible() ? 1 : 0) << "\nCAST_SHADOW " << (meshRenderer->canCastShadow() ? 1 : 0) << "\n";
			const auto& materials = meshRenderer->getMaterialInstances();
			stream << "MATERIAL_COUNT " << materials.size() << "\n";
			for (size_t j = 0; j < materials.size(); ++j) {
				if (!materials[j]) { stream << "MATERIAL " << j << " 0 0 1 1 1 1 0 1 1 1 0.5\n"; continue; }
				auto mat = materials[j]->getMaterial(); auto& p = materials[j]->getParams();
				stream << "MATERIAL " << j << " " << (mat ? (int)mat->getDomain() : 0) << " " << (mat ? (int)mat->getBlendMode() : 0) << " " 
					   << p.baseColor.x << " " << p.baseColor.y << " " << p.baseColor.z << " " << p.baseColor.w << " " 
					   << p.metallic << " " << p.roughness << " " << p.ao << " " << p.normalScale << " " << p.alphaCutoff << "\n";
			}
		}
		stream << "END_ACTOR\n";
	}
	stream << "LIGHT " << m_constantBufferStruct.LightDir.x << " " << m_constantBufferStruct.LightDir.y << " " << m_constantBufferStruct.LightDir.z << " "
		   << m_constantBufferStruct.LightColor.x << " " << m_constantBufferStruct.LightColor.y << " " << m_constantBufferStruct.LightColor.z << "\nEND_SCENE\n";
	return true;
}

bool BaseApp::loadScene(const std::string& path) {
	std::ifstream stream(path);
	if (!stream.is_open()) return false;
	std::string token; int version;
	stream >> token >> version;
	if (token != "WVSCENE" || version != 1) return false;
	EU::TSharedPointer<Actor> currentActor;
	while (stream >> token) {
		if (token == "ACTOR") {
			size_t idx; std::string name; stream >> idx >> std::quoted(name);
			currentActor = (idx < m_actors.size()) ? m_actors[idx] : EU::TSharedPointer<Actor>();
			if (!currentActor.isNull()) currentActor->setName(name);
		}
		else if (token == "POSITION" && !currentActor.isNull()) { float x, y, z; stream >> x >> y >> z; auto t = currentActor->getComponent<Transform>(); if (t) t->setPosition({x,y,z}); }
		else if (token == "ROTATION" && !currentActor.isNull()) { float x, y, z; stream >> x >> y >> z; auto t = currentActor->getComponent<Transform>(); if (t) t->setRotation({x,y,z}); }
		else if (token == "SCALE" && !currentActor.isNull()) { float x, y, z; stream >> x >> y >> z; auto t = currentActor->getComponent<Transform>(); if (t) t->setScale({x,y,z}); }
		else if (token == "LIGHT") { 
			stream >> m_constantBufferStruct.LightDir.x >> m_constantBufferStruct.LightDir.y >> m_constantBufferStruct.LightDir.z 
				   >> m_constantBufferStruct.LightColor.x >> m_constantBufferStruct.LightColor.y >> m_constantBufferStruct.LightColor.z;
		}
		else if (token == "END_SCENE") break;
	}
	return true;
}