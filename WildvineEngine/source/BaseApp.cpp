#include "BaseApp.h"
#include "ECS/AudioSourceComponent.h"
#include "ECS/RuntimeBehaviorComponent.h"
#include "ResourceManager.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <limits>
#include <chrono>
#include <cfloat>

extern LRESULT
ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace {

static std::string
toLowerCopy(std::string s) {
  for (char &c : s) {
    if (c >= 'A' && c <= 'Z')
      c = (char)(c + 32);
  }
  return s;
}

static std::string
stripExt(const std::string &f) {
  size_t d = f.find_last_of('.');
  return (d == std::string::npos) ? f : f.substr(0, d);
}

static std::string
fileBaseName(const std::string &path) {
  size_t s = path.find_last_of("/\\");
  std::string n = (s == std::string::npos) ? path : path.substr(s + 1);
  return stripExt(n);
}

static bool
endsWith(const std::string &s, const std::string &suf) {
  return s.size() >= suf.size() && s.compare(s.size() - suf.size(), suf.size(), suf) == 0;
}

static bool
containsStr(const std::string &s, const std::string &sub) {
  return s.find(sub) != std::string::npos;
}

static bool
fileExists(const std::string &path) {
  const DWORD attributes = GetFileAttributesA(path.c_str());
  return attributes != INVALID_FILE_ATTRIBUTES &&
         !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

static void
updateWorldBounds(RenderObject &object) {
  if (!object.mesh || !object.mesh->hasLocalBounds())
    return;
  const EU::Vector3 &localMin = object.mesh->getLocalBoundsMin();
  const EU::Vector3 &localMax = object.mesh->getLocalBoundsMax();
  object.boundsMin = EU::Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
  object.boundsMax = EU::Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
  for (int corner = 0; corner < 8; ++corner) {
    XMVECTOR point =
        XMVector3TransformCoord(XMVectorSet((corner & 1) ? localMax.x : localMin.x,
                                            (corner & 2) ? localMax.y : localMin.y,
                                            (corner & 4) ? localMax.z : localMin.z,
                                            1.0f),
                                object.world);
    XMFLOAT3 value;
    XMStoreFloat3(&value, point);
    object.boundsMin.x = fminf(object.boundsMin.x, value.x);
    object.boundsMin.y = fminf(object.boundsMin.y, value.y);
    object.boundsMin.z = fminf(object.boundsMin.z, value.z);
    object.boundsMax.x = fmaxf(object.boundsMax.x, value.x);
    object.boundsMax.y = fmaxf(object.boundsMax.y, value.y);
    object.boundsMax.z = fmaxf(object.boundsMax.z, value.z);
  }
}

constexpr uint32_t kSceneBinaryMagic = 0x4E435357; // WSCN
constexpr uint32_t kSceneBinaryVersion = 3;
constexpr uint32_t kPrefabBinaryMagic = 0x46505657; // WVPF
constexpr uint32_t kPrefabBinaryVersion = 1;

template <typename T>
static bool
writeBinaryValue(std::ofstream &stream, const T &value) {
  stream.write(reinterpret_cast<const char *>(&value), sizeof(T));
  return stream.good();
}

template <typename T>
static bool
readBinaryValue(std::ifstream &stream, T &value) {
  stream.read(reinterpret_cast<char *>(&value), sizeof(T));
  return stream.good();
}

static bool
writeBinaryString(std::ofstream &stream, const std::string &value) {
  const uint32_t length = static_cast<uint32_t>(value.size());
  if (!writeBinaryValue(stream, length))
    return false;
  if (length > 0) {
    stream.write(value.data(), length);
  }
  return stream.good();
}

static bool
readBinaryString(std::ifstream &stream, std::string &value) {
  uint32_t length = 0;
  if (!readBinaryValue(stream, length))
    return false;
  value.resize(length);
  if (length > 0) {
    stream.read(&value[0], length);
  }
  return stream.good();
}

static bool
writeBinaryVector3(std::ofstream &stream, const EU::Vector3 &value) {
  return writeBinaryValue(stream, value.x) && writeBinaryValue(stream, value.y) &&
         writeBinaryValue(stream, value.z);
}

static bool
readBinaryVector3(std::ifstream &stream, EU::Vector3 &value) {
  return readBinaryValue(stream, value.x) && readBinaryValue(stream, value.y) &&
         readBinaryValue(stream, value.z);
}

static bool
writeParticleSettings(std::ofstream &stream, const ParticleEmitterSettings &settings) {
  const uint8_t shape = static_cast<uint8_t>(settings.shape);
  const uint8_t emissionMode = static_cast<uint8_t>(settings.emissionMode);
  const uint8_t blendMode = static_cast<uint8_t>(settings.blendMode);
  const uint8_t enabled = settings.enabled ? 1 : 0;
  return writeBinaryValue(stream, shape) && writeBinaryValue(stream, emissionMode) &&
         writeBinaryValue(stream, blendMode) &&
         writeBinaryVector3(stream, settings.direction) &&
         writeBinaryVector3(stream, settings.gravity) &&
         writeBinaryVector3(stream, settings.boxExtents) &&
         writeBinaryValue(stream, settings.sphereRadius) &&
         writeBinaryValue(stream, settings.coneAngleDegrees) &&
         writeBinaryValue(stream, settings.spread) &&
         writeBinaryValue(stream, settings.minSpeed) &&
         writeBinaryValue(stream, settings.maxSpeed) &&
         writeBinaryValue(stream, settings.emissionRate) &&
         writeBinaryValue(stream, settings.lifetime) &&
         writeBinaryValue(stream, settings.startSize) &&
         writeBinaryValue(stream, settings.endSize) &&
         writeBinaryVector3(stream, settings.startColor) &&
         writeBinaryVector3(stream, settings.endColor) &&
         writeBinaryValue(stream, settings.burstCount) &&
         writeBinaryValue(stream, enabled);
}

static bool
readParticleSettings(std::ifstream &stream, ParticleEmitterSettings &settings) {
  uint8_t shape = 0;
  uint8_t emissionMode = 0;
  uint8_t blendMode = 0;
  uint8_t enabled = 1;
  const bool ok =
      readBinaryValue(stream, shape) && readBinaryValue(stream, emissionMode) &&
      readBinaryValue(stream, blendMode) &&
      readBinaryVector3(stream, settings.direction) &&
      readBinaryVector3(stream, settings.gravity) &&
      readBinaryVector3(stream, settings.boxExtents) &&
      readBinaryValue(stream, settings.sphereRadius) &&
      readBinaryValue(stream, settings.coneAngleDegrees) &&
      readBinaryValue(stream, settings.spread) &&
      readBinaryValue(stream, settings.minSpeed) &&
      readBinaryValue(stream, settings.maxSpeed) &&
      readBinaryValue(stream, settings.emissionRate) &&
      readBinaryValue(stream, settings.lifetime) &&
      readBinaryValue(stream, settings.startSize) &&
      readBinaryValue(stream, settings.endSize) &&
      readBinaryVector3(stream, settings.startColor) &&
      readBinaryVector3(stream, settings.endColor) &&
      readBinaryValue(stream, settings.burstCount) && readBinaryValue(stream, enabled);
  if (!ok)
    return false;
  if (shape > static_cast<uint8_t>(ParticleEmitterShape::Cone) ||
      emissionMode > static_cast<uint8_t>(ParticleEmissionMode::Burst) ||
      blendMode > static_cast<uint8_t>(ParticleBlendMode::Alpha)) {
    return false;
  }
  settings.shape = static_cast<ParticleEmitterShape>(shape);
  settings.emissionMode = static_cast<ParticleEmissionMode>(emissionMode);
  settings.blendMode = static_cast<ParticleBlendMode>(blendMode);
  settings.enabled = enabled != 0;
  return true;
}

static bool
isAbsolutePath(const std::string &path) {
  return path.size() > 2 && ((path[1] == ':' && (path[2] == '\\' || path[2] == '/')) ||
                             (path[0] == '\\' && path[1] == '\\'));
}

static std::string
directoryOf(const std::string &path) {
  const size_t slash = path.find_last_of("/\\");
  return (slash == std::string::npos) ? std::string() : path.substr(0, slash);
}

static std::string
fileNameOf(const std::string &path) {
  const size_t slash = path.find_last_of("/\\");
  return (slash == std::string::npos) ? path : path.substr(slash + 1);
}

static ExtensionType
extFromName(const std::string &lower) {
  if (endsWith(lower, ".jpg") || endsWith(lower, ".jpeg"))
    return JPG;
  if (endsWith(lower, ".dds"))
    return DDS;
  return PNG;
}

static std::vector<std::string>
listImageFiles(const std::string &dir) {
  std::vector<std::string> out;
  std::string pat = dir + "\\*";
  WIN32_FIND_DATAA fd;
  HANDLE h = FindFirstFileA(pat.c_str(), &fd);
  if (h == INVALID_HANDLE_VALUE)
    return out;
  do {
    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      continue;
    std::string lo = toLowerCopy(fd.cFileName);
    if (endsWith(lo, ".png") || endsWith(lo, ".jpg") || endsWith(lo, ".jpeg") ||
        endsWith(lo, ".dds") || endsWith(lo, ".tga"))
      out.push_back(fd.cFileName);
  } while (FindNextFileA(h, &fd));
  FindClose(h);
  return out;
}

static std::vector<std::string>
listSubfolders(const std::string &dir) {
  std::vector<std::string> out;
  std::string pat = dir + "\\*";
  WIN32_FIND_DATAA fd;
  HANDLE h = FindFirstFileA(pat.c_str(), &fd);
  if (h == INVALID_HANDLE_VALUE)
    return out;
  do {
    if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      continue;
    std::string n = fd.cFileName;
    if (n == "." || n == "..")
      continue;
    out.push_back(n);
  } while (FindNextFileA(h, &fd));
  FindClose(h);
  return out;
}

class

    TransformCommand : public ICommand {
public:
  TransformCommand(EU::TSharedPointer<Actor> actor,
                   const GizmoEditState &before,
                   const GizmoEditState &after)
      : m_actor(actor)
      , m_before(before)
      , m_after(after) {
  }
  void
  undo() override {
    apply(m_before);
  }
  void
  redo() override {
    apply(m_after);
  }
  const char *
  name() const override {
    return "Transform";
  }

private:
  void
  apply(const GizmoEditState &s) {
    if (m_actor.isNull())
      return;
    EU::TSharedPointer<Transform> t = m_actor->getComponent<Transform>();
    if (!t)
      return;
    t->setPosition(s.position);
    t->setRotation(s.rotation);
    t->setScale(s.scale);
  }
  EU::TSharedPointer<Actor> m_actor;
  GizmoEditState m_before;
  GizmoEditState m_after;
};

class

    SpawnActorCommand : public ICommand {
public:
  SpawnActorCommand(BaseApp *app, EU::TSharedPointer<Actor> actor)
      : m_app(app)
      , m_actor(actor) {
  }
  void
  undo() override {
    if (m_app)
      m_app->removeActorFromScene(m_actor);
  }
  void
  redo() override {
    if (m_app)
      m_app->addActorToScene(m_actor);
  }
  const char *
  name() const override {
    return "Spawn Actor";
  }

private:
  BaseApp *m_app;
  EU::TSharedPointer<Actor> m_actor;
};

class

    DeleteActorCommand : public ICommand {
public:
  DeleteActorCommand(BaseApp *app, EU::TSharedPointer<Actor> actor)
      : m_app(app)
      , m_actor(actor) {
  }
  void
  undo() override {
    if (m_app)
      m_app->addActorToScene(m_actor);
  }
  void
  redo() override {
    if (m_app)
      m_app->removeActorFromScene(m_actor);
  }
  const char *
  name() const override {
    return "Delete Actor";
  }

private:
  BaseApp *m_app;
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
BaseApp::createLightActor(LightType type, const std::string &baseName) {
  // Solo reusar la directional si ya existe una Y se pide crear otra directional
  if (type == LightType::Directional && !m_directionalLightActor.isNull()) {
    return m_directionalLightActor;
  }

  EU::TSharedPointer<Actor> lightActor = EU::MakeShared<Actor>(m_device);
  if (lightActor.isNull()) {
    return lightActor;
  }

  lightActor->setName(baseName);

  EU::TSharedPointer<LightComponent> lightComponent =
      lightActor->getComponent<LightComponent>();
  if (!lightComponent) {
    lightComponent = EU::MakeShared<LightComponent>();
    lightActor->addComponent(lightComponent);
  }

  LightData &light = lightComponent->getLightData();
  light.type = type;
  light.color = EU::Vector3(1.0f, 1.0f, 1.0f);
  light.intensity = 3.0f;
  light.direction = EU::Vector3(-0.35f, -0.25f, -1.0f);
  light.position = EU::Vector3(0.0f, 0.0f, 3.0f);

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
    light.position = EU::Vector3(-4.0f, -4.0f, 6.0f);
  }

  lightComponent->setCastShadow(type == LightType::Directional);

  m_actors.push_back(lightActor);
  m_sceneGraph.addEntity(lightActor.get());

  EU::TSharedPointer<Transform> transform = lightActor->getComponent<Transform>();
  if (transform) {
    transform->setTransform(
        light.position, EU::Vector3(0.0f, 0.0f, 0.0f), EU::Vector3(1.0f, 1.0f, 1.0f));
    transform->rebuildMatrixFromVectors();
  }

  if (type == LightType::Directional && m_directionalLightActor.isNull()) {
    m_directionalLightActor = lightActor;
  }
  selectActor(lightActor);
  return lightActor;
}

EU::TSharedPointer<Actor>
BaseApp::createParticleEmitterActor() {
  EU::TSharedPointer<Actor> actor = EU::MakeShared<Actor>(m_device);
  if (actor.isNull())
    return actor;
  actor->setName("ParticleEmitter");
  EU::TSharedPointer<Transform> transform = actor->getComponent<Transform>();
  transform->setTransform(EU::Vector3(0.0f, 0.0f, 0.0f),
                          EU::Vector3(0.0f, 0.0f, 0.0f),
                          EU::Vector3(1.0f, 1.0f, 1.0f));
  transform->rebuildMatrixFromVectors();
  EU::TSharedPointer<ParticleEmitterComponent> emitter =
      EU::MakeShared<ParticleEmitterComponent>(transform.get());
  if (FAILED(emitter->init(m_device)))
    return EU::TSharedPointer<Actor>();
  actor->addComponent(emitter);
  addActorToScene(actor);
  selectActor(actor);
  return actor;
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

    LightData &light = lightComponent->getLightData();
    light.type = LightType::Directional;
    if (light.intensity <= 0.0f)
      light.intensity = 3.0f;
    if (light.direction.x == 0.0f && light.direction.y == 0.0f &&
        light.direction.z == 0.0f) {
      light.direction = EU::Vector3(-0.35f, -0.25f, -1.0f);
    }
    light.range = 0.0f;
    light.spotAngle = 0.0f;
    lightComponent->setCastShadow(true);
  }

  Actor *modelActor = m_cyberGun.isNull() ? nullptr : m_cyberGun.get();
  Actor *lightActor =
      m_directionalLightActor.isNull() ? nullptr : m_directionalLightActor.get();

  for (int i = static_cast<int>(m_actors.size()) - 1; i >= 0; --i) {
    Actor *actor = m_actors[i].isNull() ? nullptr : m_actors[i].get();
    if (actor != modelActor && actor != lightActor) {
      if (actor)
        m_sceneGraph.removeEntity(actor);
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
    m_camera.lookAt(EU::Vector3(0.0f, -12.0f, 3.0f), EU::Vector3(0.0f, 0.0f, 0.0f));
    return;
  }

  EU::Vector3 minBounds((std::numeric_limits<float>::max)(),
                        (std::numeric_limits<float>::max)(),
                        (std::numeric_limits<float>::max)());
  EU::Vector3 maxBounds(-(std::numeric_limits<float>::max)(),
                        -(std::numeric_limits<float>::max)(),
                        -(std::numeric_limits<float>::max)());

  bool hasVertices = false;
  for (const MeshComponent &meshComponent : m_model->GetMeshes()) {
    for (const SimpleVertex &vertex : meshComponent.m_vertex) {
      const EU::Vector3 &p = vertex.Position;
      if (p.x < minBounds.x)
        minBounds.x = p.x;
      if (p.y < minBounds.y)
        minBounds.y = p.y;
      if (p.z < minBounds.z)
        minBounds.z = p.z;
      if (p.x > maxBounds.x)
        maxBounds.x = p.x;
      if (p.y > maxBounds.y)
        maxBounds.y = p.y;
      if (p.z > maxBounds.z)
        maxBounds.z = p.z;
      hasVertices = true;
    }
  }

  if (!hasVertices) {
    m_camera.lookAt(EU::Vector3(0.0f, -12.0f, 3.0f), EU::Vector3(0.0f, 0.0f, 0.0f));
    return;
  }

  EU::Vector3 center((minBounds.x + maxBounds.x) * 0.5f,
                     (minBounds.y + maxBounds.y) * 0.5f,
                     (minBounds.z + maxBounds.z) * 0.5f);

  EU::Vector3 extents((maxBounds.x - minBounds.x) * 0.5f,
                      (maxBounds.y - minBounds.y) * 0.5f,
                      (maxBounds.z - minBounds.z) * 0.5f);

  EU::TSharedPointer<Transform> transform = m_cyberGun->getComponent<Transform>();
  if (transform) {
    const EU::Vector3 &position = transform->getPosition();
    const EU::Vector3 &scale = transform->getScale();
    center = EU::Vector3(position.x + center.x * scale.x,
                         position.y + center.y * scale.y,
                         position.z + center.z * scale.z);
    extents = EU::Vector3(std::abs(extents.x * scale.x),
                          std::abs(extents.y * scale.y),
                          std::abs(extents.z * scale.z));
  }

  const float computedRadius =
      std::sqrt(extents.x * extents.x + extents.y * extents.y + extents.z * extents.z);
  const float radius = computedRadius > 1.0f ? computedRadius : 1.0f;
  const float fovY = m_camera.getFovY() > 0.35f ? m_camera.getFovY() : 0.35f;
  const float computedDistance = (radius / std::tan(fovY * 0.5f)) * 1.35f;
  const float distance = computedDistance > 8.0f ? computedDistance : 8.0f;
  const float computedHeight = radius * 0.18f;
  const float height = computedHeight > 1.5f ? computedHeight : 1.5f;

  EU::Vector3 cameraPosition(center.x, center.y - distance, center.z + height);
  m_camera.lookAt(cameraPosition, center);
}

void
BaseApp::selectActor(EU::TSharedPointer<Actor> actor) {
  if (actor.isNull()) {
    m_gui.selectedActorIndex = -1;
    return;
  }
  Actor *target = actor.get();
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

  for (auto &actor : m_actors) {
    if (actor.isNull())
      continue;
    EU::TSharedPointer<LightComponent> lightComponent =
        actor->getComponent<LightComponent>();
    if (!lightComponent)
      continue;

    LightData &light = lightComponent->getLightData();
    EU::TSharedPointer<Transform> transform = actor->getComponent<Transform>();
    if (transform)
      light.position = transform->getPosition();

    if (light.type == LightType::Directional && firstDirectional.isNull()) {
      firstDirectional = actor;
      m_constantBufferStruct.LightDir = light.direction;
      m_constantBufferStruct.LightColor = light.color * light.intensity;
    }
  }

  m_directionalLightActor = firstDirectional;
}

HRESULT
BaseApp::loadPbrTexture(Texture &texture,
                        const std::string &path,
                        const char *debugName) {
  HRESULT hr = texture.initFromFile(m_device, path);
  if (FAILED(hr)) {
    ERROR("Main",
          "LoadPbrTexture",
          ("Failed to initialize " + std::string(debugName) + " texture: " + path +
           ". HRESULT: " + std::to_string(hr))
              .c_str());
  }
  return hr;
}

HRESULT
BaseApp::buildEditorGridMesh() {
  m_editorGridMesh.destroy();

  MeshComponent gridMesh;
  constexpr int halfCells = 100;
  constexpr float spacing = 1.0f;
  const float extent = halfCells * spacing;

  auto pushVertex = [&](float x, float y) -> unsigned int {
    SimpleVertex v{};
    v.Position = EU::Vector3(x, y, 0.0f);
    v.Normal = EU::Vector3(0.0f, 0.0f, 1.0f);
    v.Tangent = EU::Vector3(1.0f, 0.0f, 0.0f);
    v.Bitangent = EU::Vector3(0.0f, 1.0f, 0.0f);
    v.TextureCoordinate = EU::Vector2(0.0f, 0.0f);
    gridMesh.m_vertex.push_back(v);
    return static_cast<unsigned int>(gridMesh.m_vertex.size() - 1);
  };

  auto pushLine = [&](float x0, float y0, float x1, float y1) {
    gridMesh.m_index.push_back(pushVertex(x0, y0));
    gridMesh.m_index.push_back(pushVertex(x1, y1));
  };

  for (int i = -halfCells; i <= halfCells; ++i) {
    const float p = i * spacing;
    pushLine(-extent, p, extent, p);
    pushLine(p, -extent, p, extent);
  }

  gridMesh.m_numVertex = static_cast<int>(gridMesh.m_vertex.size());
  gridMesh.m_numIndex = static_cast<int>(gridMesh.m_index.size());

  Submesh submesh{};
  HRESULT hr = submesh.vertexBuffer.init(m_device, gridMesh, D3D11_BIND_VERTEX_BUFFER);
  if (FAILED(hr))
    return hr;
  hr = submesh.indexBuffer.init(m_device, gridMesh, D3D11_BIND_INDEX_BUFFER);
  if (FAILED(hr))
    return hr;
  submesh.indexCount = gridMesh.m_numIndex;
  submesh.startIndex = 0;
  submesh.materialSlot = 0;
  m_editorGridMesh.getSubmeshes().push_back(std::move(submesh));
  m_editorGridReady = true;
  return S_OK;
}

void
BaseApp::addEditorGridToRenderScene() {
  if (!m_gui.m_showGrid || !m_editorGridReady)
    return;

  RenderObject gridObject{};
  gridObject.mesh = &m_editorGridMesh;
  gridObject.materialInstance = &m_editorGridMaterialInstance;
  gridObject.materialInstances.push_back(&m_editorGridMaterialInstance);
  gridObject.world = XMMatrixIdentity();
  gridObject.topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
  gridObject.castShadow = false;
  gridObject.transparent = false;
  gridObject.distanceToCamera = 0.0f;
  m_renderScene.opaqueObjects.push_back(gridObject);
}

HRESULT
BaseApp::rebuildEditorSelectionMesh(const EU::Vector3 &localMin,
                                    const EU::Vector3 &localMax) {
  m_editorSelectionMesh.destroy();
  m_editorSelectionReady = false;

  MeshComponent mesh;

  auto pushVertex = [&](float x, float y, float z) -> unsigned int {
    SimpleVertex v{};
    v.Position = EU::Vector3(x, y, z);
    v.Normal = EU::Vector3(0.0f, 0.0f, 1.0f);
    v.Tangent = EU::Vector3(1.0f, 0.0f, 0.0f);
    v.Bitangent = EU::Vector3(0.0f, 1.0f, 0.0f);
    v.TextureCoordinate = EU::Vector2(0.0f, 0.0f);
    mesh.m_vertex.push_back(v);
    return static_cast<unsigned int>(mesh.m_vertex.size() - 1);
  };

  const unsigned int corners[] = {pushVertex(localMin.x, localMin.y, localMin.z),
                                  pushVertex(localMax.x, localMin.y, localMin.z),
                                  pushVertex(localMax.x, localMax.y, localMin.z),
                                  pushVertex(localMin.x, localMax.y, localMin.z),
                                  pushVertex(localMin.x, localMin.y, localMax.z),
                                  pushVertex(localMax.x, localMin.y, localMax.z),
                                  pushVertex(localMax.x, localMax.y, localMax.z),
                                  pushVertex(localMin.x, localMax.y, localMax.z)};
  const unsigned int edges[] = {0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6,
                                6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7};
  for (unsigned int i : edges) {
    mesh.m_index.push_back(corners[i]);
  }

  mesh.m_numVertex = static_cast<int>(mesh.m_vertex.size());
  mesh.m_numIndex = static_cast<int>(mesh.m_index.size());

  Submesh submesh{};
  HRESULT hr = submesh.vertexBuffer.init(m_device, mesh, D3D11_BIND_VERTEX_BUFFER);
  if (FAILED(hr))
    return hr;
  hr = submesh.indexBuffer.init(m_device, mesh, D3D11_BIND_INDEX_BUFFER);
  if (FAILED(hr))
    return hr;
  submesh.indexCount = mesh.m_numIndex;
  submesh.startIndex = 0;
  submesh.materialSlot = 0;
  m_editorSelectionMesh.getSubmeshes().push_back(std::move(submesh));
  m_editorSelectionReady = true;
  return S_OK;
}

void
BaseApp::addEditorSelectionToRenderScene() {
  if (m_gui.selectedActorIndex < 0 ||
      m_gui.selectedActorIndex >= static_cast<int>(m_actors.size())) {
    m_editorSelectionCachedActorIndex = -1;
    return;
  }

  EU::TSharedPointer<Actor> actor = m_actors[m_gui.selectedActorIndex];
  if (actor.isNull()) {
    m_editorSelectionCachedActorIndex = -1;
    return;
  }
  EU::TSharedPointer<Transform> transform = actor->getComponent<Transform>();
  if (!transform) {
    m_editorSelectionCachedActorIndex = -1;
    return;
  }

  EU::Vector3 localMin, localMax;
  if (!getActorAABB(actor, localMin, localMax)) {
    m_editorSelectionCachedActorIndex = -1;
    return;
  }

  auto sameBounds = [](const EU::Vector3 &a, const EU::Vector3 &b) {
    const float eps = 0.0001f;
    return fabsf(a.x - b.x) < eps && fabsf(a.y - b.y) < eps && fabsf(a.z - b.z) < eps;
  };

  const bool needsSelectionRebuild =
      !m_editorSelectionReady ||
      m_editorSelectionCachedActorIndex != m_gui.selectedActorIndex ||
      !sameBounds(m_editorSelectionCachedMin, localMin) ||
      !sameBounds(m_editorSelectionCachedMax, localMax);

  if (needsSelectionRebuild) {
    if (FAILED(rebuildEditorSelectionMesh(localMin, localMax)) ||
        !m_editorSelectionReady) {
      return;
    }
    m_editorSelectionCachedActorIndex = m_gui.selectedActorIndex;
    m_editorSelectionCachedMin = localMin;
    m_editorSelectionCachedMax = localMax;
  }

  RenderObject selectionObject{};
  selectionObject.mesh = &m_editorSelectionMesh;
  selectionObject.materialInstance = &m_editorSelectionMaterialInstance;
  selectionObject.materialInstances.push_back(&m_editorSelectionMaterialInstance);
  selectionObject.world = transform->worldMatrix;
  selectionObject.topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
  selectionObject.castShadow = false;
  selectionObject.transparent = false;
  selectionObject.distanceToCamera = 0.0f;
  m_renderScene.opaqueObjects.push_back(selectionObject);
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
    } else {
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
    ERROR("Main",
          "InitDevice",
          ("Failed to initialize SwapChain. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // 2. Render Target View
  hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice", "Failed RTV.");
    return hr;
  }

  // 3. Depth Stencil Texture
  hr = m_depthStencil.init(m_device,
                           m_window.m_width,
                           m_window.m_height,
                           DXGI_FORMAT_D24_UNORM_S8_UINT,
                           D3D11_BIND_DEPTH_STENCIL,
                           4,
                           0);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice", "Failed DepthStencil.");
    return hr;
  }

  // 4. Depth Stencil View
  hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice", "Failed DSV.");
    return hr;
  }

  // 5. Viewport
  hr = m_viewport.init(m_window);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice", "Failed Viewport.");
    return hr;
  }
  m_d3dReady = true;

  hr = m_defaultWhiteSRV.initSolidColor(m_device, 255, 255, 255, 255, "DefaultWhite");
  if (FAILED(hr))
    return hr;
  hr = m_defaultBlackSRV.initSolidColor(m_device, 0, 0, 0, 255, "DefaultBlack");
  if (FAILED(hr))
    return hr;
  hr = m_defaultFlatNormalSRV.initSolidColor(
      m_device, 128, 128, 255, 255, "DefaultFlatNormal");
  if (FAILED(hr))
    return hr;

  // --------------------------------------------------------------------------
  // SKYBOX
  // --------------------------------------------------------------------------
  std::array<std::string, 6> faces = {"Skybox/cubemap_0.png",
                                      "Skybox/cubemap_1.png",
                                      "Skybox/cubemap_2.png",
                                      "Skybox/cubemap_3.png",
                                      "Skybox/cubemap_4.png",
                                      "Skybox/cubemap_5.png"};
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

    struct

        PbrTextureRequest {
      Texture *texture;
      const char *path;
      const char *debugName;
    };

    const PbrTextureRequest textureRequests[] = {
        {&m_AlbedoSRV, "Assets/Textures/CyberGun/base.tga", "Albedo"},
        {&m_MetallicSRV, "Assets/Textures/CyberGun/metallic.tga", "Metallic"},
        {&m_RoughnessSRV, "Assets/Textures/CyberGun/roughness.tga", "Roughness"},
        {&m_AOSRV, "Assets/Textures/CyberGun/ao.tga", "Ambient Occlusion"},
        {&m_NormalSRV, "Assets/Textures/CyberGun/normal.tga", "Normal"}};

    for (const PbrTextureRequest &request : textureRequests) {
      hr = loadPbrTexture(*request.texture, request.path, request.debugName);
      if (FAILED(hr))
        return hr;
    }

    std::vector<MeshComponent> cyberGunMeshes = m_model->GetMeshes();

    m_cyberGun->setName("SM_CyberGun");
    m_actors.push_back(m_cyberGun);

    EU::TSharedPointer<Transform> transform = m_cyberGun->getComponent<Transform>();
    if (transform) {
      transform->setTransform(EU::Vector3(0.0f, 0.0f, 0.0f),
                              EU::Vector3(0.0f, 0.0f, 0.0f),
                              EU::Vector3(1.0f, 1.0f, 1.0f));
      transform->rebuildMatrixFromVectors();

      EU::TSharedPointer<RuntimeBehaviorComponent> runtimeBehavior =
          EU::MakeShared<RuntimeBehaviorComponent>(transform.get());
      m_cyberGun->addComponent(runtimeBehavior);

      EU::TSharedPointer<AudioSourceComponent> audioSource =
          EU::MakeShared<AudioSourceComponent>(transform.get());
      m_cyberGun->addComponent(audioSource);
      m_audioSystem.registerSource(audioSource.get());
    }

    // Mesh de render
    m_cyberGunRenderMesh.destroy();
    for (const MeshComponent &meshComponent : cyberGunMeshes) {
      Submesh submesh{};
      hr = submesh.vertexBuffer.init(m_device, meshComponent, D3D11_BIND_VERTEX_BUFFER);
      if (FAILED(hr)) {
        ERROR("Main", "InitDevice", "Failed vertex buffer.");
        return hr;
      }
      hr = submesh.indexBuffer.init(m_device, meshComponent, D3D11_BIND_INDEX_BUFFER);
      if (FAILED(hr)) {
        ERROR("Main", "InitDevice", "Failed index buffer.");
        return hr;
      }
      submesh.indexCount = meshComponent.m_numIndex;
      submesh.startIndex = 0;
      submesh.materialSlot = 0;
      m_cyberGunRenderMesh.getSubmeshes().push_back(std::move(submesh));
    }

    // AABB local del modelo (para picking)
    m_modelLocalMin = EU::Vector3(1e9f, 1e9f, 1e9f);
    m_modelLocalMax = EU::Vector3(-1e9f, -1e9f, -1e9f);
    for (const MeshComponent &mc : cyberGunMeshes) {
      for (const SimpleVertex &v : mc.m_vertex) {
        m_modelLocalMin.x = fminf(m_modelLocalMin.x, v.Position.x);
        m_modelLocalMin.y = fminf(m_modelLocalMin.y, v.Position.y);
        m_modelLocalMin.z = fminf(m_modelLocalMin.z, v.Position.z);
        m_modelLocalMax.x = fmaxf(m_modelLocalMax.x, v.Position.x);
        m_modelLocalMax.y = fmaxf(m_modelLocalMax.y, v.Position.y);
        m_modelLocalMax.z = fmaxf(m_modelLocalMax.z, v.Position.z);
      }
    }
  } else {
    ERROR("Main", "InitDevice", "Failed to create CyberGun Actor.");
    return E_FAIL;
  }

  // --------------------------------------------------------------------------
  // SCENE GRAPH
  // --------------------------------------------------------------------------
  for (auto &actor : m_actors) {
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
    ERROR("Main", "InitDevice", "Failed ShaderProgram.");
    return hr;
  }

  hr = m_constantBuffer.init(m_device, sizeof(CBMain));
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice", "Failed constant buffer.");
    return hr;
  }

  // --------------------------------------------------------------------------
  // CAMERA / LIGHT
  // --------------------------------------------------------------------------
  m_camera.setLens(XM_PIDIV4, m_window.m_width / (float)m_window.m_height, 0.01f, 100.0f);
  m_camera.setPosition(0.0f, -6.0f, 3.0f);

  m_constantBufferStruct.LightColor = EU::Vector3(1.0f, 1.0f, 1.0f);
  m_constantBufferStruct.LightDir = EU::Vector3(-0.20f, -1.0f, -1.0f);

  m_skybox.init(m_device, &m_deviceContext, m_skyboxTex);

  // --------------------------------------------------------------------------
  // ESTADOS POR DEFECTO
  // --------------------------------------------------------------------------
  hr = m_defaultRasterizer.init(m_device, D3D11_FILL_SOLID, D3D11_CULL_BACK, false, true);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice", "Failed Rasterizer.");
    return hr;
  }

  hr = m_defaultDepthStencil.init(
      m_device, true, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice", "Failed DepthStencilState.");
    return hr;
  }

  hr = m_defaultSampler.init(m_device);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice", "Failed SamplerState.");
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

  m_editorGridMaterial.setShader(&m_shaderProgram);
  m_editorGridMaterial.setRasterizerState(&m_defaultRasterizer);
  m_editorGridMaterial.setDepthStencilState(&m_defaultDepthStencil);
  m_editorGridMaterial.setSamplerState(&m_defaultSampler);
  m_editorGridMaterial.setDomain(MaterialDomain::Opaque);
  m_editorGridMaterial.setBlendMode(BlendMode::Opaque);

  m_editorGridMaterialInstance.setMaterial(&m_editorGridMaterial);
  m_editorGridMaterialInstance.setAlbedo(&m_defaultWhiteSRV);
  m_editorGridMaterialInstance.setNormal(&m_defaultFlatNormalSRV);
  m_editorGridMaterialInstance.setMetallic(&m_defaultWhiteSRV);
  m_editorGridMaterialInstance.setRoughness(&m_defaultWhiteSRV);
  m_editorGridMaterialInstance.setAO(&m_defaultWhiteSRV);
  m_editorGridMaterialInstance.setEmissive(&m_defaultBlackSRV);
  m_editorGridMaterialInstance.getParams().baseColor =
      XMFLOAT4(0.18f, 0.20f, 0.20f, 1.0f);
  m_editorGridMaterialInstance.getParams().metallic = 0.0f;
  m_editorGridMaterialInstance.getParams().roughness = 1.0f;
  m_editorGridMaterialInstance.getParams().ao = 1.0f;
  m_editorGridMaterialInstance.getParams().normalScale = 0.0f;
  m_editorGridMaterialInstance.getParams().emissiveStrength = 0.0f;
  m_editorGridMaterialInstance.getParams().alphaCutoff = 0.0f;

  m_editorSelectionMaterialInstance.setMaterial(&m_editorGridMaterial);
  m_editorSelectionMaterialInstance.setAlbedo(&m_defaultWhiteSRV);
  m_editorSelectionMaterialInstance.setNormal(&m_defaultFlatNormalSRV);
  m_editorSelectionMaterialInstance.setMetallic(&m_defaultWhiteSRV);
  m_editorSelectionMaterialInstance.setRoughness(&m_defaultWhiteSRV);
  m_editorSelectionMaterialInstance.setAO(&m_defaultWhiteSRV);
  m_editorSelectionMaterialInstance.setEmissive(&m_defaultBlackSRV);
  m_editorSelectionMaterialInstance.getParams().baseColor =
      XMFLOAT4(0.55f, 0.24f, 0.95f, 1.0f);
  m_editorSelectionMaterialInstance.getParams().metallic = 0.0f;
  m_editorSelectionMaterialInstance.getParams().roughness = 0.65f;
  m_editorSelectionMaterialInstance.getParams().ao = 1.0f;
  m_editorSelectionMaterialInstance.getParams().normalScale = 0.0f;
  m_editorSelectionMaterialInstance.getParams().emissiveStrength = 0.0f;
  m_editorSelectionMaterialInstance.getParams().alphaCutoff = 0.0f;

  hr = buildEditorGridMesh();
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice", "Failed EditorGrid.");
    return hr;
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
  frameDefaultSceneCamera();

  // --------------------------------------------------------------------------
  // VIEWPORT PASS / RENDER PIPELINE (Deferred)
  // --------------------------------------------------------------------------
  hr = m_editorViewportPass.init(m_device, 1280, 720);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice", "Failed EditorViewportPass.");
    return hr;
  }

  hr = m_renderPipeline.init(m_device, RendererType::Deferred);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice", "Failed RenderPipeline.");
    return hr;
  }

  buildTextureThumbnails();

  m_audioSystem.init();

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

  if (m_gui.consumePlayRequest()) {
    startPlayMode();
  }

  if (m_gui.consumeStopRequest()) {
    stopPlayMode();
  }

  float audioMasterVolume = 1.0f;
  if (m_gui.consumeAudioMasterVolumeChange(audioMasterVolume)) {
    m_audioSystem.setMasterVolume(audioMasterVolume);
  }

  if (m_gui.consumeAudioPauseRequest()) {
    m_audioSystem.pause();
    m_gui.setAudioPaused(m_audioSystem.isPaused());
  }

  if (m_gui.consumeAudioResumeRequest()) {
    m_audioSystem.resume();
    m_gui.setAudioPaused(m_audioSystem.isPaused());
  }

  m_audioSystem.update(m_camera);

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

  m_gui.drawGBufferDebugPanel(m_renderPipeline.getGBufferAlbedoMetallicSRV(),
                              m_renderPipeline.getGBufferNormalRoughnessSRV(),
                              m_renderPipeline.getGBufferWorldAoSRV(),
                              m_renderPipeline.getGBufferEmissiveAlphaSRV());

  if (m_gui.shouldShowRenderDebug()) {
    m_gui.drawRenderDebugPanel(m_renderPipeline.getPreShadowSRV(),
                               m_editorViewportPass.getSRV(),
                               m_renderPipeline.getShadowMapSRV());
  }

  if (m_gui.shouldShowToolbox()) {
    m_gui.drawToolboxPanel();
  }

  if (m_gui.shouldShowAudioPanel()) {
    m_gui.drawAudioPanel();
  }

  m_gui.drawLightingPanel(&m_constantBufferStruct.LightDir.x,
                          &m_constantBufferStruct.LightColor.x);
  m_gui.drawStatsPanel(deltaTime,
                       m_lastDrawCalls,
                       m_cullingStats.submitted,
                       m_cullingStats.visible,
                       m_cullingStats.culled,
                       m_cullingStats.visitedNodes);
  m_gui.drawConsolePanel();
  m_gui.drawTexturePreview();
  m_gui.drawContentBrowser(m_thumbnails);

  if (m_gui.m_assetDeleteRequested) {
    const std::string deletePath = m_gui.m_assetDeletePath;
    m_gui.m_assetDeleteRequested = false;
    m_gui.m_assetDeletePath.clear();

    if (!deletePath.empty()) {
      const BOOL deletedModel = DeleteFileA(deletePath.c_str());
      DeleteFileA((deletePath + ".wvmesh").c_str());
      if (m_pendingModelImport.active && m_pendingModelImport.path == deletePath) {
        MESSAGE("BaseApp",
                "ContentBrowser",
                "El modelo se quitara del browser cuando termine el import activo");
      }
      if (m_queuedModelSpawnPath == deletePath) {
        m_queuedModelSpawnActive = false;
        m_queuedModelSpawnPath.clear();
      }
      if (m_gui.m_assetSpawnPath == deletePath) {
        m_gui.m_assetSpawnRequested = false;
        m_gui.m_assetSpawnPath.clear();
      }
      if (deletedModel) {
        MESSAGE("BaseApp", "ContentBrowser", ("Modelo eliminado: " + deletePath).c_str());
      } else {
        ERROR(
            "BaseApp", "ContentBrowser", ("No se pudo eliminar: " + deletePath).c_str());
      }
    }
    m_cyberGunRenderMesh.setLocalBounds(m_modelLocalMin, m_modelLocalMax);
  }

  if (m_pendingModelImport.active &&
      m_pendingModelImport.job.wait_for(std::chrono::seconds(0)) ==
          std::future_status::ready) {
    const bool imported = m_pendingModelImport.job.get();
    const std::string path = m_pendingModelImport.path;
    const EU::Vector3 spawnPosition = m_pendingModelImport.spawnPosition;
    m_pendingModelImport.active = false;

    if (imported) {
      EU::TSharedPointer<Actor> a = loadModelActor(path, spawnPosition);
      if (!a.isNull()) {
        addActorToScene(a);
        m_commands.push(std::unique_ptr<ICommand>(new SpawnActorCommand(this, a)));
        m_gui.selectedActorIndex = (int)m_actors.size() - 1;
        MESSAGE("BaseApp", "loadModelActor", "Modelo instanciado desde cache");
      }
    } else {
      ERROR("BaseApp", "AsyncModelImport", "No se pudo importar el modelo");
    }
  }

  if (m_gui.m_audioSpawnRequested) {
    const std::string audioPath = m_gui.m_audioSpawnPath;
    m_gui.m_audioSpawnRequested = false;
    m_gui.m_audioSpawnPath.clear();

    if (!audioPath.empty()) {
      EU::Vector3 spawnPosition(0.0f, 2.92f, 5.60f);
      if (!getViewportMouseGroundPosition(spawnPosition)) {
        const EU::Vector3 cameraPosition = m_camera.getPosition();
        const EU::Vector3 cameraForward = m_camera.GetForward();
        spawnPosition = EU::Vector3(cameraPosition.x + cameraForward.x * 8.0f,
                                    cameraPosition.y + cameraForward.y * 8.0f,
                                    0.0f);
      }

      EU::TSharedPointer<Actor> actor = EU::MakeShared<Actor>(m_device);
      if (!actor.isNull()) {
        actor->setName(fileBaseName(audioPath));
        EU::TSharedPointer<Transform> transform = actor->getComponent<Transform>();
        if (transform) {
          transform->setTransform(spawnPosition,
                                  EU::Vector3(0.0f, 0.0f, 0.0f),
                                  EU::Vector3(1.0f, 1.0f, 1.0f));
        }
        EU::TSharedPointer<AudioSourceComponent> audioSource =
            EU::MakeShared<AudioSourceComponent>(transform.get());
        audioSource->setAudioPath(audioPath);
        actor->addComponent(audioSource);
        m_audioSystem.registerSource(audioSource.get());
        addActorToScene(actor);
        m_commands.push(std::unique_ptr<ICommand>(new SpawnActorCommand(this, actor)));
        m_gui.selectedActorIndex = static_cast<int>(m_actors.size()) - 1;
        MESSAGE("BaseApp", "Audio", "Fuente de audio creada en la escena");
      }
    }
  }

  bool useQueuedModelSpawnPosition = false;
  EU::Vector3 queuedModelSpawnPosition(0.0f, 0.0f, 0.0f);
  if (!m_pendingModelImport.active && m_queuedModelSpawnActive) {
    m_gui.m_assetSpawnPath = m_queuedModelSpawnPath;
    m_gui.m_assetSpawnRequested = true;
    queuedModelSpawnPosition = m_queuedModelSpawnPosition;
    useQueuedModelSpawnPosition = true;
    m_queuedModelSpawnActive = false;
  }

  // Instanciar modelo desde el Content Browser
  if (m_gui.m_assetSpawnRequested) {
    EU::Vector3 spawnPosition(0.0f, 2.92f, 5.60f);
    if (useQueuedModelSpawnPosition) {
      spawnPosition = queuedModelSpawnPosition;
    } else {
      if (!getViewportMouseGroundPosition(spawnPosition)) {
        const EU::Vector3 cameraPosition = m_camera.getPosition();
        const EU::Vector3 cameraForward = m_camera.GetForward();
        spawnPosition = EU::Vector3(cameraPosition.x + cameraForward.x * 8.0f,
                                    cameraPosition.y + cameraForward.y * 8.0f,
                                    0.0f);
      }
    }
    const std::string lowerPath = toLowerCopy(m_gui.m_assetSpawnPath);
    bool importAsync = endsWith(lowerPath, ".fbx");
    WIN32_FILE_ATTRIBUTE_DATA fileData{};
    if (importAsync && GetFileAttributesExA(m_gui.m_assetSpawnPath.c_str(),
                                            GetFileExInfoStandard,
                                            &fileData)) {
      ULARGE_INTEGER size{};
      size.LowPart = fileData.nFileSizeLow;
      size.HighPart = fileData.nFileSizeHigh;
      importAsync = size.QuadPart > (8ull * 1024ull * 1024ull);
    }

    if (importAsync) {
      if (!m_pendingModelImport.active) {
        m_gui.m_assetSpawnRequested = false;
        const std::string path = m_gui.m_assetSpawnPath;
        m_pendingModelImport.path = path;
        m_pendingModelImport.spawnPosition = spawnPosition;
        m_pendingModelImport.active = true;
        m_pendingModelImport.job = std::async(std::launch::async, [path]() {
          std::string lower = toLowerCopy(path);
          ModelType type = FBX;
          if (endsWith(lower, ".obj"))
            type = OBJ;
          else if (endsWith(lower, ".glb") || endsWith(lower, ".gltf"))
            type = GLTF;
          Model3D model(path, type);
          return !model.GetMeshes().empty();
        });
        MESSAGE("BaseApp", "AsyncModelImport", "Importando FBX grande en background");
      } else {
        m_queuedModelSpawnActive = true;
        m_queuedModelSpawnPath = m_gui.m_assetSpawnPath;
        m_queuedModelSpawnPosition = spawnPosition;
        m_gui.m_assetSpawnRequested = false;
        MESSAGE("BaseApp",
                "AsyncModelImport",
                "Modelo en cola hasta terminar el import actual");
      }
    } else {
      m_gui.m_assetSpawnRequested = false;
      EU::TSharedPointer<Actor> a = loadModelActor(m_gui.m_assetSpawnPath, spawnPosition);
      if (!a.isNull()) {
        addActorToScene(a);
        m_commands.push(std::unique_ptr<ICommand>(new SpawnActorCommand(this, a)));
        m_gui.selectedActorIndex = (int)m_actors.size() - 1;
        MESSAGE("BaseApp", "loadModelActor", "Modelo instanciado desde Content");
      }
    }
  }

  // Aplicar textura desde el Content Browser
  if (m_gui.m_textureDropRequested) {
    m_gui.m_textureDropRequested = false;
    if (m_gui.selectedActorIndex >= 0 &&
        m_gui.selectedActorIndex < (int)m_actors.size()) {
      EU::TSharedPointer<Actor> selected = m_actors[m_gui.selectedActorIndex];
      if (!selected.isNull()) {
        EU::TSharedPointer<MeshRendererComponent> mr =
            selected->getComponent<MeshRendererComponent>();
        if (mr && mr->getMaterialInstance()) {
          Texture *newTex = new Texture();
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

  // Recargar miniaturas si se importÃ³ contenido
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

  if (m_gui.consumeNewSceneRequest()) {
    resetSceneToDefaults();
  }

  std::string openScenePath;
  if (m_gui.consumeOpenSceneRequest(openScenePath)) {
    if (!loadScene(openScenePath)) {
      ERROR("BaseApp", "OpenLevel", "No se pudo abrir el nivel seleccionado");
    }
  }

  LightType requestedLightType = LightType::Directional;
  if (m_gui.consumeCreateLightRequest(requestedLightType)) {
    std::string name = "Light";
    if (requestedLightType == LightType::Point)
      name = "PointLight";
    if (requestedLightType == LightType::Spot)
      name = "SpotLight";
    if (requestedLightType == LightType::Directional)
      name = "DirectionalLight";
    if (requestedLightType == LightType::Rect)
      name = "RectLight";

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
    } else if (!usingGizmo && m_prevGizmoUsing && m_gizmoEditing) {
      GizmoEditState after;
      if (captureGizmoState(m_gizmoEditActorIndex, after)) {
        const float eps = 1e-4f;
        bool changed = fabsf(after.position.x - m_gizmoBefore.position.x) > eps ||
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
    ImGuiIO &io = ImGui::GetIO();
    bool ctrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
    static bool zPrev = false, yPrev = false;
    bool zNow = (GetAsyncKeyState('Z') & 0x8000) != 0;
    bool yNow = (GetAsyncKeyState('Y') & 0x8000) != 0;

    bool menuUndo = m_gui.consumeUndoRequest();
    bool menuRedo = m_gui.consumeRedoRequest();
    bool doUndo = menuUndo || (ctrl && zNow && !zPrev && !io.WantTextInput);
    bool doRedo = menuRedo || (ctrl && yNow && !yPrev && !io.WantTextInput);

    if (doUndo) {
      m_commands.undo();
      MESSAGE("BaseApp", "undo", "Deshacer");
    }
    if (doRedo) {
      m_commands.redo();
      MESSAGE("BaseApp", "redo", "Rehacer");
    }

    zPrev = zNow;
    yPrev = yNow;
  }

  // --- Atajos Duplicar / Copiar / Pegar / Borrar ---
  {
    ImGuiIO &io = ImGui::GetIO();
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

    if (doCopy) {
      MESSAGE("BaseApp", "input", "Copy detectado");
      copySelected();
    }
    if (doDup) {
      MESSAGE("BaseApp", "input", "Duplicate detectado");
      duplicateSelected();
    }
    if (doPaste) {
      MESSAGE("BaseApp", "input", "Paste detectado");
      pasteClipboard();
    }
    if (doDel) {
      MESSAGE("BaseApp", "input", "Delete detectado");
      deleteSelected();
    }
    if (doSavePrefab)
      savePrefabSelected();
    if (doLoadPrefab)
      loadPrefab();

    dPrev = dNow;
    cPrev = cNow;
    vPrev = vNow;
    delPrev = delNow;
  }

  // --- Navegacion de camara ---
  if (!m_gui.m_isUsingGizmo) {
    ImGuiIO &io = ImGui::GetIO();
    if (m_gui.m_viewportHovered) {
      if (io.MouseWheel != 0.0f && !m_isPlaying) {
        m_camera.walk(io.MouseWheel * 0.7f);
      }

      const bool rightHeld = ImGui::IsMouseDown(ImGuiMouseButton_Right);

      if (rightHeld) {
        // Rotar camara con mouse
        m_camera.yaw(io.MouseDelta.x * 0.004f);
        m_camera.pitch(io.MouseDelta.y * 0.004f);
      }

      if (m_isPlaying || rightHeld) {
        const float speed =
            m_isPlaying ? ((GetAsyncKeyState(VK_SHIFT) & 0x8000) ? deltaTime * 20.0f
                                                                 : deltaTime * 7.0f)
                        : ((GetAsyncKeyState(VK_SHIFT) & 0x8000) ? 0.15f : 0.05f);
        if (GetAsyncKeyState('W') & 0x8000)
          m_camera.walk(speed);
        if (GetAsyncKeyState('S') & 0x8000)
          m_camera.walk(-speed);
        if (GetAsyncKeyState('A') & 0x8000)
          m_camera.strafe(-speed);
        if (GetAsyncKeyState('D') & 0x8000)
          m_camera.strafe(speed);
        if (GetAsyncKeyState('E') & 0x8000) {
          EU::Vector3 p = m_camera.getPosition();
          p.z += speed;
          m_camera.setPosition(p);
        }
        if (GetAsyncKeyState('Q') & 0x8000) {
          EU::Vector3 p = m_camera.getPosition();
          p.z -= speed;
          m_camera.setPosition(p);
        }
      }

      if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
        m_camera.strafe(-io.MouseDelta.x * 0.02f);
        EU::Vector3 p = m_camera.getPosition();
        p.z += io.MouseDelta.y * 0.02f;
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
    if (m_gui.m_viewportHovered && fNow && !fDown)
      focusCameraOnActor(selected);
    fDown = fNow;
    if (m_gui.consumeFocusRequest())
      focusCameraOnActor(selected);
    if (m_gui.consumeFitRequest())
      fitCameraToScene();
  }

  // --- Picking (click izquierdo) ---
  if (m_gui.m_viewportHovered && !m_gui.m_isUsingGizmo &&
      ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver()) {
    pickActorFromMouse();
  }

  // Resize estable del viewport
  unsigned int desiredW = static_cast<unsigned int>(m_gui.m_viewportSize.x);
  unsigned int desiredH = static_cast<unsigned int>(m_gui.m_viewportSize.y);
  const unsigned int kMinViewportSize = 64;
  if (desiredW < kMinViewportSize)
    desiredW = kMinViewportSize;
  if (desiredH < kMinViewportSize)
    desiredH = kMinViewportSize;
  if (desiredW != m_lastRequestedViewportWidth ||
      desiredH != m_lastRequestedViewportHeight) {
    m_lastRequestedViewportWidth = desiredW;
    m_lastRequestedViewportHeight = desiredH;
    m_viewportResizeStableFrames = 0;
  } else {
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
  XMStoreFloat4x4(&m_constantBufferStruct.Projection,
                  XMMatrixTranspose(m_camera.getProj()));
  m_constantBufferStruct.CameraPos = m_camera.getPosition();

  syncLightActors();

  m_skybox.update(m_deviceContext, m_camera);
  m_constantBuffer.update(
      m_deviceContext, nullptr, 0, nullptr, &m_constantBufferStruct, 0, 0);
  m_sceneGraph.update(deltaTime, m_deviceContext);
}

void
BaseApp::render() {
  float ClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

  m_deviceContext.m_drawCallCount = 0;

  m_renderScene.clear();
  m_sceneGraph.gatherRenderScene(m_renderScene, m_camera);
  std::vector<RenderObject> candidates = m_renderScene.opaqueObjects;
  candidates.insert(candidates.end(),
                    m_renderScene.transparentObjects.begin(),
                    m_renderScene.transparentObjects.end());
  for (RenderObject &object : candidates)
    updateWorldBounds(object);
  std::vector<RenderObject> visibleObjects;
  visibleObjects.reserve(candidates.size());
  m_sceneOctree.build(candidates);
  m_sceneOctree.queryVisible(m_camera, visibleObjects, m_cullingStats);
  m_renderScene.opaqueObjects.clear();
  m_renderScene.transparentObjects.clear();
  for (const RenderObject &object : visibleObjects) {
    (object.transparent ? m_renderScene.transparentObjects : m_renderScene.opaqueObjects)
        .push_back(object);
  }
  m_renderScene.skybox = &m_skybox;
  if (!m_isPlaying) {
    addEditorGridToRenderScene();
    addEditorSelectionToRenderScene();
  }

  m_renderPipeline.setShadowFactorDebugEnabled(m_gui.m_visualizeDeferredShadowFactor);
  m_renderPipeline.render(m_deviceContext, m_camera, m_renderScene, m_editorViewportPass);
  for (const EU::TSharedPointer<Actor> &actor : m_actors) {
    if (actor.isNull())
      continue;
    EU::TSharedPointer<ParticleEmitterComponent> emitter =
        actor->getComponent<ParticleEmitterComponent>();
    if (emitter)
      emitter->renderParticles(m_deviceContext, m_camera, m_editorViewportPass);
  }
  if (m_gui.consumeCreateParticleEmitterRequest()) {
    EU::TSharedPointer<Actor> emitterActor = createParticleEmitterActor();
    if (!emitterActor.isNull()) {
      m_commands.push(
          std::unique_ptr<ICommand>(new SpawnActorCommand(this, emitterActor)));
    }
  }
  m_lastDrawCalls = m_deviceContext.m_drawCallCount;

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
  for (const EU::TSharedPointer<Actor> &actor : m_actors) {
    if (actor.isNull())
      continue;
    EU::TSharedPointer<ParticleEmitterComponent> emitter =
        actor->getComponent<ParticleEmitterComponent>();
    if (emitter)
      emitter->destroy();
  }
  m_audioSystem.destroy();
  m_renderPipeline.destroy();
  m_editorViewportPass.destroy();
  m_cyberGunRenderMesh.destroy();
  m_editorGridMesh.destroy();
  m_editorSelectionMesh.destroy();
  m_AlbedoSRV.destroy();
  m_MetallicSRV.destroy();
  m_NormalSRV.destroy();
  m_RoughnessSRV.destroy();
  m_AOSRV.destroy();
  m_EmissiveSRV.destroy();
  m_defaultWhiteSRV.destroy();
  m_defaultBlackSRV.destroy();
  m_defaultFlatNormalSRV.destroy();
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
  for (auto &lm : m_loadedModels) {
    if (lm) {
      lm->mesh.destroy();
      for (auto &tex : lm->externalTextures) {
        if (tex)
          tex->destroy();
      }
    }
  }
  m_loadedModels.clear();
  for (auto &tex : m_thumbTextures)
    tex.destroy();
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
    CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
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
    if (wParam == SIZE_MINIMIZED)
      return 0;
    UINT newW = LOWORD(lParam);
    UINT newH = HIWORD(lParam);
    if (newW == 0 || newH == 0)
      return 0;
    BaseApp *app = reinterpret_cast<BaseApp *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (app)
      app->onResize(newW, newH);
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
  if (!m_deviceContext.m_deviceContext || !m_swapChain.m_swapChain)
    return;
  if (newW == 0 || newH == 0)
    return;

  m_window.m_width = (int)newW;
  m_window.m_height = (int)newH;

  ID3D11RenderTargetView *nullRTV = nullptr;
  m_deviceContext.m_deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);

  m_renderTargetView.destroy();
  m_depthStencilView.destroy();
  m_depthStencil.destroy();
  m_backBuffer.destroy();

  HRESULT hr = m_swapChain.resizeBuffers(newW, newH);
  if (FAILED(hr))
    return;
  hr = m_swapChain.getBackBuffer(m_backBuffer);
  if (FAILED(hr))
    return;
  hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
  if (FAILED(hr))
    return;
  hr = m_depthStencil.init(m_device,
                           newW,
                           newH,
                           DXGI_FORMAT_D24_UNORM_S8_UINT,
                           D3D11_BIND_DEPTH_STENCIL,
                           4,
                           0);
  if (FAILED(hr))
    return;
  hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
  if (FAILED(hr))
    return;

  m_viewport.init(m_window);
  m_camera.setLens(XM_PIDIV4, newW / (float)newH, 0.01f, 100.0f);
}

void
BaseApp::handleEditorViewportResize() {
  if (!m_editorViewportResizePending)
    return;

  m_deviceContext.m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
  ID3D11ShaderResourceView *nullSRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
  m_deviceContext.m_deviceContext->PSSetShaderResources(
      0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullSRVs);

  EditorViewportPass newPass;
  HRESULT hr = newPass.init(m_device, m_pendingViewportWidth, m_pendingViewportHeight);
  if (FAILED(hr)) {
    m_editorViewportResizePending = false;
    return;
  }

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
  for (auto &actor : m_actors) {
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
    if (m_actors[i].isNull())
      continue;
    EU::TSharedPointer<Transform> t = m_actors[i]->getComponent<Transform>();
    if (t) {
      t->setPosition(m_initialTransforms[i].position);
      t->setRotation(m_initialTransforms[i].rotation);
      t->setScale(m_initialTransforms[i].scale);
    }
  }
  if (!m_directionalLightActor.isNull()) {
    EU::TSharedPointer<LightComponent> lc =
        m_directionalLightActor->getComponent<LightComponent>();
    if (lc) {
      lc->getLightData().direction = m_initialLightDir;
      lc->getLightData().color = m_initialLightColor;
    }
  }
  MESSAGE("BaseApp", "resetSceneToDefaults", "Escena restaurada");
}

void
BaseApp::captureRuntimeState() {
  m_runtimeCameraPosition = m_camera.getPosition();
  m_runtimeCameraForward = m_camera.GetForward();
  m_runtimeCameraUp = m_camera.GetUp();
  m_runtimeTransforms.clear();
  m_runtimeTransforms.reserve(m_actors.size());

  for (const EU::TSharedPointer<Actor> &actor : m_actors) {
    if (actor.isNull()) {
      continue;
    }

    EU::TSharedPointer<Transform> transform = actor->getComponent<Transform>();
    if (transform.isNull()) {
      continue;
    }

    InitialTransform savedTransform{};
    savedTransform.position = transform->getPosition();
    savedTransform.rotation = transform->getRotation();
    savedTransform.scale = transform->getScale();
    m_runtimeTransforms.push_back(savedTransform);
  }
}

void
BaseApp::startPlayMode() {
  if (m_isPlaying) {
    return;
  }

  captureRuntimeState();
  setRuntimeBehaviorsRunning(true);
  setRuntimeAudioPlaying(true);
  m_isPlaying = true;
  m_gui.setRuntimePlaying(true);
  MESSAGE("BaseApp", "startPlayMode", "Modo Play iniciado");
}

void
BaseApp::stopPlayMode() {
  if (!m_isPlaying) {
    return;
  }

  setRuntimeBehaviorsRunning(false);
  setRuntimeAudioPlaying(false);
  m_camera.setPosition(
      m_runtimeCameraPosition.x, m_runtimeCameraPosition.y, m_runtimeCameraPosition.z);
  m_camera.lookAt(m_runtimeCameraPosition,
                  EU::Vector3(m_runtimeCameraPosition.x + m_runtimeCameraForward.x,
                              m_runtimeCameraPosition.y + m_runtimeCameraForward.y,
                              m_runtimeCameraPosition.z + m_runtimeCameraForward.z),
                  m_runtimeCameraUp);

  const size_t restoreCount = (std::min)(m_actors.size(), m_runtimeTransforms.size());
  for (size_t index = 0; index < restoreCount; ++index) {
    const EU::TSharedPointer<Actor> &actor = m_actors[index];
    if (actor.isNull()) {
      continue;
    }

    EU::TSharedPointer<Transform> transform = actor->getComponent<Transform>();
    if (transform.isNull()) {
      continue;
    }

    const InitialTransform &savedTransform = m_runtimeTransforms[index];
    transform->setTransform(
        savedTransform.position, savedTransform.rotation, savedTransform.scale);
    transform->rebuildMatrixFromVectors();
  }

  m_runtimeTransforms.clear();
  m_isPlaying = false;
  m_gui.setRuntimePlaying(false);
  MESSAGE("BaseApp", "stopPlayMode", "Modo Play detenido y escena restaurada");
}

void
BaseApp::setRuntimeBehaviorsRunning(bool running) {
  for (const EU::TSharedPointer<Actor> &actor : m_actors) {
    if (actor.isNull()) {
      continue;
    }

    EU::TSharedPointer<RuntimeBehaviorComponent> behavior =
        actor->getComponent<RuntimeBehaviorComponent>();
    if (!behavior.isNull()) {
      behavior->setRunning(running);
    }
  }
}

void
BaseApp::setRuntimeAudioPlaying(bool playing) {
  for (const EU::TSharedPointer<Actor> &actor : m_actors) {
    if (actor.isNull()) {
      continue;
    }

    EU::TSharedPointer<AudioSourceComponent> audioSource =
        actor->getComponent<AudioSourceComponent>();
    if (audioSource.isNull() || audioSource->getAudioPath().empty()) {
      continue;
    }

    if (playing) {
      if (audioSource->isAutoActivate()) {
        audioSource->play();
      }
    } else {
      audioSource->stop();
    }
  }

  if (!playing) {
    m_audioSystem.stopAll();
    m_gui.setAudioPaused(false);
  }
}

void
BaseApp::focusCameraOnActor(const EU::TSharedPointer<Actor> &actor) {
  if (actor.isNull())
    return;
  EU::TSharedPointer<Transform> t = actor->getComponent<Transform>();
  if (!t)
    return;
  EU::Vector3 target = t->getPosition();
  EU::Vector3 fwd = m_camera.GetForward();
  const float dist = 6.0f;
  EU::Vector3 eye(
      target.x - fwd.x * dist, target.y - fwd.y * dist, target.z - fwd.z * dist);
  m_camera.lookAt(eye, target);
  m_camera.setPosition(eye);
}

void
BaseApp::fitCameraToScene() {
  float minX = 1e9f, minY = 1e9f, minZ = 1e9f;
  float maxX = -1e9f, maxY = -1e9f, maxZ = -1e9f;
  int count = 0;
  for (auto &a : m_actors) {
    if (a.isNull())
      continue;
    if (a->getComponent<MeshRendererComponent>().isNull())
      continue;
    EU::TSharedPointer<Transform> t = a->getComponent<Transform>();
    if (!t)
      continue;
    EU::Vector3 p = t->getPosition();
    minX = fminf(minX, p.x);
    minY = fminf(minY, p.y);
    minZ = fminf(minZ, p.z);
    maxX = fmaxf(maxX, p.x);
    maxY = fmaxf(maxY, p.y);
    maxZ = fmaxf(maxZ, p.z);
    ++count;
  }
  if (count == 0)
    return;
  EU::Vector3 center((minX + maxX) * 0.5f, (minY + maxY) * 0.5f, (minZ + maxZ) * 0.5f);
  float dx = maxX - minX, dy = maxY - minY, dz = maxZ - minZ;
  float radius = 0.5f * sqrtf(dx * dx + dy * dy + dz * dz) + 3.0f;
  float dist = radius / tanf(m_camera.getFovY() * 0.5f);
  EU::Vector3 fwd = m_camera.GetForward();
  EU::Vector3 eye(
      center.x - fwd.x * dist, center.y - fwd.y * dist, center.z - fwd.z * dist);
  m_camera.lookAt(eye, center);
  m_camera.setPosition(eye);
}

bool
BaseApp::captureGizmoState(int index, GizmoEditState &out) {
  if (index < 0 || index >= (int)m_actors.size())
    return false;
  if (m_actors[index].isNull())
    return false;
  EU::TSharedPointer<Transform> t = m_actors[index]->getComponent<Transform>();
  if (!t)
    return false;
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
  if (vpW < 1.0f || vpH < 1.0f)
    return;

  ImVec2 mouse = ImGui::GetIO().MousePos;
  float mx = mouse.x - vpX;
  float my = mouse.y - vpY;
  if (mx < 0.0f || my < 0.0f || mx > vpW || my > vpH)
    return;

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
    if (m_actors[i].isNull())
      continue;
    EU::TSharedPointer<MeshRendererComponent> mr =
        m_actors[i]->getComponent<MeshRendererComponent>();
    if (!mr || !mr->isVisible())
      continue;
    EU::TSharedPointer<Transform> t = m_actors[i]->getComponent<Transform>();
    if (!t)
      continue;

    EU::Vector3 lmn, lmx;
    if (!getActorAABB(m_actors[i], lmn, lmx))
      continue;

    XMMATRIX world = t->worldMatrix;
    float bmin[3] = {1e9f, 1e9f, 1e9f};
    float bmax[3] = {-1e9f, -1e9f, -1e9f};
    for (int c = 0; c < 8; ++c) {
      float cx = (c & 1) ? lmx.x : lmn.x;
      float cy = (c & 2) ? lmx.y : lmn.y;
      float cz = (c & 4) ? lmx.z : lmn.z;
      XMVECTOR wc = XMVector3TransformCoord(XMVectorSet(cx, cy, cz, 1.0f), world);
      XMFLOAT3 f;
      XMStoreFloat3(&f, wc);
      bmin[0] = fminf(bmin[0], f.x);
      bmin[1] = fminf(bmin[1], f.y);
      bmin[2] = fminf(bmin[2], f.z);
      bmax[0] = fmaxf(bmax[0], f.x);
      bmax[1] = fmaxf(bmax[1], f.y);
      bmax[2] = fmaxf(bmax[2], f.z);
    }

    float ro[3] = {o.x, o.y, o.z};
    float rd[3] = {d.x, d.y, d.z};
    float tmin = 0.0f, tmax = 1e9f;
    bool hit = true;
    for (int a = 0; a < 3; ++a) {
      if (fabsf(rd[a]) < 1e-8f) {
        if (ro[a] < bmin[a] || ro[a] > bmax[a]) {
          hit = false;
          break;
        }
      } else {
        float inv = 1.0f / rd[a];
        float t1 = (bmin[a] - ro[a]) * inv;
        float t2 = (bmax[a] - ro[a]) * inv;
        if (t1 > t2) {
          float tmp = t1;
          t1 = t2;
          t2 = tmp;
        }
        tmin = fmaxf(tmin, t1);
        tmax = fminf(tmax, t2);
        if (tmin > tmax) {
          hit = false;
          break;
        }
      }
    }
    if (hit && tmin < bestT) {
      bestT = tmin;
      bestIndex = i;
    }
  }

  if (bestIndex >= 0)
    m_gui.selectedActorIndex = bestIndex;
}

bool
BaseApp::getViewportMouseGroundPosition(EU::Vector3 &outPosition) const {
  const float vpX = m_gui.m_viewportPos.x;
  const float vpY = m_gui.m_viewportPos.y;
  const float vpW = m_gui.m_viewportSize.x;
  const float vpH = m_gui.m_viewportSize.y;
  if (vpW < 1.0f || vpH < 1.0f)
    return false;

  const ImVec2 mouse = ImGui::GetIO().MousePos;
  const float mx = mouse.x - vpX;
  const float my = mouse.y - vpY;
  if (mx < 0.0f || my < 0.0f || mx > vpW || my > vpH)
    return false;

  const float ndcX = (2.0f * mx / vpW) - 1.0f;
  const float ndcY = 1.0f - (2.0f * my / vpH);

  XMMATRIX view = m_camera.getView();
  XMMATRIX proj = m_camera.getProj();
  XMVECTOR det;
  XMMATRIX invVP = XMMatrixInverse(&det, view * proj);
  XMVECTOR nearP = XMVector3TransformCoord(XMVectorSet(ndcX, ndcY, 0.0f, 1.0f), invVP);
  XMVECTOR farP = XMVector3TransformCoord(XMVectorSet(ndcX, ndcY, 1.0f, 1.0f), invVP);

  XMFLOAT3 origin;
  XMFLOAT3 direction;
  XMStoreFloat3(&origin, nearP);
  XMStoreFloat3(&direction, XMVector3Normalize(XMVectorSubtract(farP, nearP)));

  if (fabsf(direction.z) < 1e-5f)
    return false;
  const float hitT = -origin.z / direction.z;
  if (hitT < 0.0f)
    return false;
  if (hitT > 80.0f)
    return false;

  outPosition =
      EU::Vector3(origin.x + direction.x * hitT, origin.y + direction.y * hitT, 0.0f);
  return true;
}

void
BaseApp::addActorToScene(const EU::TSharedPointer<Actor> &actor) {
  if (actor.isNull())
    return;
  m_actors.push_back(actor);
  m_sceneGraph.addEntity(actor.get());
}

void
BaseApp::removeActorFromScene(const EU::TSharedPointer<Actor> &actor) {
  if (actor.isNull())
    return;
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
BaseApp::spawnPistol(const std::string &name,
                     const EU::Vector3 &pos,
                     const EU::Vector3 &rot,
                     const EU::Vector3 &scale) {
  EU::TSharedPointer<Actor> a = EU::MakeShared<Actor>(m_device);
  if (a.isNull())
    return a;
  a->setName(name);
  EU::TSharedPointer<Transform> t = a->getComponent<Transform>();
  if (t)
    t->setTransform(pos, rot, scale);
  EU::TSharedPointer<MeshRendererComponent> mr = a->getComponent<MeshRendererComponent>();
  if (!mr) {
    mr = EU::MakeShared<MeshRendererComponent>();
    a->addComponent(mr);
  }
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
  if (nt)
    nt->setTransform(pos, rot, sca);

  EU::TSharedPointer<MeshRendererComponent> srcMr =
      src->getComponent<MeshRendererComponent>();
  if (srcMr) {
    EU::TSharedPointer<MeshRendererComponent> mr =
        EU::MakeShared<MeshRendererComponent>();
    a->addComponent(mr);
    mr->setMesh(srcMr->getMesh());
    if (srcMr->getMaterialInstance()) {
      MaterialInstance *newMat = new MaterialInstance(*srcMr->getMaterialInstance());
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

  EU::TSharedPointer<MeshRendererComponent> srcMr =
      src->getComponent<MeshRendererComponent>();
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
  if (nt)
    nt->setTransform(pos, m_clipboard.rotation, m_clipboard.scale);

  if (m_clipboard.hasMesh) {
    EU::TSharedPointer<MeshRendererComponent> mr =
        EU::MakeShared<MeshRendererComponent>();
    a->addComponent(mr);
    mr->setMesh(m_clipboard.mesh);
    MaterialInstance *newMat = new MaterialInstance(m_clipboard.materialInstance);
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
  if (idx < 0 || idx >= (int)m_actors.size() || m_actors[idx].isNull())
    return;
  EU::TSharedPointer<Actor> src = m_actors[idx];
  EU::TSharedPointer<Transform> t = src->getComponent<Transform>();
  if (!t)
    return;
  CreateDirectoryA("Saved", nullptr);
  std::ofstream f("Saved/actor.prefab", std::ios::binary | std::ios::trunc);
  if (!f.is_open()) {
    ERROR("BaseApp", "savePrefab", "No se pudo abrir el archivo");
    return;
  }
  EU::Vector3 p = t->getPosition(), r = t->getRotation(), s = t->getScale();
  std::string n = src->getName();
  const bool ok = writeBinaryValue(f, kPrefabBinaryMagic) &&
                  writeBinaryValue(f, kPrefabBinaryVersion) && writeBinaryString(f, n) &&
                  writeBinaryVector3(f, p) && writeBinaryVector3(f, r) &&
                  writeBinaryVector3(f, s);
  if (!ok) {
    ERROR("BaseApp", "savePrefab", "No se pudo escribir el prefab binario");
    return;
  }
  MESSAGE("BaseApp", "savePrefab", "Prefab binario guardado en Saved/actor.prefab");
}

void
BaseApp::loadPrefab() {
  std::ifstream f("Saved/actor.prefab", std::ios::binary);
  if (!f.is_open()) {
    ERROR("BaseApp", "loadPrefab", "No existe Saved/actor.prefab");
    return;
  }
  std::string name = "Prefab";
  EU::Vector3 p(0, 0, 0), r(0, 0, 0), s(1, 1, 1);
  uint32_t magic = 0;
  uint32_t version = 0;
  const bool ok = readBinaryValue(f, magic) && readBinaryValue(f, version) &&
                  magic == kPrefabBinaryMagic && version == kPrefabBinaryVersion &&
                  readBinaryString(f, name) && readBinaryVector3(f, p) &&
                  readBinaryVector3(f, r) && readBinaryVector3(f, s);
  if (!ok) {
    ERROR("BaseApp", "loadPrefab", "Prefab binario invalido");
    return;
  }
  EU::TSharedPointer<Actor> a = spawnPistol(name, p, r, s);
  addActorToScene(a);
  m_commands.push(std::unique_ptr<ICommand>(new SpawnActorCommand(this, a)));
  m_gui.selectedActorIndex = (int)m_actors.size() - 1;
  MESSAGE("BaseApp", "loadPrefab", "Prefab binario cargado");
}

void
BaseApp::buildTextureThumbnails() {
  m_thumbTextures.clear();
  m_thumbnails.clear();
  m_thumbTextures.reserve(64);

  std::vector<std::string> folders = listSubfolders("Assets/Textures");
  folders.push_back("");
  int loaded = 0;
  for (const std::string &sub : folders) {
    std::string dir = sub.empty() ? "Assets/Textures" : ("Assets/Textures/" + sub);
    std::vector<std::string> files = listImageFiles(dir);
    for (const std::string &f : files) {
      if (loaded >= 48)
        break;
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
BaseApp::loadModelTextures(LoadedModel &lm, const std::string &folder) {
  if (lm.materialInstances.empty())
    return;

  struct

      TextureCandidate {
    std::string directory;
    std::string fileName;
  };

  auto detectSlot = [](const std::string &lower,
                       const std::string &baseNoExt,
                       bool singleTexture) -> int {
    if (containsStr(lower, "normal") || endsWith(baseNoExt, "_n") ||
        endsWith(baseNoExt, "_nrm"))
      return 1;
    if (containsStr(lower, "metallic") || containsStr(lower, "metalness") ||
        endsWith(baseNoExt, "_m") || endsWith(baseNoExt, "_met"))
      return 2;
    if (containsStr(lower, "roughness") || endsWith(baseNoExt, "_r") ||
        endsWith(baseNoExt, "_rgh"))
      return 3;
    if (containsStr(lower, "occlusion") || containsStr(lower, "ambientocclusion") ||
        endsWith(baseNoExt, "_ao") || endsWith(baseNoExt, "_o"))
      return 4;
    if (containsStr(lower, "emissive") || containsStr(lower, "emission"))
      return 5;
    if (containsStr(lower, "basecolor") || containsStr(lower, "albedo") ||
        containsStr(lower, "diffuse") || containsStr(lower, "base") ||
        endsWith(baseNoExt, "_bc") || endsWith(baseNoExt, "_d") ||
        endsWith(baseNoExt, "_alb") || singleTexture) {
      return 0;
    }
    return -1;
  };

  auto applyTexture = [](MaterialInstance *material, int slot, Texture *texture) {
    if (!material || !texture)
      return;
    switch (slot) {
    case 0:
      material->setAlbedo(texture);
      break;
    case 1:
      material->setNormal(texture);
      break;
    case 2:
      material->setMetallic(texture);
      break;
    case 3:
      material->setRoughness(texture);
      break;
    case 4:
      material->setAO(texture);
      break;
    case 5:
      material->setEmissive(texture);
      break;
    default:
      break;
    }
  };

  std::vector<TextureCandidate> candidates;
  const std::vector<std::string> folderFiles = listImageFiles(folder);
  for (const std::string &fileName : folderFiles) {
    candidates.push_back(TextureCandidate{folder, fileName});
  }

  if (candidates.empty()) {
    const std::string modelName = toLowerCopy(fileBaseName(folder));
    const std::string rootDir = "Assets/Textures";
    const std::vector<std::string> rootFiles = listImageFiles(rootDir);
    for (const std::string &fileName : rootFiles) {
      if (toLowerCopy(stripExt(fileName)) == modelName) {
        candidates.push_back(TextureCandidate{rootDir, fileName});
      }
    }
  }

  if (candidates.empty()) {
    MESSAGE("BaseApp", "loadModelTextures", "Sin texturas externas; usando defaults PBR");
    return;
  }

  const bool singleTexture = candidates.size() == 1;
  for (const TextureCandidate &candidate : candidates) {
    const std::string lower = toLowerCopy(candidate.fileName);
    const std::string baseNoExt = toLowerCopy(stripExt(candidate.fileName));
    const std::string path = candidate.directory + "/" + candidate.fileName;

    const int slot = detectSlot(lower, baseNoExt, singleTexture);

    HRESULT hr;
    auto tex = std::make_unique<Texture>();
    hr = tex->initFromFile(m_device, path);
    if (SUCCEEDED(hr)) {
      if (slot >= 0) {
        for (auto &materialInstance : lm.materialInstances) {
          applyTexture(materialInstance.get(), slot, tex.get());
        }
      }
      lm.externalTextures.push_back(std::move(tex));
    }
  }
}

EU::TSharedPointer<Actor>
BaseApp::loadModelActor(const std::string &modelPath, const EU::Vector3 &spawnPosition) {
  std::string lower = toLowerCopy(modelPath);
  ModelType type = FBX;
  if (endsWith(lower, ".obj"))
    type = OBJ;
  else if (endsWith(lower, ".glb") || endsWith(lower, ".gltf"))
    type = GLTF;

  Model3D model(modelPath, type);
  const std::vector<MeshComponent> &meshes = model.GetMeshes();
  if (meshes.empty()) {
    ERROR("BaseApp", "loadModelActor", "El modelo no tiene mallas");
    return EU::TSharedPointer<Actor>();
  }

  std::vector<MeshComponent> renderMeshes = meshes;
  if (type == FBX) {
    EU::Vector3 rawMin(1e9f, 1e9f, 1e9f);
    EU::Vector3 rawMax(-1e9f, -1e9f, -1e9f);
    bool hasVertex = false;
    for (const MeshComponent &mc : renderMeshes) {
      for (const SimpleVertex &v : mc.m_vertex) {
        rawMin.x = fminf(rawMin.x, v.Position.x);
        rawMin.y = fminf(rawMin.y, v.Position.y);
        rawMin.z = fminf(rawMin.z, v.Position.z);
        rawMax.x = fmaxf(rawMax.x, v.Position.x);
        rawMax.y = fmaxf(rawMax.y, v.Position.y);
        rawMax.z = fmaxf(rawMax.z, v.Position.z);
        hasVertex = true;
      }
    }
    if (hasVertex) {
      const EU::Vector3 pivotOffset(
          (rawMin.x + rawMax.x) * 0.5f, (rawMin.y + rawMax.y) * 0.5f, rawMin.z);
      for (MeshComponent &mc : renderMeshes) {
        for (SimpleVertex &v : mc.m_vertex) {
          v.Position.x -= pivotOffset.x;
          v.Position.y -= pivotOffset.y;
          v.Position.z -= pivotOffset.z;
        }
      }
    }
  }

  std::unique_ptr<LoadedModel> lm(new LoadedModel());

  HRESULT hr;
  int numMaterials = 1;
  for (const MeshComponent &mc : renderMeshes) {
    if (mc.m_vertex.empty() || mc.m_index.empty() || mc.m_numIndex <= 0) {
      continue;
    }
    Submesh sm{};
    hr = sm.vertexBuffer.init(m_device, mc, D3D11_BIND_VERTEX_BUFFER);
    if (FAILED(hr)) {
      ERROR("BaseApp", "loadModelActor", "Fallo vertex buffer");
      return EU::TSharedPointer<Actor>();
    }
    hr = sm.indexBuffer.init(m_device, mc, D3D11_BIND_INDEX_BUFFER);
    if (FAILED(hr)) {
      ERROR("BaseApp", "loadModelActor", "Fallo index buffer");
      return EU::TSharedPointer<Actor>();
    }
    sm.indexCount = mc.m_numIndex;
    sm.startIndex = 0;
    sm.materialSlot = mc.m_materialIndex;
    sm.localTransform = mc.m_localTransform;
    lm->mesh.getSubmeshes().push_back(std::move(sm));

    if (mc.m_materialIndex >= numMaterials)
      numMaterials = mc.m_materialIndex + 1;
  }
  if (lm->mesh.getSubmeshes().empty()) {
    ERROR("BaseApp", "loadModelActor", "El modelo no genero submallas renderizables");
    return EU::TSharedPointer<Actor>();
  }

  lm->localMin = EU::Vector3(1e9f, 1e9f, 1e9f);
  lm->localMax = EU::Vector3(-1e9f, -1e9f, -1e9f);
  for (const MeshComponent &mc : renderMeshes) {
    XMMATRIX localMat = XMLoadFloat4x4(&mc.m_localTransform);
    for (const SimpleVertex &v : mc.m_vertex) {
      XMVECTOR p = XMVector3TransformCoord(
          XMVectorSet(v.Position.x, v.Position.y, v.Position.z, 1.0f), localMat);
      XMFLOAT3 fpos;
      XMStoreFloat3(&fpos, p);
      lm->localMin.x = fminf(lm->localMin.x, fpos.x);
      lm->localMin.y = fminf(lm->localMin.y, fpos.y);
      lm->localMin.z = fminf(lm->localMin.z, fpos.z);
      lm->localMax.x = fmaxf(lm->localMax.x, fpos.x);
      lm->localMax.y = fmaxf(lm->localMax.y, fpos.y);
      lm->localMax.z = fmaxf(lm->localMax.z, fpos.z);
    }
  }

  for (int i = 0; i < numMaterials; ++i) {
    auto mat = std::make_unique<Material>();
    mat->setShader(&m_shaderProgram);
    mat->setRasterizerState(&m_defaultRasterizer);
    mat->setDepthStencilState(&m_defaultDepthStencil);
    mat->setSamplerState(&m_defaultSampler);
    mat->setDomain(MaterialDomain::Opaque);
    mat->setBlendMode(BlendMode::Opaque);

    auto inst = std::make_unique<MaterialInstance>();
    inst->setMaterial(mat.get());
    inst->setAlbedo(&m_defaultWhiteSRV);
    inst->setNormal(&m_defaultFlatNormalSRV);
    inst->setMetallic(&m_defaultBlackSRV);
    inst->setRoughness(&m_defaultWhiteSRV);
    inst->setAO(&m_defaultWhiteSRV);
    inst->setEmissive(&m_defaultBlackSRV);
    inst->getParams().baseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    inst->getParams().metallic = 0.0f;
    inst->getParams().roughness = 1.0f;
    inst->getParams().ao = 1.0f;
    inst->getParams().normalScale = 1.0f;
    inst->getParams().emissiveStrength = 1.0f;
    inst->getParams().alphaCutoff = 0.5f;

    lm->materials.push_back(std::move(mat));
    lm->materialInstances.push_back(std::move(inst));
  }

  const std::vector<ImportedMaterialInfo> &materialInfos = model.GetMaterialInfos();
  for (int i = 0; i < numMaterials && i < static_cast<int>(materialInfos.size()); ++i) {
    const ImportedMaterialInfo &info = materialInfos[i];
    Material *material = lm->materials[i].get();
    MaterialInstance *instance = lm->materialInstances[i].get();
    instance->getParams().baseColor = info.baseColor;
    instance->getParams().metallic = info.metallic;
    instance->getParams().roughness = info.roughness;
    instance->setMetallic(&m_defaultWhiteSRV);
    instance->setRoughness(&m_defaultWhiteSRV);
    instance->setAO(&m_defaultWhiteSRV);
    if (info.alphaBlend || info.baseColor.w < 0.999f) {
      material->setDomain(MaterialDomain::Transparent);
      material->setBlendMode(BlendMode::Alpha);
    }
  }
  lm->mesh.setLocalBounds(lm->localMin, lm->localMax);

  std::string modelName = fileBaseName(modelPath);
  const std::string modelDirectory = directoryOf(modelPath);
  const std::string localTextureDirectory = modelDirectory + "/Textures";
  loadModelTextures(*lm, localTextureDirectory);
  if (listImageFiles(localTextureDirectory).empty()) {
    loadModelTextures(*lm, "Assets/Textures/" + modelName);
  }

  EU::Vector3 actorScale(1.0f, 1.0f, 1.0f);
  if (type == FBX) {
    const float sizeX = lm->localMax.x - lm->localMin.x;
    const float sizeY = lm->localMax.y - lm->localMin.y;
    const float sizeZ = lm->localMax.z - lm->localMin.z;
    float maxSize = fmaxf(sizeX, fmaxf(sizeY, sizeZ));
    if (maxSize > 0.0001f) {
      const float targetMaxSize = 6.0f;
      if (maxSize > 12.0f || maxSize < 0.35f) {
        const float uniformScale = targetMaxSize / maxSize;
        actorScale = EU::Vector3(uniformScale, uniformScale, uniformScale);
      }
    }
  }

  auto resolveTexturePath = [&](const std::string &rawPath) -> std::string {
    if (rawPath.empty())
      return std::string();

    std::vector<std::string> candidates;
    candidates.push_back(rawPath);
    const std::string modelDir = directoryOf(modelPath);
    const std::string rawFile = fileNameOf(rawPath);
    if (!modelDir.empty()) {
      if (!isAbsolutePath(rawPath)) {
        candidates.push_back(modelDir + "/" + rawPath);
      }
      candidates.push_back(modelDir + "/" + rawFile);
      candidates.push_back(modelDir + "/textures/" + rawFile);
      candidates.push_back(modelDir + "/Textures/" + rawFile);
    }
    candidates.push_back("Assets/Textures/" + modelName + "/" + rawFile);
    candidates.push_back("Assets/Textures/" + rawFile);

    for (const std::string &candidate : candidates) {
      if (fileExists(candidate))
        return candidate;
    }
    return std::string();
  };

  auto applyTexture = [](MaterialInstance *material, int slot, Texture *texture) {
    if (!material || !texture)
      return;
    switch (slot) {
    case 0:
      material->setAlbedo(texture);
      break;
    case 1:
      material->setNormal(texture);
      break;
    case 2:
      material->setMetallic(texture);
      break;
    case 3:
      material->setRoughness(texture);
      break;
    case 4:
      material->setAO(texture);
      break;
    case 5:
      material->setEmissive(texture);
      break;
    default:
      break;
    }
  };

  for (int materialIndex = 0; materialIndex < numMaterials &&
                              materialIndex < static_cast<int>(materialInfos.size());
       ++materialIndex) {
    MaterialInstance *materialInstance = lm->materialInstances[materialIndex].get();
    for (int slot = 0; slot < 6; ++slot) {
      const std::string resolvedPath =
          resolveTexturePath(materialInfos[materialIndex].texturePaths[slot]);
      if (resolvedPath.empty())
        continue;

      auto texture = std::make_unique<Texture>();
      if (SUCCEEDED(texture->initFromFile(m_device, resolvedPath))) {
        applyTexture(materialInstance, slot, texture.get());
        lm->externalTextures.push_back(std::move(texture));
      }
    }
  }

  if (type == GLTF) {
    const auto &embTextures = model.GetEmbeddedTextures();
    for (const auto &emb : embTextures) {
      int targetMat = (emb.materialIndex >= 0 && emb.materialIndex < numMaterials)
                          ? emb.materialIndex
                          : 0;
      MaterialInstance *mInst = lm->materialInstances[targetMat].get();

      if (emb.textureSlot == 6) {
        auto metallicTex = std::make_unique<Texture>();
        auto roughnessTex = std::make_unique<Texture>();
        HRESULT hrMetallic = metallicTex->initSingleChannelFromMemory(
            m_device, emb.data.data(), emb.data.size(), 2, emb.name + "_metallic");
        HRESULT hrRoughness = roughnessTex->initSingleChannelFromMemory(
            m_device, emb.data.data(), emb.data.size(), 1, emb.name + "_roughness");

        if (SUCCEEDED(hrMetallic)) {
          mInst->setMetallic(metallicTex.get());
          lm->externalTextures.push_back(std::move(metallicTex));
        }
        if (SUCCEEDED(hrRoughness)) {
          mInst->setRoughness(roughnessTex.get());
          lm->externalTextures.push_back(std::move(roughnessTex));
        }
        continue;
      }

      auto tex = std::make_unique<Texture>();
      HRESULT hrTex =
          tex->initFromMemory(m_device, emb.data.data(), emb.data.size(), emb.name);
      if (SUCCEEDED(hrTex)) {
        switch (emb.textureSlot) {
        case 0:
          mInst->setAlbedo(tex.get());
          break;
        case 1:
          mInst->setNormal(tex.get());
          break;
        case 2:
          mInst->setMetallic(tex.get());
          break;
        case 3:
          mInst->setRoughness(tex.get());
          break;
        case 4:
          mInst->setAO(tex.get());
          break;
        case 5:
          mInst->setEmissive(tex.get());
          break;
        default:
          mInst->setAlbedo(tex.get());
          break;
        }
        lm->externalTextures.push_back(std::move(tex));
      }
    }
  }

  EU::TSharedPointer<Actor> a = EU::MakeShared<Actor>(m_device);
  if (a.isNull())
    return a;
  a->setName(modelName);
  EU::TSharedPointer<Transform> t = a->getComponent<Transform>();
  if (t) {
    t->setTransform(spawnPosition, EU::Vector3(0.0f, 0.0f, 0.0f), actorScale);
  }
  EU::TSharedPointer<AudioSourceComponent> audioSource =
      EU::MakeShared<AudioSourceComponent>(t.get());
  a->addComponent(audioSource);
  m_audioSystem.registerSource(audioSource.get());
  EU::TSharedPointer<MeshRendererComponent> mr = a->getComponent<MeshRendererComponent>();
  if (!mr) {
    mr = EU::MakeShared<MeshRendererComponent>();
    a->addComponent(mr);
  }
  if (mr) {
    mr->setMesh(&lm->mesh);
    std::vector<MaterialInstance *> insts;
    for (auto &inst : lm->materialInstances)
      insts.push_back(inst.get());
    mr->setMaterialInstances(insts);
  }
  mr->setVisible(true);
  mr->setCastShadow(true);

  m_loadedModels.push_back(std::move(lm));
  return a;
}

bool
BaseApp::getActorAABB(const EU::TSharedPointer<Actor> &actor,
                      EU::Vector3 &outMin,
                      EU::Vector3 &outMax) {
  if (actor.isNull())
    return false;
  EU::TSharedPointer<MeshRendererComponent> mr =
      actor->getComponent<MeshRendererComponent>();
  if (!mr)
    return false;
  Mesh *mesh = mr->getMesh();
  if (!mesh)
    return false;

  if (mesh == &m_cyberGunRenderMesh) {
    outMin = m_modelLocalMin;
    outMax = m_modelLocalMax;
    return true;
  }
  for (auto &lm : m_loadedModels) {
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
BaseApp::saveScene(const std::string &path) {
  std::ofstream stream(path, std::ios::binary | std::ios::trunc);
  if (!stream.is_open()) {
    ERROR(
        "Main", "saveScene", ("Failed to open scene file for writing: " + path).c_str());
    return false;
  }

  uint32_t actorCount = 0;
  for (const EU::TSharedPointer<Actor> &actor : m_actors) {
    if (!actor.isNull())
      ++actorCount;
  }

  bool ok = writeBinaryValue(stream, kSceneBinaryMagic) &&
            writeBinaryValue(stream, kSceneBinaryVersion) &&
            writeBinaryValue(stream, actorCount);

  for (size_t actorIndex = 0; actorIndex < m_actors.size(); ++actorIndex) {
    const EU::TSharedPointer<Actor> &actor = m_actors[actorIndex];
    if (actor.isNull())
      continue;

    const uint32_t savedIndex = static_cast<uint32_t>(actorIndex);
    ok = ok && writeBinaryValue(stream, savedIndex) &&
         writeBinaryString(stream, actor->getName());

    EU::TSharedPointer<Transform> transform = actor->getComponent<Transform>();
    const uint8_t hasTransform = transform ? 1 : 0;
    ok = ok && writeBinaryValue(stream, hasTransform);
    if (transform) {
      const EU::Vector3 &position = transform->getPosition();
      const EU::Vector3 &rotation = transform->getRotation();
      const EU::Vector3 &scale = transform->getScale();
      ok = ok && writeBinaryVector3(stream, position) &&
           writeBinaryVector3(stream, rotation) && writeBinaryVector3(stream, scale);
    }

    EU::TSharedPointer<MeshRendererComponent> meshRenderer =
        actor->getComponent<MeshRendererComponent>();
    const uint8_t hasMeshRenderer = meshRenderer ? 1 : 0;
    ok = ok && writeBinaryValue(stream, hasMeshRenderer);
    if (meshRenderer) {
      const uint8_t visible = meshRenderer->isVisible() ? 1 : 0;
      const uint8_t castShadow = meshRenderer->canCastShadow() ? 1 : 0;
      ok =
          ok && writeBinaryValue(stream, visible) && writeBinaryValue(stream, castShadow);
    }

    EU::TSharedPointer<LightComponent> lightComponent =
        actor->getComponent<LightComponent>();
    const uint8_t hasLight = lightComponent ? 1 : 0;
    ok = ok && writeBinaryValue(stream, hasLight);
    if (lightComponent) {
      const LightData &light = lightComponent->getLightData();
      const int32_t lightType = static_cast<int32_t>(light.type);
      ok = ok && writeBinaryValue(stream, lightType) &&
           writeBinaryVector3(stream, light.color) &&
           writeBinaryValue(stream, light.intensity) &&
           writeBinaryVector3(stream, light.direction) &&
           writeBinaryValue(stream, light.range) &&
           writeBinaryVector3(stream, light.position) &&
           writeBinaryValue(stream, light.spotAngle) &&
           writeBinaryValue(stream, light.width) &&
           writeBinaryValue(stream, light.height);
    }

    EU::TSharedPointer<AudioSourceComponent> audioSource =
        actor->getComponent<AudioSourceComponent>();
    const uint8_t hasAudioSource = audioSource ? 1 : 0;
    ok = ok && writeBinaryValue(stream, hasAudioSource);
    if (audioSource) {
      const uint8_t spatial = audioSource->isSpatial() ? 1 : 0;
      const uint8_t loop = audioSource->isLooping() ? 1 : 0;
      const uint8_t autoActivate = audioSource->isAutoActivate() ? 1 : 0;
      const uint8_t muted = audioSource->isMuted() ? 1 : 0;
      ok = ok && writeBinaryString(stream, audioSource->getAudioPath()) &&
           writeBinaryValue(stream, audioSource->getVolume()) &&
           writeBinaryValue(stream, audioSource->getPitch()) &&
           writeBinaryValue(stream, spatial) && writeBinaryValue(stream, loop) &&
           writeBinaryValue(stream, autoActivate) && writeBinaryValue(stream, muted) &&
           writeBinaryValue(stream, audioSource->getMinDistance()) &&
           writeBinaryValue(stream, audioSource->getMaxDistance());
    }

    EU::TSharedPointer<ParticleEmitterComponent> particleEmitter =
        actor->getComponent<ParticleEmitterComponent>();
    const uint8_t hasParticleEmitter = particleEmitter ? 1 : 0;
    ok = ok && writeBinaryValue(stream, hasParticleEmitter);
    if (particleEmitter) {
      ok = ok && writeParticleSettings(stream, particleEmitter->settings());
    }
  }

  if (!ok) {
    ERROR("Main", "saveScene", "Failed to write binary scene data");
    return false;
  }

  MESSAGE("Main", "saveScene", ("Binary scene saved to " + path).c_str());
  return true;
}

bool
BaseApp::loadScene(const std::string &path) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream.is_open())
    return false;

  uint32_t magic = 0;
  uint32_t binaryVersion = 0;
  if (readBinaryValue(stream, magic) && readBinaryValue(stream, binaryVersion) &&
      magic == kSceneBinaryMagic && binaryVersion >= 1 &&
      binaryVersion <= kSceneBinaryVersion) {
    uint32_t actorCount = 0;
    if (!readBinaryValue(stream, actorCount))
      return false;

    for (uint32_t i = 0; i < actorCount; ++i) {
      uint32_t actorIndex = 0;
      std::string name;
      if (!readBinaryValue(stream, actorIndex) || !readBinaryString(stream, name)) {
        return false;
      }

      EU::TSharedPointer<Actor> actor;
      if (actorIndex < m_actors.size()) {
        actor = m_actors[actorIndex];
      }
      if (actor.isNull()) {
        actor = EU::MakeShared<Actor>(m_device);
        addActorToScene(actor);
      }
      if (!actor.isNull()) {
        actor->setName(name);
      }

      uint8_t hasTransform = 0;
      if (!readBinaryValue(stream, hasTransform))
        return false;
      if (hasTransform) {
        EU::Vector3 position(0, 0, 0);
        EU::Vector3 rotation(0, 0, 0);
        EU::Vector3 scale(1, 1, 1);
        if (!readBinaryVector3(stream, position) ||
            !readBinaryVector3(stream, rotation) || !readBinaryVector3(stream, scale)) {
          return false;
        }
        if (!actor.isNull()) {
          EU::TSharedPointer<Transform> t = actor->getComponent<Transform>();
          if (t)
            t->setTransform(position, rotation, scale);
        }
      }

      uint8_t hasMeshRenderer = 0;
      if (!readBinaryValue(stream, hasMeshRenderer))
        return false;
      if (hasMeshRenderer) {
        uint8_t visible = 0;
        uint8_t castShadow = 0;
        if (!readBinaryValue(stream, visible) || !readBinaryValue(stream, castShadow)) {
          return false;
        }
        if (!actor.isNull()) {
          EU::TSharedPointer<MeshRendererComponent> mr =
              actor->getComponent<MeshRendererComponent>();
          if (mr) {
            mr->setVisible(visible != 0);
            mr->setCastShadow(castShadow != 0);
          }
        }
      }

      uint8_t hasLight = 0;
      if (!readBinaryValue(stream, hasLight))
        return false;
      if (hasLight) {
        int32_t type = 0;
        LightData light;
        if (!readBinaryValue(stream, type) || !readBinaryVector3(stream, light.color) ||
            !readBinaryValue(stream, light.intensity) ||
            !readBinaryVector3(stream, light.direction) ||
            !readBinaryValue(stream, light.range) ||
            !readBinaryVector3(stream, light.position) ||
            !readBinaryValue(stream, light.spotAngle) ||
            !readBinaryValue(stream, light.width) ||
            !readBinaryValue(stream, light.height)) {
          return false;
        }
        light.type = static_cast<LightType>(type);
        if (!actor.isNull()) {
          EU::TSharedPointer<LightComponent> lc = actor->getComponent<LightComponent>();
          if (lc)
            lc->getLightData() = light;
        }
      }

      if (binaryVersion >= 2) {
        uint8_t hasAudioSource = 0;
        if (!readBinaryValue(stream, hasAudioSource))
          return false;
        if (hasAudioSource) {
          std::string audioPath;
          float volume = 1.0f;
          float pitch = 0.0f;
          uint8_t spatial = 0, loop = 0, autoActivate = 0, muted = 0;
          float minDistance = 1.0f, maxDistance = 25.0f;
          if (!readBinaryString(stream, audioPath) || !readBinaryValue(stream, volume) ||
              !readBinaryValue(stream, pitch) || !readBinaryValue(stream, spatial) ||
              !readBinaryValue(stream, loop) || !readBinaryValue(stream, autoActivate) ||
              !readBinaryValue(stream, muted) || !readBinaryValue(stream, minDistance) ||
              !readBinaryValue(stream, maxDistance))
            return false;

          EU::TSharedPointer<Transform> transform = actor->getComponent<Transform>();
          EU::TSharedPointer<AudioSourceComponent> audioSource =
              actor->getComponent<AudioSourceComponent>();
          if (!audioSource) {
            audioSource = EU::MakeShared<AudioSourceComponent>(transform.get());
            actor->addComponent(audioSource);
            m_audioSystem.registerSource(audioSource.get());
          }
          audioSource->setAudioPath(audioPath);
          audioSource->setVolume(volume);
          audioSource->setPitch(pitch);
          audioSource->setSpatial(spatial != 0);
          audioSource->setLoop(loop != 0);
          audioSource->setAutoActivate(autoActivate != 0);
          audioSource->setMuted(muted != 0);
          audioSource->setAttenuation(minDistance, maxDistance);
        }
      }

      if (binaryVersion >= 3) {
        uint8_t hasParticleEmitter = 0;
        if (!readBinaryValue(stream, hasParticleEmitter))
          return false;
        if (hasParticleEmitter) {
          ParticleEmitterSettings settings;
          if (!readParticleSettings(stream, settings))
            return false;
          EU::TSharedPointer<Transform> transform = actor->getComponent<Transform>();
          EU::TSharedPointer<ParticleEmitterComponent> particleEmitter =
              actor->getComponent<ParticleEmitterComponent>();
          if (!particleEmitter) {
            particleEmitter = EU::MakeShared<ParticleEmitterComponent>(transform.get());
            if (FAILED(particleEmitter->init(m_device)))
              return false;
            actor->addComponent(particleEmitter);
          }
          particleEmitter->settings() = settings;
          if (settings.emissionMode == ParticleEmissionMode::Burst)
            particleEmitter->triggerBurst();
        }
      }
    }

    MESSAGE("Main", "loadScene", ("Binary scene loaded from " + path).c_str());
    return true;
  }

  stream.close();
  stream.clear();
  stream.open(path);
  if (!stream.is_open())
    return false;

  std::string token;
  int version = 0;
  stream >> token >> version;
  if (token != "WVSCENE" || version < 1)
    return false;

  int actorCount = 0;
  stream >> token >> actorCount;
  EU::TSharedPointer<Actor> currentActor;

  while (stream >> token) {
    if (token == "ACTOR") {
      int index;
      std::string name;
      stream >> index >> std::quoted(name);

      currentActor.reset();
      if (index < (int)m_actors.size()) {
        currentActor = m_actors[index];
      }
      if (!currentActor.isNull()) {
        currentActor->setName(name);
      }
    } else if (token == "POSITION") {
      float x, y, z;
      stream >> x >> y >> z;
      if (!currentActor.isNull()) {
        EU::TSharedPointer<Transform> t = currentActor->getComponent<Transform>();
        if (t)
          t->setPosition(EU::Vector3(x, y, z));
      }
    } else if (token == "ROTATION") {
      float x, y, z;
      stream >> x >> y >> z;
      if (!currentActor.isNull()) {
        EU::TSharedPointer<Transform> t = currentActor->getComponent<Transform>();
        if (t)
          t->setRotation(EU::Vector3(x, y, z));
      }
    } else if (token == "SCALE") {
      float x, y, z;
      stream >> x >> y >> z;
      if (!currentActor.isNull()) {
        EU::TSharedPointer<Transform> t = currentActor->getComponent<Transform>();
        if (t)
          t->setScale(EU::Vector3(x, y, z));
      }
    } else if (token == "LIGHT") {
      int type;
      float cx, cy, cz, intensity, dx, dy, dz, range;
      stream >> type >> cx >> cy >> cz >> intensity >> dx >> dy >> dz >> range;
      if (!m_directionalLightActor.isNull()) {
        EU::TSharedPointer<LightComponent> lc =
            m_directionalLightActor->getComponent<LightComponent>();
        if (lc) {
          LightData &light = lc->getLightData();
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
