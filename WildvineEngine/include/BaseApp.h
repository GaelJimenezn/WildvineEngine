#pragma once
#include "Prerequisites.h"
#include "Window.h"
#include "Device.h"
#include "DeviceContext.h"
#include "SwapChain.h"
#include "Texture.h"
#include "RenderTargetView.h"
#include "DepthStencilView.h"
#include "Viewport.h"
#include "ShaderProgram.h"
#include "MeshComponent.h"
#include "Buffer.h"
#include "SamplerState.h"
#include "Model3D.h"
#include "ECS/Actor.h"
#include "EngineUtilities\GUI/GUI.h"
#include "SceneGraph\SceneGraph.h"
#include "EngineUtilities\Utilities\Camera.h"
#include "EngineUtilities\Utilities\Skybox.h"
#include "EngineUtilities\Utilities\LayoutBuilder.h"
#include "EngineUtilities/Utilities/EditorViewportPass.h"
extern IMGUI_IMPL_API
LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class 
BaseApp {
public:
	BaseApp() = default;
	~BaseApp() { destroy(); }

	HRESULT
	awake();

	int 
	run(HINSTANCE hInst, int nCmdShow);
	
	HRESULT
	init();

	void 
	update(float deltaTime);

	void 
	render();

	void 
	destroy();

	void 
	onResize(unsigned int newW, unsigned int newH);

	void handleEditorViewportResize();
private:
	static LRESULT CALLBACK 
	WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


private:
	Window                              m_window;
	Device															m_device;
	DeviceContext												m_deviceContext;
	SwapChain                           m_swapChain;
	Texture                             m_backBuffer;
	RenderTargetView									  m_renderTargetView;
	Texture                             m_depthStencil;
	DepthStencilView									  m_depthStencilView;
	Viewport                            m_viewport;
	ShaderProgram												m_shaderProgram;
	//Buffer															m_cbNeverChanges;
	//Buffer															m_cbChangeOnResize;
	bool m_d3dReady = false;
	Buffer m_constantBuffer;
	CBMain m_constantBufferStruct;

	// Textures
	Texture m_AlbedoSRV;
	Texture m_MetallicSRV;
	Texture m_RoughnessSRV;
	Texture m_AOSRV;
	Texture m_NormalSRV;

	Camera															m_camera;

	SceneGraph													m_sceneGraph;
	std::vector<EU::TSharedPointer<Actor>> m_actors;
	EU::TSharedPointer<Actor> m_cyberGun;

	
	Model3D*														m_model;

	//CBChangeOnResize										cbChangesOnResize;
	//CBNeverChanges											cbNeverChanges;
	GUI																m_gui;
	EU::Vector3 m_cameraPos;

	Skybox m_skybox;
	Texture															m_skyboxTex;
	RasterizerState m_defaultRasterizer;
	DepthStencilState m_defaultDepthStencil;

	EditorViewportPass m_editorViewportPass;
	bool m_editorViewportResizePending = false;
	unsigned int m_pendingViewportWidth = 1;
	unsigned int m_pendingViewportHeight = 1;

	unsigned int m_lastRequestedViewportWidth = 1;
	unsigned int m_lastRequestedViewportHeight = 1;
	int m_viewportResizeStableFrames = 0;
};