#include "BaseApp.h"
#include "ResourceManager.h"

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
		0);

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

	// Load Resources -> Modelos, Texturas e Interfaz de usuario
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
		// Crear vertex buffer y index buffer para el pistol
		std::vector<MeshComponent> cyberGunMeshes;
		m_model = new Model3D("CyberGun.fbx", ModelType::FBX);
		cyberGunMeshes = m_model->GetMeshes();

		std::vector<Texture> cyberGunTextures;
		hr = m_AlbedoSRV.init(m_device, "Textures/CyberGun/base.tga", PNG);
		if (FAILED(hr)) {
			ERROR("Main", "InitDevice",
				("Failed to initialize DrakePistol Texture. HRESULT: " + std::to_string(hr)).c_str());
			return hr;
		}
		hr = m_MetallicSRV.init(m_device, "Textures/CyberGun/metallic.tga", PNG);
		if (FAILED(hr)) {
			ERROR("Main", "InitDevice",
				("Failed to initialize DrakePistol Texture. HRESULT: " + std::to_string(hr)).c_str());
			return hr;
		}
		hr = m_RoughnessSRV.init(m_device, "Textures/CyberGun/roughness.tga", PNG);
		if (FAILED(hr)) {
			ERROR("Main", "InitDevice",
				("Failed to initialize DrakePistol Texture. HRESULT: " + std::to_string(hr)).c_str());
			return hr;
		}
		hr = m_AOSRV.init(m_device, "Textures/CyberGun/ao.tga", PNG);
		if (FAILED(hr)) {
			ERROR("Main", "InitDevice",
				("Failed to initialize DrakePistol Texture. HRESULT: " + std::to_string(hr)).c_str());
			return hr;
		}
		hr = m_NormalSRV.init(m_device, "Textures/CyberGun/normal.tga", PNG);
		if (FAILED(hr)) {
			ERROR("Main", "InitDevice",
				("Failed to initialize DrakePistol Texture. HRESULT: " + std::to_string(hr)).c_str());
			return hr;
		}
		cyberGunTextures.push_back(m_AlbedoSRV);
		cyberGunTextures.push_back(m_NormalSRV);
		cyberGunTextures.push_back(m_MetallicSRV);
		cyberGunTextures.push_back(m_RoughnessSRV);
		cyberGunTextures.push_back(m_AOSRV);

		m_cyberGun->setMesh(m_device, cyberGunMeshes);
		m_cyberGun->setTextures(cyberGunTextures);
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
		ERROR("Main", "InitDevice",
			("Failed to initialize ShaderProgram. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	// Create the constant buffers
	hr = m_constantBuffer.init(m_device, sizeof(CBMain));
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize m_constantBuffer Buffer. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}
	
	m_camera.setLens(XM_PIDIV4, m_window.m_width / (float)m_window.m_height, 0.01f, 100.0f);
	m_camera.setPosition(0.0f, 3.0f, -6.0f);

	m_constantBufferStruct.LightColor = EU::Vector3(1.0f, 1.0f, 1.0f);
	m_constantBufferStruct.LightDir = EU::Vector3(-0.20f, -1.0f, 1.0f);

	// Initialize the Skybox pass -> Carga de textura + creación de buffers/ shaders específicos para el skybox
	m_skybox.init(m_device, &m_deviceContext, m_skyboxTex);

	// Initialize default states (Rasterizer, DepthStencil)
	hr = m_defaultRasterizer.init(m_device, D3D11_FILL_SOLID, D3D11_CULL_BACK, false, true);
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize default Rasterizer. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}
	hr = m_defaultDepthStencil.init(m_device, true, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS);
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize default DepthStencilState. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	hr = m_editorViewportPass.init(m_device, 1280, 720);
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize EditorViewportPass. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	return S_OK;
}

void 
BaseApp::update(float deltaTime) {
	// Update our time
	static float t = 0.0f;
	if (m_swapChain.m_driverType == D3D_DRIVER_TYPE_REFERENCE)
	{
		t += (float)XM_PI * 0.0125f;
	}
	else
	{
		static DWORD dwTimeStart = 0;
		DWORD dwTimeCur = GetTickCount();
		if (dwTimeStart == 0)
			dwTimeStart = dwTimeCur;
		t = (dwTimeCur - dwTimeStart) / 1000.0f;
	}
	// Update User Interface
	m_gui.update(m_viewport, m_window);
	bool show_demo_window = true;
	//ImGui::ShowDemoWindow(&show_demo_window);
	m_gui.drawViewportPanel(m_editorViewportPass.getSRV());
	m_gui.inspectorGeneral(m_actors[m_gui.selectedActorIndex]);
	m_gui.outliner(m_actors);
	m_gui.editTransform(m_camera, m_window, m_actors[m_gui.selectedActorIndex]);

	unsigned int desiredW = static_cast<unsigned int>(m_gui.m_viewportSize.x);
	unsigned int desiredH = static_cast<unsigned int>(m_gui.m_viewportSize.y);

	const unsigned int kMinViewportSize = 64;

	if (desiredW < kMinViewportSize) desiredW = kMinViewportSize;
	if (desiredH < kMinViewportSize) desiredH = kMinViewportSize;

	// Si cambió el tamaño solicitado, reinicia estabilidad
	if (desiredW != m_lastRequestedViewportWidth || desiredH != m_lastRequestedViewportHeight)
	{
		m_lastRequestedViewportWidth = desiredW;
		m_lastRequestedViewportHeight = desiredH;
		m_viewportResizeStableFrames = 0;
	}
	else
	{
		// El tamaño ya no cambió este frame
		m_viewportResizeStableFrames++;
	}

	// Solo marcar resize cuando el tamaño se haya mantenido estable
	const int kStableFramesRequired = 2;

	if (m_viewportResizeStableFrames >= kStableFramesRequired)
	{
		if (desiredW != m_editorViewportPass.getWidth() ||
			desiredH != m_editorViewportPass.getHeight())
		{
			m_editorViewportResizePending = true;
			m_pendingViewportWidth = desiredW;
			m_pendingViewportHeight = desiredH;
		}
	}

	// Actualizar la matriz de proyección y vista
	m_camera.updateViewMatrix();

	XMStoreFloat4x4(&m_constantBufferStruct.View, XMMatrixTranspose(m_camera.getView()));
	XMStoreFloat4x4(&m_constantBufferStruct.Projection, XMMatrixTranspose(m_camera.getProj()));
	m_constantBufferStruct.CameraPos = m_camera.getPosition();
	
	// Luz blanca fuerte
	m_gui.vec3Control("Light Direction", &m_constantBufferStruct.LightDir.x, 0.1f);
	m_gui.vec3Control("Light Color", &m_constantBufferStruct.LightColor.x, 0.1f);

	// Update Skybox Pass -> Solo necesita la vista sin traslación + proyección para funcionar correctamente (ver método update de Skybox)
	m_skybox.update(m_deviceContext, m_camera);

	// Update constant buffer for Scene Pass
	m_constantBuffer.update(m_deviceContext, nullptr, 0, nullptr, &m_constantBufferStruct, 0, 0);

	// Update Actors
	m_sceneGraph.update(deltaTime, m_deviceContext);

}

void 
BaseApp::render() {
	handleEditorViewportResize();

	float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	const float viewportClear[4] = { 0.10f, 0.10f, 0.10f, 1.0f };
	m_editorViewportPass.begin(m_deviceContext, viewportClear);
	m_editorViewportPass.setViewport(m_deviceContext);
	m_editorViewportPass.clearDepth(m_deviceContext);

	// 1) SKYBOX PASS
	m_skybox.render(m_deviceContext);

	// 2) RESTAURAR ESTADOS + PIPELINE DE ESCENA
	m_defaultRasterizer.render(m_deviceContext);
	m_defaultDepthStencil.render(m_deviceContext, 0, false);

	// limpia SRVs por seguridad
	//ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	//m_deviceContext.m_deviceContext->PSSetShaderResources(10, 1, nullSRV);
	//m_deviceContext.m_deviceContext->PSSetShaderResources(0, 1, nullSRV);

	// Re-bindea shader/layout de escena
	m_shaderProgram.render(m_deviceContext);

	// CBs para VS (view/proj)
	m_constantBuffer.render(m_deviceContext, 0, 1, true);

	// 3) SCENE PASS
	m_sceneGraph.render(m_deviceContext);

	// 2) Volver al backbuffer principal
	m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);
	m_viewport.render(m_deviceContext);
	m_depthStencilView.render(m_deviceContext);

	// 4) GUI
	m_gui.render();

	m_swapChain.present();
}

void
BaseApp::destroy() {
	if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();
	m_sceneGraph.destroy();
	m_editorViewportPass.destroy();
	m_AlbedoSRV.destroy();
	m_MetallicSRV.destroy();
	m_NormalSRV.destroy();
	m_RoughnessSRV.destroy();
	m_AOSRV.destroy();
	m_defaultRasterizer.destroy();
	m_defaultDepthStencil.destroy();
	//m_cbNeverChanges.destroy();
	//m_cbChangeOnResize.destroy();
	m_shaderProgram.destroy();
	m_depthStencil.destroy();
	m_depthStencilView.destroy();
	m_renderTargetView.destroy();
	m_swapChain.destroy();
	m_backBuffer.destroy();
	m_gui.destroy();
	m_deviceContext.destroy();
	m_device.destroy();
}

LRESULT
BaseApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam)) {
		return true;
	}

	switch (message) {
	case WM_CREATE:	{
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
		// Evita recrear cuando está minimizada
		if (wParam == SIZE_MINIMIZED) return 0;

		unsigned int newW = LOWORD(lParam);
		unsigned int newH = HIWORD(lParam);
		if (newW == 0 || newH == 0) return 0;

		// Recupera tu instancia BaseApp (lo más común es guardarla en GWLP_USERDATA en WM_CREATE)
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

void BaseApp::onResize(unsigned int newW, unsigned int newH)
{
	// 1) Actualiza window size (tu init lo calcula con GetClientRect solo una vez) :contentReference[oaicite:6]{index=6}
	if (!m_d3dReady) {
		// Aun así puedes actualizar el tamaño lógico de la ventana
		m_window.m_width = (int)newW;
		m_window.m_height = (int)newH;
		return;
	}

	if (!m_deviceContext.m_deviceContext || !m_swapChain.m_swapChain) return;
	if (newW == 0 || newH == 0) return;

	m_window.m_width = (int)newW;
	m_window.m_height = (int)newH;
	// 2) Desbindea targets actuales (clave antes de destruir)
	ID3D11RenderTargetView* nullRTV = nullptr;
	m_deviceContext.m_deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);

	// 3) Libera recursos dependientes del tamaño (RTV/DSV/Depth/BackBuffer)
	m_renderTargetView.destroy();
	m_depthStencilView.destroy();
	m_depthStencil.destroy();
	m_backBuffer.destroy();

	// 4) Resize swapchain
	HRESULT hr = m_swapChain.resizeBuffers(newW, newH);
	if (FAILED(hr)) return;

	// 5) Re-obtén backbuffer
	hr = m_swapChain.getBackBuffer(m_backBuffer);
	if (FAILED(hr)) return;

	// 6) Re-crea RTV
	hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
	if (FAILED(hr)) return;

	// 7) Re-crea Depth/DSV (tu init actual lo hace con m_window.m_width/m_height)
	hr = m_depthStencil.init(m_device, newW, newH, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, 4, 0);
	if (FAILED(hr)) return;

	hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
	if (FAILED(hr)) return;

	// 8) Viewport
	m_viewport.init(m_window);

	// 9) Cámara (aspect ratio) (tu cámara lo calcula a partir de m_window) 
	m_camera.setLens(XM_PIDIV4, newW / (float)newH, 0.01f, 100.0f);
}

void BaseApp::handleEditorViewportResize()
{
	if (!m_editorViewportResizePending)
		return;

	// Desbindear antes de tocar recursos
	m_deviceContext.m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);

	ID3D11ShaderResourceView* nullSRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
	m_deviceContext.m_deviceContext->PSSetShaderResources(
		0,
		D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT,
		nullSRVs
	);

	// Crear pass temporal nuevo
	EditorViewportPass newPass;
	HRESULT hr = newPass.init(m_device, m_pendingViewportWidth, m_pendingViewportHeight);
	if (FAILED(hr))
	{
		// Si falla, conserva el pass actual
		m_editorViewportResizePending = false;
		return;
	}

	// Intercambio seguro: el pass viejo queda en newPass y se destruye al salir
	m_editorViewportPass.swap(newPass);

	m_editorViewportResizePending = false;
}