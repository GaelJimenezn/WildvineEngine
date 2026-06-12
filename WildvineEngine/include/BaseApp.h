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
#include "GUI/GUI.h"
#include "SceneGraph\SceneGraph.h"
#include "EngineUtilities\Utilities\Camera.h"
#include "EngineUtilities\Utilities\Skybox.h"
#include "EngineUtilities\Utilities\LayoutBuilder.h"
#include "EngineUtilities/Utilities/EditorViewportPass.h"
#include "ECS/LightComponent.h"
#include "ECS/MeshRendererComponent.h"
#include "Rendering/Material.h"
#include "Rendering/MaterialInstance.h"
#include "Rendering/Mesh.h"
#include "Rendering/ForwardRenderer.h"
#include "Rendering/RenderScene.h"
#include <string>
/**
 * @brief Método ImGui_ImplWin32_WndProcHandler.
 *
 * @param hWnd Parámetro del método.
 * @param msg Parámetro del método.
 * @param wParam Parámetro del método.
 * @param lParam Parámetro del método.
 * @return Retorna el resultado de la operación.
 */
extern IMGUI_IMPL_API LRESULT
	ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/**
 * @class BaseApp
 * @brief Coordinates the engine application lifetime, editor UI, scene, and render
 * pipeline.
 *
 * BaseApp owns the Win32 window, D3D device/context wrappers, swap chain, editor
 * viewport,
 * GUI layer, scene graph, startup assets, and the active renderer.
 */
class
	BaseApp {
public:
	/** @brief Creates an empty application object. Call run() to start the engine loop. */
	BaseApp() = default;
	/** @brief Shuts down owned engine resources through destroy(). */
	~BaseApp() { destroy(); }

	/** @brief Performs early application setup before D3D-dependent initialization. */
	HRESULT
		awake();

	/** @brief Creates the window, initializes the engine, and enters the main loop. */
	int
		run(HINSTANCE hInst, int nCmdShow);

	/** @brief Initializes D3D resources, editor state, scene assets, and render passes. */
	HRESULT
		init();

	/** @brief Advances scene, camera, editor, and per-frame state. */
	void
		update(float deltaTime);

	/** @brief Renders the active scene, editor viewport, and GUI for one frame. */
	void
		render();

	/** @brief Releases GPU resources, editor state, scene assets, and window state. */
	void
		destroy();

	/** @brief Recreates size-dependent resources after a window resize. */
	void
		onResize(unsigned int newW, unsigned int newH);

	/** @brief Applies a pending editor viewport resize to off-screen render resources. */
	void
		handleEditorViewportResize();

	/** @brief Serializes the current scene to @p path. */
	bool
		saveScene(const std::string& path);
	/** @brief Loads scene data from @p path and rebuilds runtime objects. */
	bool
		loadScene(const std::string& path);
	/** @brief Returns the default scene path used by editor save/load commands. */
	std::string
		getDefaultScenePath() const;
private:
	/** @brief Static Win32 message procedure that forwards messages to BaseApp and ImGui. */
	static LRESULT CALLBACK
		WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


private:
	/** @brief Main Win32 window wrapper. */
	Window                              m_window;
	/** @brief D3D11 device facade used to create GPU resources. */
	Device															m_device;
	/** @brief D3D11 immediate-context facade used to issue rendering commands. */
	DeviceContext										m_deviceContext;
	/** @brief Swap chain used for back-buffer presentation. */
	SwapChain                           m_swapChain;
	/** @brief Back-buffer texture wrapper. */
	Texture                             m_backBuffer;
	/** @brief Render-target view for the back buffer. */
	RenderTargetView									  m_renderTargetView;
	/** @brief Depth-stencil texture paired with the back buffer. */
	Texture                             m_depthStencil;
	/** @brief Depth-stencil view paired with the back buffer. */
	DepthStencilView									  m_depthStencilView;
	/** @brief Main viewport used for swap-chain rendering. */
	Viewport                            m_viewport;
	/** @brief Legacy/default shader program retained by startup rendering paths. */
	ShaderProgram												m_shaderProgram;
	//Buffer															m_cbNeverChanges;
	//Buffer															m_cbChangeOnResize;
	/** @brief True once D3D resources have been created successfully. */
	bool m_d3dReady = false;
	/** @brief Constant buffer used by legacy/default shader paths. */
	Buffer m_constantBuffer;
	/** @brief CPU payload mirrored into @c m_constantBuffer. */
	CBMain m_constantBufferStruct;

	// Textures
	/** @brief Default albedo shader resource. */
	Texture m_AlbedoSRV;
	/** @brief Default metallic shader resource. */
	Texture m_MetallicSRV;
	/** @brief Default roughness shader resource. */
	Texture m_RoughnessSRV;
	/** @brief Default ambient-occlusion shader resource. */
	Texture m_AOSRV;
	/** @brief Default normal-map shader resource. */
	Texture m_NormalSRV;
	/** @brief Default emissive shader resource. */
	Texture m_EmissiveSRV;
	/** @brief Drakefire albedo shader resource. */
	Texture m_drakefireAlbedoSRV;
	/** @brief Drakefire normal shader resource. */
	Texture m_drakefireNormalSRV;
	/** @brief Drakefire metallic shader resource. */
	Texture m_drakefireMetallicSRV;
	/** @brief Drakefire roughness shader resource. */
	Texture m_drakefireRoughnessSRV;
	/** @brief Drakefire ambient-occlusion shader resource. */
	Texture m_drakefireAOSRV;

	/** @brief Editor/game camera used to view the scene. */
	Camera															m_camera;

	/** @brief Scene graph that owns hierarchy relationships for active actors. */
	SceneGraph												m_sceneGraph;
	/** @brief Active actor list used by editor and scene serialization. */
	std::vector<EU::TSharedPointer<Actor>> m_actors;
	/** @brief Startup cyber-gun actor reference. */
	EU::TSharedPointer<Actor> m_cyberGun;
	/** @brief Startup drakefire-pistol actor reference. */
	EU::TSharedPointer<Actor> m_drakefirePistol;
	/** @brief Startup directional-light actor reference. */
	EU::TSharedPointer<Actor> m_directionalLightActor;


	/** @brief Primary startup model resource pointer. */
	Model3D* m_model;
	/** @brief Drakefire model resource pointer. */
	Model3D* m_drakefireModel = nullptr;

	//CBChangeOnResize										cbChangesOnResize;
	//CBNeverChanges											cbNeverChanges;
	/** @brief ImGui/ImGuizmo editor UI layer. */
	GUI																m_gui;
	/** @brief True after the GUI layer has initialized its backends. */
	bool m_guiInitialized = false;
	/** @brief Cached camera position used by frame constants and UI. */
	EU::Vector3 m_cameraPos;

	/** @brief Skybox renderer and resources for background rendering. */
	Skybox m_skybox;
	/** @brief Cubemap texture used by @c m_skybox. */
	Texture															m_skyboxTex;
	/** @brief Default rasterizer state for scene materials. */
	RasterizerState m_defaultRasterizer;
	/** @brief Default depth-stencil state for scene materials. */
	DepthStencilState m_defaultDepthStencil;
	/** @brief Default sampler used by PBR material instances. */
	SamplerState m_defaultSampler;
	/** @brief Runtime mesh extracted from the cyber-gun model. */
	Mesh m_cyberGunRenderMesh;
	/** @brief Runtime mesh extracted from the drakefire model. */
	Mesh m_drakefireRenderMesh;
	/** @brief Opaque PBR material shared by default objects. */
	Material m_pbrMaterial;
	/** @brief Transparent PBR material variant for blended objects. */
	Material m_transparentPbrMaterial;
	/** @brief Material instance assigned to the cyber-gun render mesh. */
	MaterialInstance m_cyberGunMaterial;
	/** @brief Material instance assigned to the drakefire render mesh. */
	MaterialInstance m_drakefireMaterial;

	/** @brief Off-screen editor viewport render target. */
	EditorViewportPass m_editorViewportPass;
	/** @brief Forward renderer used by the current application pipeline. */
	ForwardRenderer m_forwardRenderer;
	/** @brief Per-frame render packet gathered from the scene graph. */
	RenderScene m_renderScene;
	/** @brief True when editor viewport resources must be resized. */
	bool m_editorViewportResizePending = false;
	/** @brief Requested editor viewport width for deferred resize. */
	unsigned int m_pendingViewportWidth = 1;
	/** @brief Requested editor viewport height for deferred resize. */
	unsigned int m_pendingViewportHeight = 1;

	/** @brief Last viewport width requested by the GUI. */
	unsigned int m_lastRequestedViewportWidth = 1;
	/** @brief Last viewport height requested by the GUI. */
	unsigned int m_lastRequestedViewportHeight = 1;
	/** @brief Number of stable frames used to debounce viewport resizing. */
	int m_viewportResizeStableFrames = 0;
};
