#include "BaseApp.h"
#include "ResourceManager.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>

// Necesario para que Win32 reenvíe los inputs a ImGui
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

BaseApp::BaseApp(HINSTANCE hInst, int nCmdShow) {
  // Constructor vacío
}

HRESULT
BaseApp::awake() {
  HRESULT hr = S_OK;

  m_sceneGraph.init();

  MESSAGE("Main", "Awake", "Application awake successfully.");
  return hr;
}

EU::TSharedPointer<Actor>
BaseApp::createLightActor(LightType type, const std::string& baseName)
{
  if (!m_directionalLightActor.isNull()) {
    return m_directionalLightActor;
  }

  EU::TSharedPointer<Actor> lightActor = EU::MakeShared<Actor>(m_device);
  if (lightActor.isNull()) {
    return lightActor;
  }

  type = LightType::Directional;
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
  light.position = EU::Vector3(-4.0f, 6.0f, -4.0f);
  light.range = 0.0f;
  light.spotAngle = 0.0f;
  lightComponent->setCastShadow(true);

  m_actors.push_back(lightActor);
  m_sceneGraph.addEntity(lightActor.get());

  EU::TSharedPointer<Transform> transform = lightActor->getComponent<Transform>();
  if (transform) {
    transform->setTransform(
      light.position,
      EU::Vector3(0.0f, 0.0f, 0.0f),
      EU::Vector3(1.0f, 1.0f, 1.0f)
    );
    transform->rebuildMatrixFromVectors();
  }

  m_directionalLightActor = lightActor;

  selectActor(lightActor);

  return lightActor;
}

void
BaseApp::enforceDefaultSceneLayout()
{
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
    if (light.intensity <= 0.0f) {
      light.intensity = 3.0f;
    }
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
      if (actor) {
        m_sceneGraph.removeEntity(actor);
      }
      m_actors.erase(m_actors.begin() + i);
    }
  }

  if (m_gui.selectedActorIndex >= static_cast<int>(m_actors.size())) {
    m_gui.selectedActorIndex = m_actors.empty() ? -1 : 0;
  }
}

void
BaseApp::frameDefaultSceneCamera()
{
  if (m_model == nullptr || m_cyberGun.isNull()) {
    m_camera.lookAt(EU::Vector3(0.0f, 3.0f, -12.0f), EU::Vector3(0.0f, 0.0f, 0.0f));
    return;
  }

  EU::Vector3 minBounds(
    (std::numeric_limits<float>::max)(),
    (std::numeric_limits<float>::max)(),
    (std::numeric_limits<float>::max)()
  );
  EU::Vector3 maxBounds(
    -(std::numeric_limits<float>::max)(),
    -(std::numeric_limits<float>::max)(),
    -(std::numeric_limits<float>::max)()
  );

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
    (minBounds.z + maxBounds.z) * 0.5f
  );

  EU::Vector3 extents(
    (maxBounds.x - minBounds.x) * 0.5f,
    (maxBounds.y - minBounds.y) * 0.5f,
    (maxBounds.z - minBounds.z) * 0.5f
  );

  EU::TSharedPointer<Transform> transform = m_cyberGun->getComponent<Transform>();
  if (transform) {
    const EU::Vector3& position = transform->getPosition();
    const EU::Vector3& scale = transform->getScale();
    center = EU::Vector3(
      position.x + center.x * scale.x,
      position.y + center.y * scale.y,
      position.z + center.z * scale.z
    );
    extents = EU::Vector3(
      std::abs(extents.x * scale.x),
      std::abs(extents.y * scale.y),
      std::abs(extents.z * scale.z)
    );
  }

  const float computedRadius = std::sqrt(
    extents.x * extents.x +
    extents.y * extents.y +
    extents.z * extents.z
  );
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
BaseApp::selectActor(EU::TSharedPointer<Actor> actor)
{
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
BaseApp::syncLightActors()
{
  EU::TSharedPointer<Actor> firstDirectional;

  for (auto& actor : m_actors) {
    if (actor.isNull()) {
      continue;
    }

    EU::TSharedPointer<LightComponent> lightComponent = actor->getComponent<LightComponent>();
    if (!lightComponent) {
      continue;
    }

    LightData& light = lightComponent->getLightData();
    EU::TSharedPointer<Transform> transform = actor->getComponent<Transform>();
    if (transform) {
      light.position = transform->getPosition();
    }

    if (light.type == LightType::Directional && firstDirectional.isNull()) {
      firstDirectional = actor;
      m_constantBufferStruct.LightDir = light.direction;
      m_constantBufferStruct.LightColor = light.color * light.intensity;
    }
  }

  m_directionalLightActor = firstDirectional;
}

HRESULT
BaseApp::loadPbrTexture(Texture& texture, const std::string& path, const char* debugName)
{
  HRESULT hr = texture.init(m_device, path, ExtensionType::PNG);
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

  // 1. SwapChain
  hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize SwapChain. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // 2. Render Target View
  hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize RenderTargetView. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // 3. Depth Stencil Texture
  hr = m_depthStencil.init(
    m_device,
    m_window.m_width,
    m_window.m_height,
    DXGI_FORMAT_D24_UNORM_S8_UINT,
    D3D11_BIND_DEPTH_STENCIL,
    4,
    16
  );
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize DepthStencil. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // 4. Depth Stencil View
  hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize DepthStencilView. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // 5. Viewport
  hr = m_viewport.init(m_window);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize Viewport. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  m_d3dReady = true;

  // --------------------------------------------------------------------------
  // SKYBOX
  // --------------------------------------------------------------------------
  std::array<std::string, 6> faces = {
    "Skybox/cubemap_0.png",
    "Skybox/cubemap_1.png",
    "Skybox/cubemap_2.png",
    "Skybox/cubemap_3.png",
    "Skybox/cubemap_4.png",
    "Skybox/cubemap_5.png"
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
      if (FAILED(hr)) {
        return hr;
      }
    }

    m_cyberGun->setName("SM_CyberGun");
    m_actors.push_back(m_cyberGun);

    EU::TSharedPointer<Transform> transform = m_cyberGun->getComponent<Transform>();
    if (transform) {
      transform->setTransform(
        EU::Vector3(0.0f, 0.0f, 0.0f),
        EU::Vector3(0.0f, 0.0f, 0.0f),
        EU::Vector3(1.0f, 1.0f, 1.0f)
      );
      transform->rebuildMatrixFromVectors();
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
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize ShaderProgram. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  hr = m_constantBuffer.init(m_device, sizeof(CBMain));
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize m_constantBuffer Buffer. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // --------------------------------------------------------------------------
  // CAMERA / LIGHT
  // --------------------------------------------------------------------------
  m_camera.setLens(XM_PIDIV4, m_window.m_width / (float)m_window.m_height, 0.01f, 100.0f);
  m_camera.setPosition(0.0f, 3.0f, -6.0f);

  m_constantBufferStruct.LightColor = EU::Vector3(1.0f, 1.0f, 1.0f);
  m_constantBufferStruct.LightDir = EU::Vector3(-0.20f, -1.0f, 1.0f);

  // --------------------------------------------------------------------------
  // SKYBOX
  // --------------------------------------------------------------------------
  m_skybox.init(m_device, &m_deviceContext, m_skyboxTex);

  // --------------------------------------------------------------------------
  // ESTADOS POR DEFECTO
  // --------------------------------------------------------------------------
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

  hr = m_defaultSampler.init(m_device);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize default SamplerState. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // --------------------------------------------------------------------------
  // MATERIALES
  // --------------------------------------------------------------------------
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

  m_cyberGunMaterial.getParams().baseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
  m_cyberGunMaterial.getParams().metallic = 1.0f;
  m_cyberGunMaterial.getParams().roughness = 1.0f;
  m_cyberGunMaterial.getParams().ao = 1.0f;
  m_cyberGunMaterial.getParams().normalScale = 1.0f;
  m_cyberGunMaterial.getParams().alphaCutoff = 0.5f;

  // --------------------------------------------------------------------------
  // MESH RENDERIZABLE
  // --------------------------------------------------------------------------
  m_cyberGunRenderMesh.destroy();

  for (const MeshComponent& meshComponent : m_model->GetMeshes()) {
    Submesh submesh{};

    hr = submesh.vertexBuffer.init(m_device, meshComponent, D3D11_BIND_VERTEX_BUFFER);
    if (FAILED(hr)) {
      ERROR("Main", "InitDevice",
        ("Failed to initialize CyberGun vertex buffer. HRESULT: " + std::to_string(hr)).c_str());
      return hr;
    }

    hr = submesh.indexBuffer.init(m_device, meshComponent, D3D11_BIND_INDEX_BUFFER);
    if (FAILED(hr)) {
      ERROR("Main", "InitDevice",
        ("Failed to initialize CyberGun index buffer. HRESULT: " + std::to_string(hr)).c_str());
      return hr;
    }

    submesh.indexCount = meshComponent.m_numIndex;
    submesh.materialSlot = 0;

    m_cyberGunRenderMesh.getSubmeshes().push_back(std::move(submesh));
  }

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
  // VIEWPORT PASS / FORWARD RENDERER
  // --------------------------------------------------------------------------
  hr = m_editorViewportPass.init(m_device, 1280, 720);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize EditorViewportPass. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  hr = m_forwardRenderer.init(m_device);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize ForwardRenderer. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  return S_OK;
}

void
BaseApp::update(float deltaTime) {
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

  m_gui.update(m_viewport, m_window);
  m_gui.drawViewportPanel(m_editorViewportPass.getSRV());

  if (m_gui.shouldShowRenderDebug()) {
    m_gui.drawRenderDebugPanel(
      m_forwardRenderer.getPreShadowSRV(),
      m_editorViewportPass.getSRV(),
      m_forwardRenderer.getShadowMapSRV()
    );
  }

  m_gui.drawMaterialSRVDebugPanel(
    m_AlbedoSRV.m_textureFromImg,
    m_NormalSRV.m_textureFromImg,
    m_MetallicSRV.m_textureFromImg,
    m_RoughnessSRV.m_textureFromImg,
    m_AOSRV.m_textureFromImg
  );

  if (m_gui.shouldShowGBufferDebug()) {
    m_gui.drawGBufferDebugPanel(
      m_AlbedoSRV.m_textureFromImg,
      m_NormalSRV.m_textureFromImg,
      m_AOSRV.m_textureFromImg,
      m_RoughnessSRV.m_textureFromImg
    );
  }

  if (m_gui.shouldShowOutliner()) {
    m_gui.outliner(m_actors);
  }

  EU::TSharedPointer<Actor> selectedActor;
  if (m_gui.selectedActorIndex >= 0 &&
    m_gui.selectedActorIndex < static_cast<int>(m_actors.size())) {
    selectedActor = m_actors[m_gui.selectedActorIndex];
  }

  if (m_gui.shouldShowInspector()) {
    m_gui.inspectorGeneral(selectedActor);
  }

  if (m_gui.shouldShowToolbox()) {
    m_gui.drawToolboxPanel();
  }

  m_gui.editTransform(m_camera, m_window, selectedActor);

  if (m_gui.consumeSaveSceneRequest()) {
    saveScene(getDefaultScenePath());
  }

  LightType requestedLightType = LightType::Directional;
  if (m_gui.consumeCreateLightRequest(requestedLightType)) {
    selectActor(m_directionalLightActor);
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

  m_camera.updateViewMatrix();

  XMStoreFloat4x4(&m_constantBufferStruct.View, XMMatrixTranspose(m_camera.getView()));
  XMStoreFloat4x4(&m_constantBufferStruct.Projection, XMMatrixTranspose(m_camera.getProj()));
  m_constantBufferStruct.CameraPos = m_camera.getPosition();

  syncLightActors();

  m_skybox.update(m_deviceContext, m_camera);

  m_sceneGraph.update(deltaTime, m_deviceContext);
}

void
BaseApp::render() {
  handleEditorViewportResize();

  float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

  m_renderScene.clear();
  m_sceneGraph.gatherRenderScene(m_renderScene, m_camera);
  m_renderScene.skybox = &m_skybox;

  m_forwardRenderer.render(
    m_deviceContext,
    m_camera,
    m_renderScene,
    m_editorViewportPass
  );

  m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);
  m_viewport.render(m_deviceContext);
  m_depthStencilView.render(m_deviceContext);

  m_gui.render();

  m_swapChain.present();
}

void
BaseApp::destroy() {
  if (m_deviceContext.m_deviceContext)
    m_deviceContext.m_deviceContext->ClearState();

  m_sceneGraph.destroy();
  m_editorViewportPass.destroy();
  m_forwardRenderer.destroy();
  m_cyberGunRenderMesh.destroy();

  m_AlbedoSRV.destroy();
  m_MetallicSRV.destroy();
  m_NormalSRV.destroy();
  m_RoughnessSRV.destroy();
  m_AOSRV.destroy();

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

LRESULT
BaseApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam)) {
    return true;
  }

  switch (message) {
  case WM_CREATE:
  {
    CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCreate->lpCreateParams);
  }
  return 0;

  case WM_PAINT:
  {
    PAINTSTRUCT ps;
    BeginPaint(hWnd, &ps);
    EndPaint(hWnd, &ps);
  }
  return 0;

  case WM_SIZE:
  {
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

void
BaseApp::onResize(unsigned int newW, unsigned int newH)
{
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

  hr = m_depthStencil.init(
    m_device,
    newW,
    newH,
    DXGI_FORMAT_D24_UNORM_S8_UINT,
    D3D11_BIND_DEPTH_STENCIL,
    4,
    16
  );
  if (FAILED(hr)) return;

  hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
  if (FAILED(hr)) return;

  m_viewport.init(m_window);

  m_camera.setLens(XM_PIDIV4, newW / (float)newH, 0.01f, 100.0f);
}

void
BaseApp::handleEditorViewportResize()
{
  if (!m_editorViewportResizePending)
    return;

  m_deviceContext.m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);

  ID3D11ShaderResourceView* nullSRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
  m_deviceContext.m_deviceContext->PSSetShaderResources(
    0,
    D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT,
    nullSRVs
  );

  EditorViewportPass newPass;
  HRESULT hr = newPass.init(m_device, m_pendingViewportWidth, m_pendingViewportHeight);
  if (FAILED(hr))
  {
    m_editorViewportResizePending = false;
    return;
  }

  m_editorViewportPass.swap(newPass);
  m_forwardRenderer.resize(m_device, m_pendingViewportWidth, m_pendingViewportHeight);

  m_editorViewportResizePending = false;
}

std::string
BaseApp::getDefaultScenePath() const
{
  CreateDirectoryA("Saved", nullptr);
  return "Saved/DefaultScene.wvscene";
}

bool
BaseApp::saveScene(const std::string& path)
{
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
    if (actor.isNull()) {
      continue;
    }

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

      const std::vector<MaterialInstance*>& materials = meshRenderer->getMaterialInstances();
      stream << "MATERIAL_COUNT " << materials.size() << "\n";

      for (size_t i = 0; i < materials.size(); ++i) {
        MaterialInstance* materialInstance = materials[i];
        if (!materialInstance) {
          stream << "MATERIAL " << i << " 0 0 1 1 1 1 0 1 1 1 0.5\n";
          continue;
        }

        Material* material = materialInstance->getMaterial();
        const MaterialParams& params = materialInstance->getParams();

        const int domain = material ? static_cast<int>(material->getDomain()) : 0;
        const int blendMode = material ? static_cast<int>(material->getBlendMode()) : 0;

        stream << "MATERIAL " << i << " "
          << domain << " "
          << blendMode << " "
          << params.baseColor.x << " "
          << params.baseColor.y << " "
          << params.baseColor.z << " "
          << params.baseColor.w << " "
          << params.metallic << " "
          << params.roughness << " "
          << params.ao << " "
          << params.normalScale << " "
          << params.alphaCutoff << "\n";
      }
    }

    EU::TSharedPointer<LightComponent> lightComponent =
      actor->getComponent<LightComponent>();

    if (lightComponent) {
      const LightData& light = lightComponent->getLightData();
      stream << "LIGHT_COMPONENT "
        << static_cast<int>(light.type) << " "
        << light.color.x << " "
        << light.color.y << " "
        << light.color.z << " "
        << light.intensity << " "
        << light.direction.x << " "
        << light.direction.y << " "
        << light.direction.z << " "
        << light.range << " "
        << light.position.x << " "
        << light.position.y << " "
        << light.position.z << " "
        << light.spotAngle << " "
        << (lightComponent->canCastShadow() ? 1 : 0) << "\n";
    }

    stream << "END_ACTOR\n";
  }

  stream << "LIGHT "
    << m_constantBufferStruct.LightDir.x << " "
    << m_constantBufferStruct.LightDir.y << " "
    << m_constantBufferStruct.LightDir.z << " "
    << m_constantBufferStruct.LightColor.x << " "
    << m_constantBufferStruct.LightColor.y << " "
    << m_constantBufferStruct.LightColor.z << "\n";

  stream << "END_SCENE\n";
  return true;
}

bool
BaseApp::loadScene(const std::string& path)
{
  std::ifstream stream(path);
  if (!stream.is_open()) {
    return false;
  }

  std::string token;
  stream >> token;
  if (token != "WVSCENE") {
    return false;
  }

  int version = 0;
  stream >> version;
  if (version != 1) {
    return false;
  }

  EU::TSharedPointer<Actor> currentActor;

  while (stream >> token) {
    if (token == "ACTOR_COUNT") {
      size_t ignoredCount = 0;
      stream >> ignoredCount;
    }
    else if (token == "ACTOR") {
      size_t actorIndex = 0;
      std::string actorName;
      stream >> actorIndex >> std::quoted(actorName);

      currentActor = EU::TSharedPointer<Actor>();
      if (actorIndex < m_actors.size()) {
        currentActor = m_actors[actorIndex];
      }

      if (!currentActor.isNull()) {
        currentActor->setName(actorName);
      }
    }
    else if (token == "POSITION" && !currentActor.isNull()) {
      float x = 0.0f, y = 0.0f, z = 0.0f;
      stream >> x >> y >> z;

      EU::TSharedPointer<Transform> transform = currentActor->getComponent<Transform>();
      if (transform) {
        transform->setPosition(EU::Vector3(x, y, z));
      }
    }
    else if (token == "ROTATION" && !currentActor.isNull()) {
      float x = 0.0f, y = 0.0f, z = 0.0f;
      stream >> x >> y >> z;

      EU::TSharedPointer<Transform> transform = currentActor->getComponent<Transform>();
      if (transform) {
        transform->setRotation(EU::Vector3(x, y, z));
      }
    }
    else if (token == "SCALE" && !currentActor.isNull()) {
      float x = 1.0f, y = 1.0f, z = 1.0f;
      stream >> x >> y >> z;

      EU::TSharedPointer<Transform> transform = currentActor->getComponent<Transform>();
      if (transform) {
        transform->setScale(EU::Vector3(x, y, z));
      }
    }
    else if (token == "VISIBLE" && !currentActor.isNull()) {
      int value = 1;
      stream >> value;

      EU::TSharedPointer<MeshRendererComponent> meshRenderer =
        currentActor->getComponent<MeshRendererComponent>();

      if (meshRenderer) {
        meshRenderer->setVisible(value != 0);
      }
    }
    else if (token == "CAST_SHADOW" && !currentActor.isNull()) {
      int value = 1;
      stream >> value;

      EU::TSharedPointer<MeshRendererComponent> meshRenderer =
        currentActor->getComponent<MeshRendererComponent>();

      if (meshRenderer) {
        meshRenderer->setCastShadow(value != 0);
      }
    }
    else if (token == "MATERIAL_COUNT") {
      size_t ignoredCount = 0;
      stream >> ignoredCount;
    }
    else if (token == "MATERIAL" && !currentActor.isNull()) {
      size_t materialIndex = 0;
      int domain = 0;
      int blendMode = 0;
      MaterialParams params{};

      stream >> materialIndex
        >> domain
        >> blendMode
        >> params.baseColor.x
        >> params.baseColor.y
        >> params.baseColor.z
        >> params.baseColor.w
        >> params.metallic
        >> params.roughness
        >> params.ao
        >> params.normalScale
        >> params.alphaCutoff;

      EU::TSharedPointer<MeshRendererComponent> meshRenderer =
        currentActor->getComponent<MeshRendererComponent>();

      if (meshRenderer) {
        const std::vector<MaterialInstance*>& materials = meshRenderer->getMaterialInstances();
        if (materialIndex < materials.size() && materials[materialIndex]) {
          materials[materialIndex]->getParams() = params;

          Material* material = materials[materialIndex]->getMaterial();
          if (material) {
            material->setDomain(static_cast<MaterialDomain>(domain));
            material->setBlendMode(static_cast<BlendMode>(blendMode));
          }
        }
      }
    }
    else if (token == "LIGHT_COMPONENT" && !currentActor.isNull()) {
      int lightType = static_cast<int>(LightType::Directional);
      int castShadow = 0;
      LightData light{};

      stream >> lightType
        >> light.color.x
        >> light.color.y
        >> light.color.z
        >> light.intensity
        >> light.direction.x
        >> light.direction.y
        >> light.direction.z
        >> light.range
        >> light.position.x
        >> light.position.y
        >> light.position.z
        >> light.spotAngle
        >> castShadow;

      if (lightType < static_cast<int>(LightType::Directional) ||
        lightType > static_cast<int>(LightType::Spot)) {
        lightType = static_cast<int>(LightType::Directional);
      }

      light.type = static_cast<LightType>(lightType);

      EU::TSharedPointer<LightComponent> lightComponent =
        currentActor->getComponent<LightComponent>();

      if (!lightComponent) {
        lightComponent = EU::MakeShared<LightComponent>();
        currentActor->addComponent(lightComponent);
      }

      lightComponent->getLightData() = light;
      lightComponent->setCastShadow(castShadow != 0);

      if (light.type == LightType::Directional && m_directionalLightActor.isNull()) {
        m_directionalLightActor = currentActor;
      }
    }
    else if (token == "LIGHT") {
      stream >> m_constantBufferStruct.LightDir.x
        >> m_constantBufferStruct.LightDir.y
        >> m_constantBufferStruct.LightDir.z
        >> m_constantBufferStruct.LightColor.x
        >> m_constantBufferStruct.LightColor.y
        >> m_constantBufferStruct.LightColor.z;

      if (!m_directionalLightActor.isNull()) {
        EU::TSharedPointer<LightComponent> lightComponent =
          m_directionalLightActor->getComponent<LightComponent>();

        if (lightComponent) {
          lightComponent->getLightData().direction = m_constantBufferStruct.LightDir;
          lightComponent->getLightData().color = m_constantBufferStruct.LightColor;
        }
      }
    }
    else if (token == "END_ACTOR") {
      currentActor = EU::TSharedPointer<Actor>();
    }
    else if (token == "END_SCENE") {
      break;
    }
  }

  enforceDefaultSceneLayout();
  return true;
}
