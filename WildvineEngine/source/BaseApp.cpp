#include "BaseApp.h"
#include "ResourceManager.h"

HRESULT
BaseApp::awake() {
	m_sceneGraph.init();
	return S_OK;
}

int
BaseApp::run(HINSTANCE hInst, int nCmdShow) {
	if (FAILED(m_window.init(hInst, nCmdShow, WndProc, this))) return 0;
	if (FAILED(awake())) return 0;
	if (FAILED(init())) return 0;

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
		} else {
			LARGE_INTEGER curr;
			QueryPerformanceCounter(&curr);
			float dt = static_cast<float>(curr.QuadPart - prev.QuadPart) / 
			           freq.QuadPart;
			prev = curr;
			update(dt);
			render();
		}
	}
	return (int)msg.wParam;
}

HRESULT
BaseApp::init() {
	HRESULT hr = S_OK;
	hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window);
	hr = m_renderTargetView.init(m_device, m_backBuffer, 
	                             DXGI_FORMAT_R8G8B8A8_UNORM);
	hr = m_depthStencil.init(m_device, m_window.m_width, m_window.m_height, 
	                         DXGI_FORMAT_D24_UNORM_S8_UINT, 
	                         D3D11_BIND_DEPTH_STENCIL, 4, 16);
	hr = m_depthStencilView.init(m_device, m_depthStencil, 
	                             DXGI_FORMAT_D24_UNORM_S8_UINT);
	hr = m_viewport.init(m_window);
	m_d3dReady = true;

	std::array<std::string, 6> faces = {
		"Skybox/cubemap_0.png", "Skybox/cubemap_1.png", "Skybox/cubemap_2.png",
		"Skybox/cubemap_3.png", "Skybox/cubemap_4.png", "Skybox/cubemap_5.png"
	};
	m_skyboxTex.CreateCubemap(m_device, m_deviceContext, faces, false);

	m_ToadBodyBC.init(m_device, "Assets/Textures/rana/Sci-FIToad_Body_BC.png", PNG);
	m_ToadBodyN.init(m_device, "Assets/Textures/rana/Sci-FIToad_Body_N.png", PNG);
	m_ToadBodyM.init(m_device, "Assets/Textures/rana/Sci-FIToad_Body_M.png", PNG);
	m_ToadBodyR.init(m_device, "Assets/Textures/rana/Sci-FIToad_Body_R.png", PNG);
	m_ToadBodyAO.init(m_device, "Assets/Textures/rana/Sci-FIToad_Body_AO.png", PNG);
	m_ToadGlassBC.init(m_device, "Assets/Textures/rana/Sci-FIToad_Glass_BC.png", PNG);
	m_ToadGlassN.init(m_device, "Assets/Textures/rana/Sci-FIToad_Glass_N.png", PNG);
	m_ToadGlassR.init(m_device, "Assets/Textures/rana/Sci-FIToad_Glass_R.png", PNG);
	m_ToadGlassO.init(m_device, "Assets/Textures/rana/Sci-FIToad_Glass_O.png", PNG);
	m_ToadHeadBC.init(m_device, "Assets/Textures/rana/Sci-FIToad_Head_BC.png", PNG);
	m_ToadHeadN.init(m_device, "Assets/Textures/rana/Sci-FIToad_Head_N.png", PNG);
	m_ToadHeadR.init(m_device, "Assets/Textures/rana/Sci-FIToad_Head_R.png", PNG);

	m_cyberGun = EU::MakeShared<Actor>(m_device);
	if (!m_cyberGun.isNull()) {
		m_model = new Model3D("Assets/Models/Rana.fbx", ModelType::FBX);
		m_cyberGun->setMesh(m_device, m_model->GetMeshes());
		m_cyberGun->setName("Sci-Fi Toad");
		m_actors.push_back(m_cyberGun);
		m_cyberGun->getComponent<Transform>()->setTransform(
			EU::Vector3(0.0f, -1.90f, 10.5f), 
			EU::Vector3(0.0f, 3.14f, 0.0f), 
			EU::Vector3(1.0f, 1.0f, 1.0f));
	}
	for (auto& actor : m_actors) m_sceneGraph.addEntity(actor.get());

	LayoutBuilder b;
	b.Add("POSITION", DXGI_FORMAT_R32G32B32_FLOAT)
	 .Add("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT)
	 .Add("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
	 .Add("BITANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
	 .Add("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	m_shaderProgram.init(m_device, "PBRShader.hlsl", b);
	m_constantBuffer.init(m_device, sizeof(CBMain));

	m_camera.setLens(XM_PIDIV4, m_window.m_width / (float)m_window.m_height, 
	                 0.01f, 100.0f);
	m_camera.setPosition(0.0f, 3.0f, -6.0f);
	m_constantBufferStruct.LightColor = EU::Vector3(1.0f, 1.0f, 1.0f);
	m_constantBufferStruct.LightDir = EU::Vector3(-0.20f, -1.0f, 1.0f);

	m_skybox.init(m_device, &m_deviceContext, m_skyboxTex);
	m_defaultRasterizer.init(m_device, D3D11_FILL_SOLID, D3D11_CULL_BACK, 
	                         false, true);
	m_defaultDepthStencil.init(m_device, true, D3D11_DEPTH_WRITE_MASK_ALL, 
	                           D3D11_COMPARISON_LESS);
	m_defaultSampler.init(m_device);

	m_pbrMaterial.setShader(&m_shaderProgram);
	m_pbrMaterial.setRasterizerState(&m_defaultRasterizer);
	m_pbrMaterial.setDepthStencilState(&m_defaultDepthStencil);
	m_pbrMaterial.setSamplerState(&m_defaultSampler);

	m_transparentPbrMaterial.setShader(&m_shaderProgram);
	m_transparentPbrMaterial.setRasterizerState(&m_defaultRasterizer);
	m_transparentPbrMaterial.setDepthStencilState(&m_defaultDepthStencil);
	m_transparentPbrMaterial.setSamplerState(&m_defaultSampler);
	m_transparentPbrMaterial.setDomain(MaterialDomain::Transparent);
	m_transparentPbrMaterial.setBlendMode(BlendMode::Alpha);

	m_bodyMatInst.setMaterial(&m_pbrMaterial);
	m_bodyMatInst.setAlbedo(&m_ToadBodyBC);
	m_bodyMatInst.setNormal(&m_ToadBodyN);
	m_bodyMatInst.setMetallic(&m_ToadBodyM);
	m_bodyMatInst.setRoughness(&m_ToadBodyR);
	m_bodyMatInst.setAO(&m_ToadBodyAO);

	m_glassMatInst.setMaterial(&m_transparentPbrMaterial);
	m_glassMatInst.setAlbedo(&m_ToadGlassBC);
	m_glassMatInst.setNormal(&m_ToadGlassN);
	m_glassMatInst.setRoughness(&m_ToadGlassR);
	m_glassMatInst.getParams().baseColor = XMFLOAT4(1, 1, 1, 0.35f);

	m_headMatInst.setMaterial(&m_pbrMaterial);
	m_headMatInst.setAlbedo(&m_ToadHeadBC);
	m_headMatInst.setNormal(&m_ToadHeadN);
	m_headMatInst.setRoughness(&m_ToadHeadR);

	m_toadRenderMesh.destroy();
	for (const MeshComponent& mc : m_model->GetMeshes()) {
		Submesh s{};
		s.vertexBuffer.init(m_device, mc, D3D11_BIND_VERTEX_BUFFER);
		s.indexBuffer.init(m_device, mc, D3D11_BIND_INDEX_BUFFER);
		s.indexCount = mc.m_numIndex;
		std::string n = mc.m_name;
		if (n.find("Eyes") != std::string::npos || 
		    n.find("Head") != std::string::npos) s.materialSlot = 2;
		else if (n.find("Glass") != std::string::npos) s.materialSlot = 1;
		else s.materialSlot = 0;
		m_toadRenderMesh.getSubmeshes().push_back(std::move(s));
	}
	EU::TSharedPointer<MeshRendererComponent> mr = 
		m_cyberGun->getComponent<MeshRendererComponent>();
	if (!mr) {
		mr = EU::MakeShared<MeshRendererComponent>();
		m_cyberGun->addComponent(mr);
	}
	mr->setMesh(&m_toadRenderMesh);
	mr->setMaterialInstances({ &m_bodyMatInst, &m_glassMatInst, &m_headMatInst });

	m_directionalLightActor = EU::MakeShared<Actor>(m_device);
	if (!m_directionalLightActor.isNull()) {
		m_directionalLightActor->setName("Light Actor 1");
		EU::TSharedPointer<LightComponent> lc = 
			m_directionalLightActor->getComponent<LightComponent>();
		if (!lc) {
			lc = EU::MakeShared<LightComponent>();
			m_directionalLightActor->addComponent(lc);
		}
		lc->getLightData().type = LightType::Point;
		lc->getLightData().direction = m_constantBufferStruct.LightDir;
		lc->getLightData().color = m_constantBufferStruct.LightColor;
		m_actors.push_back(m_directionalLightActor);
		m_sceneGraph.addEntity(m_directionalLightActor.get());
	}
	m_editorViewportPass.init(m_device, 1280, 720);
	return S_OK;
}

void
BaseApp::update(float dt) {
	m_gui.update(m_viewport, m_window);
	m_gui.drawViewportPanel(m_editorViewportPass.getSRV());
	EU::TSharedPointer<Actor> sel;
	if (m_gui.selectedActorIndex >= 0 && 
	    m_gui.selectedActorIndex < (int)m_actors.size()) 
		sel = m_actors[m_gui.selectedActorIndex];

	m_gui.outliner(m_actors);
	m_gui.inspectorGeneral(sel);
	m_gui.editTransform(m_camera, m_window, sel);

	unsigned int dW = (unsigned int)m_gui.m_viewportSize.x;
	unsigned int dH = (unsigned int)m_gui.m_viewportSize.y;
	if (dW != m_lastRequestedViewportWidth || 
	    dH != m_lastRequestedViewportHeight) {
		m_lastRequestedViewportWidth = dW;
		m_lastRequestedViewportHeight = dH;
		m_viewportResizeStableFrames = 0;
	} else m_viewportResizeStableFrames++;

	if (m_viewportResizeStableFrames >= 2) {
		if (dW != m_editorViewportPass.getWidth() || 
		    dH != m_editorViewportPass.getHeight()) {
			m_editorViewportResizePending = true;
			m_pendingViewportWidth = (dW < 64) ? 64 : dW;
			m_pendingViewportHeight = (dH < 64) ? 64 : dH;
		}
	}
	m_camera.updateViewMatrix();
	XMStoreFloat4x4(&m_constantBufferStruct.View, 
	                XMMatrixTranspose(m_camera.getView()));
	XMStoreFloat4x4(&m_constantBufferStruct.Projection, 
	                XMMatrixTranspose(m_camera.getProj()));
	m_constantBufferStruct.CameraPos = m_camera.getPosition();

	m_gui.vec3Control("Light Direction", &m_constantBufferStruct.LightDir.x, 0.1f);
	m_gui.vec3Control("Light Color", &m_constantBufferStruct.LightColor.x, 0.1f);
	m_skybox.update(m_deviceContext, m_camera);
	m_constantBuffer.update(m_deviceContext, nullptr, 0, nullptr, 
	                        &m_constantBufferStruct, 0, 0);
	m_sceneGraph.update(dt, m_deviceContext);
}

void
BaseApp::render() {
	handleEditorViewportResize();
	const float cl[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	m_editorViewportPass.begin(m_deviceContext, cl);
	m_editorViewportPass.setViewport(m_deviceContext);
	m_editorViewportPass.clearDepth(m_deviceContext);
	m_skybox.render(m_deviceContext);
	m_defaultRasterizer.render(m_deviceContext);
	m_defaultDepthStencil.render(m_deviceContext, 0, false);
	m_shaderProgram.render(m_deviceContext);
	m_constantBuffer.render(m_deviceContext, 0, 1, true);
	m_sceneGraph.render(m_deviceContext);
	m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, cl);
	m_viewport.render(m_deviceContext);
	m_gui.render();
	m_swapChain.present();
}

void
BaseApp::destroy() {
	if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();
	m_sceneGraph.destroy();
	m_editorViewportPass.destroy();
	m_toadRenderMesh.destroy();
	m_ToadBodyBC.destroy(); m_ToadBodyN.destroy(); m_ToadBodyM.destroy();
	m_ToadBodyR.destroy(); m_ToadBodyAO.destroy(); m_ToadGlassBC.destroy();
	m_ToadGlassN.destroy(); m_ToadGlassR.destroy(); m_ToadGlassO.destroy();
	m_ToadHeadBC.destroy(); m_ToadHeadN.destroy(); m_ToadHeadR.destroy();
	m_defaultRasterizer.destroy(); m_defaultDepthStencil.destroy();
	m_defaultSampler.destroy(); m_shaderProgram.destroy();
	m_depthStencil.destroy(); m_depthStencilView.destroy();
	m_renderTargetView.destroy(); m_swapChain.destroy(); m_backBuffer.destroy();
	if (m_guiInitialized) m_gui.destroy();
	delete m_model; m_deviceContext.destroy(); m_device.destroy();
}

LRESULT
BaseApp::WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
	if (ImGui_ImplWin32_WndProcHandler(h, m, w, l)) return true;
	switch (m) {
		case WM_CREATE: {
			CREATESTRUCT* p = (CREATESTRUCT*)l;
			SetWindowLongPtr(h, GWLP_USERDATA, (LONG_PTR)p->lpCreateParams);
		} return 0;
		case WM_SIZE: {
			if (w == SIZE_MINIMIZED) return 0;
			BaseApp* a = (BaseApp*)GetWindowLongPtr(h, GWLP_USERDATA);
			if (a) a->onResize(LOWORD(l), HIWORD(l));
		} return 0;
		case WM_DESTROY: PostQuitMessage(0); return 0;
	}
	return DefWindowProc(h, m, w, l);
}

void
BaseApp::onResize(unsigned int nW, unsigned int nH) {
	if (!m_d3dReady || nW == 0 || nH == 0) return;
	m_window.m_width = nW; m_window.m_height = nH;
	ID3D11RenderTargetView* nR = nullptr;
	m_deviceContext.m_deviceContext->OMSetRenderTargets(1, &nR, nullptr);
	m_renderTargetView.destroy(); m_depthStencilView.destroy();
	m_depthStencil.destroy(); m_backBuffer.destroy();
	m_swapChain.resizeBuffers(nW, nH);
	m_swapChain.getBackBuffer(m_backBuffer);
	m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_depthStencil.init(m_device, nW, nH, DXGI_FORMAT_D24_UNORM_S8_UINT, 
	                    D3D11_BIND_DEPTH_STENCIL, 4, 16);
	m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
	m_viewport.init(m_window);
	m_camera.setLens(XM_PIDIV4, nW / (float)nH, 0.01f, 100.0f);
}

void
BaseApp::handleEditorViewportResize() {
	if (!m_editorViewportResizePending) return;
	m_deviceContext.m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	ID3D11ShaderResourceView* nS[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
	m_deviceContext.m_deviceContext->PSSetShaderResources(0, 
		D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nS);
	EditorViewportPass nP;
	if (SUCCEEDED(nP.init(m_device, m_pendingViewportWidth, 
	                      m_pendingViewportHeight))) 
		m_editorViewportPass.swap(nP);
	m_editorViewportResizePending = false;
}

bool
BaseApp::saveScene(const std::string& path) { return true; }

bool
BaseApp::loadScene(const std::string& path) { return true; }

std::string
BaseApp::getDefaultScenePath() const { return ""; }