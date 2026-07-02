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
#include "GUI.h"
#include "ECS/Actor.h"

#include "SceneGraph/SceneGraph.h"

#include "EngineUtilities/Utilities/Camera.h"
#include "EngineUtilities/Utilities/Skybox.h"
#include "EngineUtilities/Utilities/LayoutBuilder.h"
#include "EngineUtilities/Utilities/EditorViewportPass.h"

#include "ECS/LightComponent.h"
#include "ECS/MeshRendererComponent.h"

#include "Rendering/Material.h"
#include "Rendering/MaterialInstance.h"
#include "Rendering/Mesh.h"
#include "Rendering/RenderPipeline.h"
#include "Rendering/RenderScene.h"

#include "CommandManager.h"

#include <string>

extern IMGUI_IMPL_API
LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

struct ActorClipboard {
  std::string name;
  EU::Vector3 position;
  EU::Vector3 rotation;
  EU::Vector3 scale;
};

struct LoadedModel {
  Mesh mesh;
  Material material;
  MaterialInstance materialInstance;
  Texture albedo, normal, metallic, roughness, ao;
  EU::Vector3 localMin;
  EU::Vector3 localMax;
};

struct GizmoEditState {
  EU::Vector3 position;
  EU::Vector3 rotation;
  EU::Vector3 scale;
};

class BaseApp {
public:
  BaseApp() = default;
  ~BaseApp() { destroy(); }

  HRESULT awake();
  int run(HINSTANCE hInst, int nCmdShow);
  HRESULT init();
  void update(float deltaTime);
  void render();
  void destroy();

  void onResize(unsigned int newW, unsigned int newH);
  void handleEditorViewportResize();

  bool saveScene(const std::string& path);
  bool loadScene(const std::string& path);
  std::string getDefaultScenePath() const;

  void addActorToScene(const EU::TSharedPointer<Actor>& actor);
  void removeActorFromScene(const EU::TSharedPointer<Actor>& actor);

private:
  EU::TSharedPointer<Actor> createLightActor(LightType type, const std::string& baseName);
  void enforceDefaultSceneLayout();
  void frameDefaultSceneCamera();
  void selectActor(EU::TSharedPointer<Actor> actor);
  void syncLightActors();
  HRESULT loadPbrTexture(Texture& texture, const std::string& path, const char* debugName);

  static LRESULT CALLBACK
    WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
  Window              m_window;
  Device              m_device;
  DeviceContext       m_deviceContext;
  SwapChain           m_swapChain;
  Texture             m_backBuffer;
  RenderTargetView    m_renderTargetView;
  Texture             m_depthStencil;
  DepthStencilView    m_depthStencilView;
  Viewport            m_viewport;
  ShaderProgram       m_shaderProgram;

  bool                m_d3dReady = false;
  bool                m_guiInitialized = false;

  Buffer              m_constantBuffer;
  CBMain              m_constantBufferStruct;

  // --- Estado inicial para el boton Reset ---
  struct InitialTransform {
    EU::Vector3 position;
    EU::Vector3 rotation;
    EU::Vector3 scale;
  };
  bool m_initialStateCaptured = false;
  EU::Vector3 m_initialLightDir;
  EU::Vector3 m_initialLightColor;
  EU::Vector3 m_initialCameraPos;
  std::vector<InitialTransform> m_initialTransforms;
  void captureInitialState();
  void resetSceneToDefaults();
  void focusCameraOnActor(const EU::TSharedPointer<Actor>& actor);
  void fitCameraToScene();

  unsigned int m_lastDrawCalls = 0;

  // Picking
  EU::Vector3 m_modelLocalMin;
  EU::Vector3 m_modelLocalMax;
  void pickActorFromMouse();

  // Undo/Redo
  CommandManager m_commands;
  bool m_prevGizmoUsing = false;
  bool m_gizmoEditing = false;
  int  m_gizmoEditActorIndex = -1;
  GizmoEditState m_gizmoBefore;
  bool captureGizmoState(int index, GizmoEditState& out);

  // Clipboard
  ActorClipboard m_clipboard;
  bool m_hasClipboard = false;

  EU::TSharedPointer<Actor> spawnPistol(const std::string& name,
    const EU::Vector3& pos, const EU::Vector3& rot, const EU::Vector3& scale);
  void duplicateSelected();
  void deleteSelected();
  void copySelected();
  void pasteClipboard();
  void savePrefabSelected();
  void loadPrefab();

  // Content Browser / Modelos dinamicos
  std::vector<std::unique_ptr<LoadedModel>> m_loadedModels;
  std::vector<Texture> m_thumbTextures;
  std::vector<AssetThumb> m_thumbnails;

  EU::TSharedPointer<Actor> loadModelActor(const std::string& modelPath);
  void loadModelTextures(LoadedModel& lm, const std::string& folder);
  void buildTextureThumbnails();

  bool getActorAABB(const EU::TSharedPointer<Actor>& actor, EU::Vector3& outMin, EU::Vector3& outMax);

  // Texturas del modelo principal
  Texture             m_AlbedoSRV;
  Texture             m_MetallicSRV;
  Texture             m_RoughnessSRV;
  Texture             m_AOSRV;
  Texture             m_NormalSRV;
  Texture             m_EmissiveSRV;

  // Recursos generales
  Texture             m_skyboxTex;

  Camera              m_camera;
  SceneGraph          m_sceneGraph;

  // Actores
  std::vector<EU::TSharedPointer<Actor>> m_actors;
  EU::TSharedPointer<Actor> m_cyberGun;
  EU::TSharedPointer<Actor> m_directionalLightActor;

  // Recursos de modelo
  Model3D* m_model = nullptr;

  // GUI
  GUI                 m_gui;

  // Render base
  Skybox              m_skybox;
  RasterizerState     m_defaultRasterizer;
  DepthStencilState   m_defaultDepthStencil;
  SamplerState        m_defaultSampler;

  // Render meshes y materiales
  Mesh                m_cyberGunRenderMesh;
  Material            m_pbrMaterial;
  Material            m_transparentPbrMaterial;
  MaterialInstance    m_cyberGunMaterial;

  EditorViewportPass  m_editorViewportPass;
  RenderPipeline      m_renderPipeline;
  RenderScene         m_renderScene;

  // Resize diferido del viewport editor
  bool                m_editorViewportResizePending = false;
  unsigned int        m_pendingViewportWidth = 1;
  unsigned int        m_pendingViewportHeight = 1;

  unsigned int        m_lastRequestedViewportWidth = 1;
  unsigned int        m_lastRequestedViewportHeight = 1;
  int                 m_viewportResizeStableFrames = 0;
};
