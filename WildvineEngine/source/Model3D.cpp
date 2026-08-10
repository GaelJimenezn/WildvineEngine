/**
 * @file Model3D.cpp
 * @brief Implementa la logica de Model3D dentro del subsistema Core.
 * @ingroup core
 */
#include "Model3D.h"
#include <chrono>
#include <cstdint>
#include <cmath>
#include <fstream>
#include <unordered_map>
#include <sstream>

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

namespace {
constexpr uint32_t kModelCacheMagic = 0x48564D57; // WMVH
constexpr uint32_t kModelCacheVersion = 11;

struct

    ModelCacheEntry {
  std::vector<MeshComponent> meshes;
  std::vector<std::string> textureFileNames;
  std::vector<EmbeddedTexture> embeddedTextures;
  std::vector<ImportedMaterialInfo> materialInfos;
};

std::unordered_map<std::string, ModelCacheEntry> g_modelCache;

bool
GetFileWriteTime(const std::string &path, ULONGLONG &outWriteTime) {
  WIN32_FILE_ATTRIBUTE_DATA attributes{};
  if (!GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &attributes)) {
    return false;
  }

  ULARGE_INTEGER fileTime{};
  fileTime.LowPart = attributes.ftLastWriteTime.dwLowDateTime;
  fileTime.HighPart = attributes.ftLastWriteTime.dwHighDateTime;
  outWriteTime = fileTime.QuadPart;
  return true;
}

bool
WriteString(std::ofstream &stream, const std::string &value) {
  const uint32_t length = static_cast<uint32_t>(value.size());
  stream.write(reinterpret_cast<const char *>(&length), sizeof(length));
  if (length > 0) {
    stream.write(value.data(), length);
  }
  return stream.good();
}

bool
ReadString(std::ifstream &stream, std::string &value) {
  uint32_t length = 0;
  stream.read(reinterpret_cast<char *>(&length), sizeof(length));
  if (!stream.good()) {
    return false;
  }

  value.resize(length);
  if (length > 0) {
    stream.read(&value[0], length);
  }
  return stream.good();
}

std::string
ToLowerCopy(std::string value) {
  for (char &c : value) {
    if (c >= 'A' && c <= 'Z')
      c = static_cast<char>(c + 32);
  }
  return value;
}

bool
EndsWith(const std::string &value, const std::string &suffix) {
  return value.size() >= suffix.size() &&
         value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

void
NormalizeVector(EU::Vector3 &value) {
  const float lengthSq = value.x * value.x + value.y * value.y + value.z * value.z;
  if (lengthSq <= 1e-20f) {
    value = EU::Vector3(0.0f, 0.0f, 1.0f);
    return;
  }
  const float invLength = 1.0f / std::sqrt(lengthSq);
  value.x *= invLength;
  value.y *= invLength;
  value.z *= invLength;
}

EU::Vector3
TransformGLTFPoint(const cgltf_float *m, const EU::Vector3 &p) {
  return EU::Vector3(p.x * m[0] + p.y * m[4] + p.z * m[8] + m[12],
                     p.x * m[1] + p.y * m[5] + p.z * m[9] + m[13],
                     p.x * m[2] + p.y * m[6] + p.z * m[10] + m[14]);
}

EU::Vector3
TransformGLTFDirection(const cgltf_float *m, const EU::Vector3 &v) {
  return EU::Vector3(v.x * m[0] + v.y * m[4] + v.z * m[8],
                     v.x * m[1] + v.y * m[5] + v.z * m[9],
                     v.x * m[2] + v.y * m[6] + v.z * m[10]);
}

EU::Vector3
GLTFToEnginePoint(const EU::Vector3 &p) {
  return EU::Vector3(p.x, p.y, -p.z);
}

EU::Vector3
GLTFToEngineDirection(const EU::Vector3 &v) {
  return EU::Vector3(v.x, v.y, -v.z);
}

EU::Vector3
YUpToZUp(const EU::Vector3 &v) {
  return EU::Vector3(v.x, -v.z, v.y);
}

EU::Vector3
TransformFBXPoint(const FbxAMatrix &m, const FbxVector4 &p) {
  const FbxVector4 transformed = m.MultT(p);
  return YUpToZUp(EU::Vector3(static_cast<float>(transformed[0]),
                              static_cast<float>(transformed[1]),
                              static_cast<float>(transformed[2])));
}

EU::Vector3
TransformFBXDirection(const FbxAMatrix &m, const FbxVector4 &v) {
  const FbxVector4 transformed = m.MultR(v);
  EU::Vector3 result = YUpToZUp(EU::Vector3(static_cast<float>(transformed[0]),
                                            static_cast<float>(transformed[1]),
                                            static_cast<float>(transformed[2])));
  NormalizeVector(result);
  return result;
}
} // namespace

Model3D::~Model3D() {
  unload();
}

bool
Model3D::load(const std::string &path) {
  SetPath(path);
  SetState(ResourceState::Loading);

  auto cacheIt = g_modelCache.find(path);
  if (cacheIt != g_modelCache.end()) {
    m_meshes = cacheIt->second.meshes;
    textureFileNames = cacheIt->second.textureFileNames;
    m_embeddedTextures = cacheIt->second.embeddedTextures;
    m_materialInfos = cacheIt->second.materialInfos;
    SetState(ResourceState::Loaded);
    return true;
  }

  const bool success = init();
  SetState(success ? ResourceState::Loaded : ResourceState::Failed);
  return success;
}

bool
Model3D::init() {
  m_meshes.clear();
  textureFileNames.clear();
  m_embeddedTextures.clear();
  m_materialInfos.clear();

  const std::string cachePath = GetBinaryCachePath();
  if (IsBinaryCacheUpToDate(m_filePath, cachePath) && LoadBinaryCache(cachePath)) {
    g_modelCache[m_filePath] =
        ModelCacheEntry{m_meshes, textureFileNames, m_embeddedTextures, m_materialInfos};
    return true;
  }

  const auto begin = std::chrono::high_resolution_clock::now();
  std::vector<MeshComponent> loadedMeshes;
  if (m_modelType == ModelType::FBX) {
    loadedMeshes = LoadFBXModel(m_filePath);
  } else if (m_modelType == ModelType::OBJ) {
    loadedMeshes = LoadOBJModel(m_filePath);
  } else if (m_modelType == ModelType::GLTF) {
    loadedMeshes = LoadGLTFModel(m_filePath);
  }
  const auto end = std::chrono::high_resolution_clock::now();
  const auto elapsedMs =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

  if (loadedMeshes.empty()) {
    return false;
  }

  m_meshes = loadedMeshes;
  g_modelCache[m_filePath] =
      ModelCacheEntry{m_meshes, textureFileNames, m_embeddedTextures, m_materialInfos};
  SaveBinaryCache(cachePath);

  const std::wstring modelPathW(m_filePath.begin(), m_filePath.end());
  MESSAGE("ModelLoader",
          "ModelLoader",
          L"Loaded model '" << modelPathW << L"' in " << elapsedMs << L" ms. Meshes: "
                            << m_meshes.size())
  return true;
}

void
Model3D::unload() {
  if (lScene) {
    lScene->Destroy();
    lScene = nullptr;
  }
  if (lSdkManager) {
    lSdkManager->Destroy();
    lSdkManager = nullptr;
  }

  SetState(ResourceState::Unloaded);
}

size_t
Model3D::getSizeInBytes() const {
  size_t totalSize = 0;
  for (const auto &mesh : m_meshes) {
    totalSize += mesh.m_vertex.size() * sizeof(SimpleVertex);
    totalSize += mesh.m_index.size() * sizeof(unsigned int);
  }
  return totalSize;
}

bool
Model3D::InitializeFBXManager() {
  lSdkManager = FbxManager::Create();
  if (!lSdkManager) {
    ERROR("ModelLoader", "FbxManager::Create()", "Unable to create FBX Manager!");
    return false;
  }

  FbxIOSettings *ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
  lSdkManager->SetIOSettings(ios);

  lScene = FbxScene::Create(lSdkManager, "MyScene");
  if (!lScene) {
    ERROR("ModelLoader", "FbxScene::Create()", "Unable to create FBX Scene!");
    return false;
  }
  return true;
}

std::vector<MeshComponent>
Model3D::LoadFBXModel(const std::string &filePath) {
  std::vector<MeshComponent> loadedMeshes;

  if (InitializeFBXManager()) {
    FbxImporter *lImporter = FbxImporter::Create(lSdkManager, "");
    if (!lImporter) {
      ERROR("ModelLoader", "FbxImporter::Create()", "Unable to create FBX Importer!");
      return loadedMeshes;
    }

    if (!lImporter->Initialize(filePath.c_str(), -1, lSdkManager->GetIOSettings())) {
      ERROR("ModelLoader",
            "FbxImporter::Initialize()",
            "Unable to initialize FBX Importer! Error: "
                << lImporter->GetStatus().GetErrorString());
      lImporter->Destroy();
      return loadedMeshes;
    }

    if (!lImporter->Import(lScene)) {
      ERROR("ModelLoader",
            "FbxImporter::Import()",
            "Unable to import FBX Scene! Error: "
                << lImporter->GetStatus().GetErrorString());
      lImporter->Destroy();
      return loadedMeshes;
    } else {
      m_name = lImporter->GetFileName();
    }

    FbxAxisSystem::DirectX.ConvertScene(lScene);
    FbxGeometryConverter gc(lSdkManager);
    gc.Triangulate(lScene, true);

    lImporter->Destroy();

    FbxNode *lRootNode = lScene->GetRootNode();
    if (lRootNode) {
      m_meshes.clear();
      for (int i = 0; i < lRootNode->GetChildCount(); i++) {
        ProcessFBXNode(lRootNode->GetChild(i));
      }
      loadedMeshes = m_meshes;
      return loadedMeshes;
    } else {
      ERROR("ModelLoader",
            "FbxScene::GetRootNode()",
            "Unable to get root node from FBX Scene!");
      return loadedMeshes;
    }
  }
  return loadedMeshes;
}

std::vector<MeshComponent>
Model3D::LoadOBJModel(const std::string &filePath) {
  struct ObjIndex {
    int position = -1;
    int texcoord = -1;
    int normal = -1;

    bool
    operator==(const ObjIndex &other) const {
      return position == other.position && texcoord == other.texcoord &&
             normal == other.normal;
    }
  };

  struct

      ObjIndexHasher {
    size_t
    operator()(const ObjIndex &index) const {
      size_t seed = static_cast<size_t>(index.position + 1);
      seed ^= static_cast<size_t>(index.texcoord + 1) + 0x9e3779b9 + (seed << 6) +
              (seed >> 2);
      seed ^=
          static_cast<size_t>(index.normal + 1) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      return seed;
    }
  };

  struct

      ObjMeshBuilder {
    std::string name = "default";
    std::vector<SimpleVertex> vertices;
    std::vector<unsigned int> indices;
    std::unordered_map<ObjIndex, unsigned int, ObjIndexHasher> vertexLookup;
  };

  auto fixIndex = [](int index, int count) -> int {
    if (index > 0)
      return index - 1;
    if (index < 0)
      return count + index;
    return -1;
  };

  auto normalize = [](EU::Vector3 &value) {
    const float lengthSq = value.x * value.x + value.y * value.y + value.z * value.z;
    if (lengthSq <= 1e-20f) {
      value = EU::Vector3(0.0f, 0.0f, 1.0f);
      return;
    }
    const float invLength = 1.0f / std::sqrt(lengthSq);
    value.x *= invLength;
    value.y *= invLength;
    value.z *= invLength;
  };

  auto parseFaceVertex = [&](const std::string &token,
                             int positionCount,
                             int texcoordCount,
                             int normalCount) -> ObjIndex {
    ObjIndex result{};
    size_t firstSlash = token.find('/');
    size_t secondSlash =
        token.find('/', firstSlash == std::string::npos ? token.size() : firstSlash + 1);

    const std::string positionToken = token.substr(0, firstSlash);
    const std::string texcoordToken =
        (firstSlash == std::string::npos)
            ? std::string()
            : (secondSlash == std::string::npos
                   ? token.substr(firstSlash + 1)
                   : token.substr(firstSlash + 1, secondSlash - firstSlash - 1));
    const std::string normalToken = (secondSlash == std::string::npos)
                                        ? std::string()
                                        : token.substr(secondSlash + 1);

    if (!positionToken.empty()) {
      result.position = fixIndex(std::stoi(positionToken), positionCount);
    }
    if (!texcoordToken.empty()) {
      result.texcoord = fixIndex(std::stoi(texcoordToken), texcoordCount);
    }
    if (!normalToken.empty()) {
      result.normal = fixIndex(std::stoi(normalToken), normalCount);
    }

    return result;
  };

  auto computeTangents = [&](MeshComponent &mesh) {
    for (size_t i = 0; i + 2 < mesh.m_index.size(); i += 3) {
      SimpleVertex &v0 = mesh.m_vertex[mesh.m_index[i + 0]];
      SimpleVertex &v1 = mesh.m_vertex[mesh.m_index[i + 1]];
      SimpleVertex &v2 = mesh.m_vertex[mesh.m_index[i + 2]];

      const EU::Vector3 edge1 = v1.Position - v0.Position;
      const EU::Vector3 edge2 = v2.Position - v0.Position;
      const float du1 = v1.TextureCoordinate.x - v0.TextureCoordinate.x;
      const float dv1 = v1.TextureCoordinate.y - v0.TextureCoordinate.y;
      const float du2 = v2.TextureCoordinate.x - v0.TextureCoordinate.x;
      const float dv2 = v2.TextureCoordinate.y - v0.TextureCoordinate.y;
      const float denominator = du1 * dv2 - du2 * dv1;
      const float invDenominator =
          std::fabs(denominator) < 1e-8f ? 0.0f : 1.0f / denominator;

      const EU::Vector3 tangent((edge1.x * dv2 - edge2.x * dv1) * invDenominator,
                                (edge1.y * dv2 - edge2.y * dv1) * invDenominator,
                                (edge1.z * dv2 - edge2.z * dv1) * invDenominator);
      const EU::Vector3 bitangent((edge2.x * du1 - edge1.x * du2) * invDenominator,
                                  (edge2.y * du1 - edge1.y * du2) * invDenominator,
                                  (edge2.z * du1 - edge1.z * du2) * invDenominator);

      v0.Tangent += tangent;
      v1.Tangent += tangent;
      v2.Tangent += tangent;
      v0.Bitangent += bitangent;
      v1.Bitangent += bitangent;
      v2.Bitangent += bitangent;
    }

    for (SimpleVertex &vertex : mesh.m_vertex) {
      normalize(vertex.Normal);

      const float tangentDotNormal = vertex.Tangent.x * vertex.Normal.x +
                                     vertex.Tangent.y * vertex.Normal.y +
                                     vertex.Tangent.z * vertex.Normal.z;
      vertex.Tangent = vertex.Tangent - (vertex.Normal * tangentDotNormal);
      normalize(vertex.Tangent);

      vertex.Bitangent = EU::Vector3(
          vertex.Normal.y * vertex.Tangent.z - vertex.Normal.z * vertex.Tangent.y,
          vertex.Normal.z * vertex.Tangent.x - vertex.Normal.x * vertex.Tangent.z,
          vertex.Normal.x * vertex.Tangent.y - vertex.Normal.y * vertex.Tangent.x);
      normalize(vertex.Bitangent);
    }
  };

  auto flushMesh = [&](ObjMeshBuilder &builder, std::vector<MeshComponent> &meshes) {
    if (builder.indices.empty() || builder.vertices.empty()) {
      builder = ObjMeshBuilder{};
      return;
    }

    MeshComponent mesh;
    mesh.m_name = builder.name;
    mesh.m_vertex = std::move(builder.vertices);
    mesh.m_index = std::move(builder.indices);
    mesh.m_numVertex = static_cast<int>(mesh.m_vertex.size());
    mesh.m_numIndex = static_cast<int>(mesh.m_index.size());
    computeTangents(mesh);
    meshes.push_back(std::move(mesh));
    builder = ObjMeshBuilder{};
  };

  std::ifstream file(filePath);
  if (!file.is_open()) {
    ERROR(
        "ModelLoader", "LoadOBJModel", ("Unable to open OBJ file: " + filePath).c_str());
    return {};
  }

  std::vector<EU::Vector3> positions;
  std::vector<EU::Vector2> texcoords;
  std::vector<EU::Vector3> normals;
  std::vector<MeshComponent> loadedMeshes;
  ObjMeshBuilder currentMesh;
  std::string currentGroupName = "default";
  currentMesh.name = currentGroupName;

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#') {
      continue;
    }

    std::istringstream stream(line);
    std::string command;
    stream >> command;

    if (command == "v") {
      EU::Vector3 position;
      stream >> position.x >> position.y >> position.z;
      positions.push_back(position);
    } else if (command == "vt") {
      EU::Vector2 uv;
      stream >> uv.x >> uv.y;
      uv.y = 1.0f - uv.y;
      texcoords.push_back(uv);
    } else if (command == "vn") {
      EU::Vector3 normal;
      stream >> normal.x >> normal.y >> normal.z;
      normalize(normal);
      normals.push_back(normal);
    } else if (command == "g" || command == "o") {
      flushMesh(currentMesh, loadedMeshes);
      stream >> currentGroupName;
      if (currentGroupName.empty()) {
        currentGroupName = "default";
      }
      currentMesh.name = currentGroupName;
    } else if (command == "usemtl") {
      if (!currentMesh.indices.empty()) {
        flushMesh(currentMesh, loadedMeshes);
      }
      currentMesh.name = currentGroupName;
    } else if (command == "f") {
      std::vector<unsigned int> polygonIndices;
      std::string token;
      while (stream >> token) {
        const ObjIndex objIndex = parseFaceVertex(token,
                                                  static_cast<int>(positions.size()),
                                                  static_cast<int>(texcoords.size()),
                                                  static_cast<int>(normals.size()));

        auto it = currentMesh.vertexLookup.find(objIndex);
        if (it == currentMesh.vertexLookup.end()) {
          SimpleVertex vertex{};
          if (objIndex.position >= 0 &&
              objIndex.position < static_cast<int>(positions.size())) {
            vertex.Position = positions[objIndex.position];
          }
          if (objIndex.texcoord >= 0 &&
              objIndex.texcoord < static_cast<int>(texcoords.size())) {
            vertex.TextureCoordinate = texcoords[objIndex.texcoord];
          } else {
            vertex.TextureCoordinate = EU::Vector2(0.0f, 0.0f);
          }
          if (objIndex.normal >= 0 &&
              objIndex.normal < static_cast<int>(normals.size())) {
            vertex.Normal = normals[objIndex.normal];
          } else {
            vertex.Normal = EU::Vector3(0.0f, 0.0f, 1.0f);
          }
          vertex.Tangent = EU::Vector3(0.0f, 0.0f, 0.0f);
          vertex.Bitangent = EU::Vector3(0.0f, 0.0f, 0.0f);

          const unsigned int newIndex =
              static_cast<unsigned int>(currentMesh.vertices.size());
          currentMesh.vertices.push_back(vertex);
          currentMesh.vertexLookup[objIndex] = newIndex;
          polygonIndices.push_back(newIndex);
        } else {
          polygonIndices.push_back(it->second);
        }
      }

      for (size_t i = 1; i + 1 < polygonIndices.size(); ++i) {
        currentMesh.indices.push_back(polygonIndices[0]);
        currentMesh.indices.push_back(polygonIndices[i]);
        currentMesh.indices.push_back(polygonIndices[i + 1]);
      }
    }
  }

  flushMesh(currentMesh, loadedMeshes);
  return loadedMeshes;
}

std::vector<MeshComponent>
Model3D::LoadGLTFModel(const std::string &filePath) {
  std::vector<MeshComponent> loadedMeshes;
  cgltf_options options = {};
  cgltf_data *data = NULL;
  cgltf_result result = cgltf_parse_file(&options, filePath.c_str(), &data);
  if (result != cgltf_result_success) {
    ERROR("ModelLoader",
          "LoadGLTFModel",
          ("Unable to parse GLTF file: " + filePath).c_str());
    return loadedMeshes;
  }

  result = cgltf_load_buffers(&options, data, filePath.c_str());
  if (result != cgltf_result_success) {
    ERROR("ModelLoader",
          "LoadGLTFModel",
          ("Unable to load buffers for GLTF file: " + filePath).c_str());
    cgltf_free(data);
    return loadedMeshes;
  }

  auto appendEmbeddedTexture = [&](cgltf_texture *texture,
                                   int materialIndex,
                                   int textureSlot,
                                   const char *slotName) {
    if (!texture || !texture->image)
      return;
    cgltf_image *image = texture->image;
    if (!image->buffer_view || !image->buffer_view->buffer ||
        !image->buffer_view->buffer->data) {
      return;
    }

    EmbeddedTexture emb;
    emb.materialIndex = materialIndex;
    emb.textureSlot = textureSlot;
    emb.name = image->uri ? image->uri
                          : (image->name ? image->name
                                         : ("embedded_" + std::string(slotName) + "_" +
                                            std::to_string(m_embeddedTextures.size())));

    const std::string lowerName = ToLowerCopy(emb.name);
    if (!image->uri && image->mime_type && !EndsWith(lowerName, ".png") &&
        !EndsWith(lowerName, ".jpg") && !EndsWith(lowerName, ".jpeg")) {
      if (std::string(image->mime_type) == "image/jpeg")
        emb.name += ".jpg";
      else if (std::string(image->mime_type) == "image/png") {
        emb.name += ".png";
      }
    }

    unsigned char *bufferData =
        static_cast<unsigned char *>(image->buffer_view->buffer->data) +
        image->buffer_view->offset;
    emb.data.assign(bufferData, bufferData + image->buffer_view->size);
    m_embeddedTextures.push_back(std::move(emb));
  };

  m_materialInfos.clear();
  m_materialInfos.resize(data->materials_count);
  for (cgltf_size m = 0; m < data->materials_count; ++m) {
    cgltf_material &mat = data->materials[m];
    ImportedMaterialInfo &materialInfo = m_materialInfos[m];
    if (mat.has_pbr_metallic_roughness) {
      const cgltf_pbr_metallic_roughness &pbr = mat.pbr_metallic_roughness;
      materialInfo.baseColor = XMFLOAT4(pbr.base_color_factor[0],
                                        pbr.base_color_factor[1],
                                        pbr.base_color_factor[2],
                                        pbr.base_color_factor[3]);
      materialInfo.metallic = pbr.metallic_factor;
      materialInfo.roughness = pbr.roughness_factor;
      appendEmbeddedTexture(
          pbr.base_color_texture.texture, static_cast<int>(m), 0, "basecolor");
      appendEmbeddedTexture(pbr.metallic_roughness_texture.texture,
                            static_cast<int>(m),
                            6,
                            "metallicroughness");
    }
    materialInfo.alphaBlend = mat.alpha_mode == cgltf_alpha_mode_blend;
    appendEmbeddedTexture(mat.normal_texture.texture, static_cast<int>(m), 1, "normal");
    appendEmbeddedTexture(
        mat.occlusion_texture.texture, static_cast<int>(m), 4, "occlusion");
    appendEmbeddedTexture(
        mat.emissive_texture.texture, static_cast<int>(m), 5, "emissive");
  }

  for (cgltf_size i = 0; i < data->nodes_count; ++i) {
    cgltf_node &node = data->nodes[i];
    if (!node.mesh)
      continue;

    cgltf_float world_transform[16];
    cgltf_node_transform_world(&node, world_transform);
    cgltf_mesh &mesh = *node.mesh;

    for (cgltf_size j = 0; j < mesh.primitives_count; ++j) {
      cgltf_primitive &primitive = mesh.primitives[j];
      if (primitive.type != cgltf_primitive_type_triangles)
        continue;

      MeshComponent mc;
      mc.m_name = mesh.name ? mesh.name : "unnamed_mesh";
      mc.m_materialIndex =
          primitive.material ? (int)(primitive.material - data->materials) : 0;

      // Parse indices
      if (primitive.indices) {
        cgltf_accessor *accessor = primitive.indices;
        mc.m_numIndex = (int)accessor->count;
        for (cgltf_size k = 0; k < accessor->count; ++k) {
          mc.m_index.push_back((unsigned int)cgltf_accessor_read_index(accessor, k));
        }
      }

      // Parse vertices
      cgltf_size vertexCount = 0;
      if (primitive.attributes_count > 0) {
        vertexCount = primitive.attributes[0].data->count;
        mc.m_numVertex = (int)vertexCount;
        mc.m_vertex.resize(vertexCount);
        memset(mc.m_vertex.data(), 0, vertexCount * sizeof(SimpleVertex));
      }

      for (cgltf_size a = 0; a < primitive.attributes_count; ++a) {
        cgltf_attribute &attr = primitive.attributes[a];
        for (cgltf_size v = 0; v < vertexCount; ++v) {
          SimpleVertex &vertex = mc.m_vertex[v];
          if (attr.type == cgltf_attribute_type_position) {
            cgltf_accessor_read_float(attr.data, v, &vertex.Position.x, 3);
          } else if (attr.type == cgltf_attribute_type_normal) {
            cgltf_accessor_read_float(attr.data, v, &vertex.Normal.x, 3);
          } else if (attr.type == cgltf_attribute_type_texcoord) {
            cgltf_accessor_read_float(attr.data, v, &vertex.TextureCoordinate.x, 2);
          } else if (attr.type == cgltf_attribute_type_tangent) {
            cgltf_accessor_read_float(attr.data, v, &vertex.Tangent.x, 3);
          }
        }
      }

      for (SimpleVertex &vertex : mc.m_vertex) {
        vertex.Position = YUpToZUp(
            GLTFToEnginePoint(TransformGLTFPoint(world_transform, vertex.Position)));
        vertex.Normal = YUpToZUp(GLTFToEngineDirection(
            TransformGLTFDirection(world_transform, vertex.Normal)));
        vertex.Tangent = YUpToZUp(GLTFToEngineDirection(
            TransformGLTFDirection(world_transform, vertex.Tangent)));
      }

      // Si no hay m_index, generamos secuencialmente
      if (mc.m_index.empty()) {
        for (cgltf_size v = 0; v < vertexCount; ++v) {
          mc.m_index.push_back((unsigned int)v);
        }
        mc.m_numIndex = (int)mc.m_index.size();
      }

      // Invertir winding order para DirectX (GLTF usa counter-clockwise)
      for (size_t k = 0; k + 2 < mc.m_index.size(); k += 3) {
        std::swap(mc.m_index[k + 1], mc.m_index[k + 2]);
      }

      // Generate tangents and bitangents if they are missing
      for (size_t k = 0; k + 2 < mc.m_index.size(); k += 3) {
        SimpleVertex &v0 = mc.m_vertex[mc.m_index[k + 0]];
        SimpleVertex &v1 = mc.m_vertex[mc.m_index[k + 1]];
        SimpleVertex &v2 = mc.m_vertex[mc.m_index[k + 2]];

        const EU::Vector3 edge1 = v1.Position - v0.Position;
        const EU::Vector3 edge2 = v2.Position - v0.Position;
        const float du1 = v1.TextureCoordinate.x - v0.TextureCoordinate.x;
        const float dv1 = v1.TextureCoordinate.y - v0.TextureCoordinate.y;
        const float du2 = v2.TextureCoordinate.x - v0.TextureCoordinate.x;
        const float dv2 = v2.TextureCoordinate.y - v0.TextureCoordinate.y;
        const float denominator = du1 * dv2 - du2 * dv1;
        const float invDenominator =
            std::fabs(denominator) < 1e-8f ? 0.0f : 1.0f / denominator;

        const EU::Vector3 tangent((edge1.x * dv2 - edge2.x * dv1) * invDenominator,
                                  (edge1.y * dv2 - edge2.y * dv1) * invDenominator,
                                  (edge1.z * dv2 - edge2.z * dv1) * invDenominator);
        const EU::Vector3 bitangent((edge2.x * du1 - edge1.x * du2) * invDenominator,
                                    (edge2.y * du1 - edge1.y * du2) * invDenominator,
                                    (edge2.z * du1 - edge1.z * du2) * invDenominator);

        v0.Tangent += tangent;
        v1.Tangent += tangent;
        v2.Tangent += tangent;
        v0.Bitangent += bitangent;
        v1.Bitangent += bitangent;
        v2.Bitangent += bitangent;
      }

      for (SimpleVertex &vertex : mc.m_vertex) {
        NormalizeVector(vertex.Normal);
        const float tangentDotNormal = vertex.Tangent.x * vertex.Normal.x +
                                       vertex.Tangent.y * vertex.Normal.y +
                                       vertex.Tangent.z * vertex.Normal.z;
        vertex.Tangent = vertex.Tangent - (vertex.Normal * tangentDotNormal);
        NormalizeVector(vertex.Tangent);

        vertex.Bitangent = EU::Vector3(
            vertex.Normal.y * vertex.Tangent.z - vertex.Normal.z * vertex.Tangent.y,
            vertex.Normal.z * vertex.Tangent.x - vertex.Normal.x * vertex.Tangent.z,
            vertex.Normal.x * vertex.Tangent.y - vertex.Normal.y * vertex.Tangent.x);
        NormalizeVector(vertex.Bitangent);
      }

      loadedMeshes.push_back(mc);
    }
  }

  cgltf_free(data);
  return loadedMeshes;
}

void
Model3D::ProcessFBXNode(FbxNode *node) {
  if (node->GetNodeAttribute()) {
    if (node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh) {
      ProcessFBXMesh(node);
    }
  }

  for (int i = 0; i < node->GetChildCount(); i++) {
    ProcessFBXNode(node->GetChild(i));
  }
}

void
Model3D::ProcessFBXMesh(FbxNode *node) {
  FbxMesh *mesh = node->GetMesh();
  if (!mesh)
    return;
  if (mesh->GetPolygonCount() == 0)
    return; // Prevent empty vertex buffer error

  if (mesh->GetElementNormalCount() == 0)
    mesh->GenerateNormals(true, true);

  const char *uvSetName = nullptr;
  {
    FbxStringList uvSets;
    mesh->GetUVSetNames(uvSets);
    if (uvSets.GetCount() > 0)
      uvSetName = uvSets[0];
  }

  if (mesh->GetElementTangentCount() == 0 && uvSetName)
    mesh->GenerateTangentsData(uvSetName);

  const FbxGeometryElementUV *uvElem =
      (mesh->GetElementUVCount() > 0) ? mesh->GetElementUV(0) : nullptr;
  const FbxGeometryElementTangent *tanElem =
      (mesh->GetElementTangentCount() > 0) ? mesh->GetElementTangent(0) : nullptr;
  const FbxGeometryElementBinormal *binElem =
      (mesh->GetElementBinormalCount() > 0) ? mesh->GetElementBinormal(0) : nullptr;
  const FbxGeometryElementMaterial *materialElem =
      (mesh->GetElementMaterialCount() > 0) ? mesh->GetElementMaterial(0) : nullptr;

  FbxAMatrix geometryTransform;
  geometryTransform.SetT(node->GetGeometricTranslation(FbxNode::eSourcePivot));
  geometryTransform.SetR(node->GetGeometricRotation(FbxNode::eSourcePivot));
  geometryTransform.SetS(node->GetGeometricScaling(FbxNode::eSourcePivot));
  const FbxAMatrix nodeTransform = node->EvaluateGlobalTransform() * geometryTransform;

  std::vector<SimpleVertex> vertices;
  vertices.reserve(mesh->GetPolygonCount() * 3);

  struct

      Triangle {
    unsigned int i0;
    unsigned int i1;
    unsigned int i2;
    int materialSlot;
  };
  std::vector<Triangle> triangles;
  triangles.reserve(mesh->GetPolygonCount() * 3);

  std::vector<int> nodeMaterialIndices;
  const int nodeMaterialCount = node->GetMaterialCount();
  if (nodeMaterialCount > 0) {
    nodeMaterialIndices.reserve(nodeMaterialCount);
    for (int i = 0; i < nodeMaterialCount; ++i) {
      nodeMaterialIndices.push_back(ProcessFBXMaterials(node->GetMaterial(i)));
    }
  } else {
    nodeMaterialIndices.push_back(ProcessFBXMaterials(nullptr));
  }

  auto readV2 = [](const FbxGeometryElementUV *elem, int cpIdx, int pvIdx) -> FbxVector2 {
    if (!elem)
      return FbxVector2(0, 0);
    using E = FbxGeometryElement;
    int idx;
    if (elem->GetMappingMode() == E::eByControlPoint)
      idx = (elem->GetReferenceMode() == E::eIndexToDirect)
                ? elem->GetIndexArray().GetAt(cpIdx)
                : cpIdx;
    else
      idx = (elem->GetReferenceMode() == E::eIndexToDirect)
                ? elem->GetIndexArray().GetAt(pvIdx)
                : pvIdx;
    return elem->GetDirectArray().GetAt(idx);
  };
  auto readV4 = [](auto *elem, int cpIdx, int pvIdx) -> FbxVector4 {
    if (!elem)
      return FbxVector4(0, 0, 0, 0);
    using E = FbxGeometryElement;
    int idx;
    if (elem->GetMappingMode() == E::eByControlPoint)
      idx = (elem->GetReferenceMode() == E::eIndexToDirect)
                ? elem->GetIndexArray().GetAt(cpIdx)
                : cpIdx;
    else
      idx = (elem->GetReferenceMode() == E::eIndexToDirect)
                ? elem->GetIndexArray().GetAt(pvIdx)
                : pvIdx;
    return elem->GetDirectArray().GetAt(idx);
  };

  auto readMaterialSlot = [&](int polygonIndex) -> int {
    if (!materialElem || nodeMaterialIndices.empty())
      return 0;
    using E = FbxGeometryElement;
    int idx = 0;
    if (materialElem->GetMappingMode() == E::eByPolygon) {
      idx = (materialElem->GetReferenceMode() == E::eIndexToDirect)
                ? materialElem->GetIndexArray().GetAt(polygonIndex)
                : polygonIndex;
    } else if (materialElem->GetMappingMode() == E::eAllSame) {
      idx = (materialElem->GetReferenceMode() == E::eIndexToDirect)
                ? materialElem->GetIndexArray().GetAt(0)
                : 0;
    }
    if (idx < 0 || idx >= static_cast<int>(nodeMaterialIndices.size()))
      return 0;
    return idx;
  };

  for (int p = 0; p < mesh->GetPolygonCount(); ++p) {
    const int polySize = mesh->GetPolygonSize(p);
    std::vector<unsigned> cornerIdx;
    cornerIdx.reserve(polySize);
    const int materialSlot = readMaterialSlot(p);

    for (int v = 0; v < polySize; ++v) {
      const int cpIndex = mesh->GetPolygonVertex(p, v);
      const int pvIndex = mesh->GetPolygonVertexIndex(p) + v;

      SimpleVertex out{};

      FbxVector4 P = mesh->GetControlPointAt(cpIndex);
      out.Position = TransformFBXPoint(nodeTransform, P);

      FbxVector4 N(0, 1, 0, 0);
      mesh->GetPolygonVertexNormal(p, v, N);
      N.Normalize();
      out.Normal = TransformFBXDirection(nodeTransform, N);

      if (uvElem && uvSetName) {
        int uvIdx = mesh->GetTextureUVIndex(p, v);
        FbxVector2 uv = (uvIdx >= 0) ? uvElem->GetDirectArray().GetAt(uvIdx)
                                     : readV2(uvElem, cpIndex, pvIndex);
        out.TextureCoordinate = {(float)uv[0], 1.0f - (float)uv[1]};
      } else {
        out.TextureCoordinate = {0.0f, 0.0f};
      }

      if (tanElem) {
        FbxVector4 T = readV4(tanElem, cpIndex, pvIndex);
        out.Tangent = TransformFBXDirection(nodeTransform, T);
      } else
        out.Tangent = {0, 0, 0};

      if (binElem) {
        FbxVector4 B = readV4(binElem, cpIndex, pvIndex);
        out.Bitangent = TransformFBXDirection(nodeTransform, B);
      } else
        out.Bitangent = {0, 0, 0};

      cornerIdx.push_back((unsigned)vertices.size());
      vertices.push_back(out);
    }

    for (int k = 1; k + 1 < polySize; ++k) {
      triangles.push_back(
          Triangle{cornerIdx[0], cornerIdx[k + 1], cornerIdx[k], materialSlot});
    }
  }

  if (mesh->GetElementTangentCount() == 0 || mesh->GetElementBinormalCount() == 0) {
    auto add = [](EU::Vector3 a, const EU::Vector3 &b) {
      a.x += b.x;
      a.y += b.y;
      a.z += b.z;
      return a;
    };
    auto sub = [](const EU::Vector3 &a, const EU::Vector3 &b) {
      return EU::Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
    };
    auto mul = [](const EU::Vector3 &a, float s) {
      return EU::Vector3(a.x * s, a.y * s, a.z * s);
    };

    for (const Triangle &triangle : triangles) {
      SimpleVertex &v0 = vertices[triangle.i0];
      SimpleVertex &v1 = vertices[triangle.i1];
      SimpleVertex &v2 = vertices[triangle.i2];

      EU::Vector3 e1 = sub(v1.Position, v0.Position);
      EU::Vector3 e2 = sub(v2.Position, v0.Position);

      float du1 = v1.TextureCoordinate.x - v0.TextureCoordinate.x;
      float dv1 = v1.TextureCoordinate.y - v0.TextureCoordinate.y;
      float du2 = v2.TextureCoordinate.x - v0.TextureCoordinate.x;
      float dv2 = v2.TextureCoordinate.y - v0.TextureCoordinate.y;

      float denom = du1 * dv2 - du2 * dv1;
      float r = (std::fabs(denom) < 1e-8f) ? 0.0f : 1.0f / denom;

      EU::Vector3 T = mul(EU::Vector3(e1.x * dv2 - e2.x * dv1,
                                      e1.y * dv2 - e2.y * dv1,
                                      e1.z * dv2 - e2.z * dv1),
                          r);
      EU::Vector3 B = mul(EU::Vector3(e2.x * du1 - e1.x * du2,
                                      e2.y * du1 - e1.y * du2,
                                      e2.z * du1 - e1.z * du2),
                          r);

      v0.Tangent = add(v0.Tangent, T);
      v1.Tangent = add(v1.Tangent, T);
      v2.Tangent = add(v2.Tangent, T);
      v0.Bitangent = add(v0.Bitangent, B);
      v1.Bitangent = add(v1.Bitangent, B);
      v2.Bitangent = add(v2.Bitangent, B);
    }
  }

  bool autoDetectMirror = true;
  bool forceFlipWinding = true;

  bool mirrored = true;
  if (autoDetectMirror) {
    FbxAMatrix geo;
    geo.SetT(node->GetGeometricTranslation(FbxNode::eSourcePivot));
    geo.SetR(node->GetGeometricRotation(FbxNode::eSourcePivot));
    geo.SetS(node->GetGeometricScaling(FbxNode::eSourcePivot));
    FbxAMatrix world = node->EvaluateGlobalTransform() * geo;

    FbxVector4 S = world.GetS();
    double detScale = S[0] * S[1] * S[2];
    mirrored = (detScale < 0.0);
  }

  if (mirrored || forceFlipWinding) {
    for (Triangle &triangle : triangles)
      std::swap(triangle.i1, triangle.i2);

    for (auto &v : vertices) {
      v.Normal = {v.Normal.x, v.Normal.y, v.Normal.z};
      v.Tangent = {v.Tangent.x, v.Tangent.y, v.Tangent.z};
      v.Bitangent = {v.Bitangent.x, v.Bitangent.y, v.Bitangent.z};
    }
  }

  auto dot3 = [](const EU::Vector3 &a, const EU::Vector3 &b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
  };
  auto norm3 = [](EU::Vector3 &v) {
    float l = std::sqrt(EU::EMax(1e-20f, v.x * v.x + v.y * v.y + v.z * v.z));
    v.x /= l;
    v.y /= l;
    v.z /= l;
  };
  auto sub3 = [](const EU::Vector3 &a, const EU::Vector3 &b) {
    return EU::Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
  };
  auto cross3 = [](const EU::Vector3 &a, const EU::Vector3 &b) {
    return EU::Vector3(
        a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
  };

  for (auto &v : vertices) {
    norm3(v.Normal);
    float dTN = dot3(v.Tangent, v.Normal);
    v.Tangent = sub3(v.Tangent,
                     EU::Vector3(v.Normal.x * dTN, v.Normal.y * dTN, v.Normal.z * dTN));
    norm3(v.Tangent);

    EU::Vector3 Bcalc = cross3(v.Normal, v.Tangent);
    float hand = (dot3(Bcalc, v.Bitangent) < 0.0f) ? -1.0f : 1.0f;
    v.Bitangent = {Bcalc.x * hand, Bcalc.y * hand, Bcalc.z * hand};
    norm3(v.Bitangent);
  }

  struct

      MaterialBucket {
    std::vector<SimpleVertex> vertices;
    std::vector<unsigned int> indices;
    std::unordered_map<unsigned int, unsigned int> remap;
  };
  std::vector<MaterialBucket> buckets(nodeMaterialIndices.size());
  auto remapVertex = [&](MaterialBucket &bucket, unsigned int sourceIndex) {
    auto it = bucket.remap.find(sourceIndex);
    if (it != bucket.remap.end())
      return it->second;
    const unsigned int targetIndex = static_cast<unsigned int>(bucket.vertices.size());
    bucket.vertices.push_back(vertices[sourceIndex]);
    bucket.remap[sourceIndex] = targetIndex;
    return targetIndex;
  };

  for (const Triangle &triangle : triangles) {
    const int materialSlot = (triangle.materialSlot >= 0 &&
                              triangle.materialSlot < static_cast<int>(buckets.size()))
                                 ? triangle.materialSlot
                                 : 0;
    MaterialBucket &bucket = buckets[materialSlot];
    bucket.indices.push_back(remapVertex(bucket, triangle.i0));
    bucket.indices.push_back(remapVertex(bucket, triangle.i1));
    bucket.indices.push_back(remapVertex(bucket, triangle.i2));
  }

  for (size_t i = 0; i < buckets.size(); ++i) {
    MaterialBucket &bucket = buckets[i];
    if (bucket.indices.empty())
      continue;

    MeshComponent mc;
    mc.m_name = node->GetName();
    if (buckets.size() > 1) {
      mc.m_name += "_mat" + std::to_string(i);
    }
    mc.m_materialIndex = nodeMaterialIndices[i];
    mc.m_vertex = std::move(bucket.vertices);
    mc.m_index = std::move(bucket.indices);
    mc.m_numVertex = static_cast<int>(mc.m_vertex.size());
    mc.m_numIndex = static_cast<int>(mc.m_index.size());
    m_meshes.push_back(std::move(mc));
  }
}

int
Model3D::ProcessFBXMaterials(FbxSurfaceMaterial *material) {
  ImportedMaterialInfo info;
  if (material) {
    if (FbxSurfaceLambert *lambert = FbxCast<FbxSurfaceLambert>(material)) {
      const FbxDouble3 diffuse = lambert->Diffuse.Get();
      info.baseColor = XMFLOAT4(static_cast<float>(diffuse[0]),
                                static_cast<float>(diffuse[1]),
                                static_cast<float>(diffuse[2]),
                                1.0f);
      info.alphaBlend = false;
    }
    if (FbxSurfacePhong *phong = FbxCast<FbxSurfacePhong>(material)) {
      const double shininess = phong->Shininess.Get();
      const double safeShininess = shininess < 1.0 ? 1.0 : shininess;
      double reflection = phong->ReflectionFactor.Get();
      if (reflection < 0.0)
        reflection = 0.0;
      if (reflection > 1.0)
        reflection = 1.0;
      info.roughness = static_cast<float>(1.0 / std::sqrt(safeShininess));
      info.metallic = static_cast<float>(reflection);
    }

    auto assignTexture = [&](const char *propertyName, int slot) {
      FbxProperty prop = material->FindProperty(propertyName);
      if (!prop.IsValid())
        return;

      const int fileTextureCount = prop.GetSrcObjectCount<FbxFileTexture>();
      for (int i = 0; i < fileTextureCount; ++i) {
        FbxFileTexture *texture =
            FbxCast<FbxFileTexture>(prop.GetSrcObject<FbxFileTexture>(i));
        if (!texture)
          continue;

        std::string path = texture->GetFileName();
        if (path.empty())
          path = texture->GetRelativeFileName();
        if (path.empty())
          path = texture->GetName();
        if (path.empty())
          continue;

        info.texturePaths[slot] = path;
        textureFileNames.push_back(path);
        break;
      }
    };

    assignTexture(FbxSurfaceMaterial::sDiffuse, 0);
    assignTexture(FbxSurfaceMaterial::sNormalMap, 1);
    assignTexture(FbxSurfaceMaterial::sBump, 1);
    assignTexture(FbxSurfaceMaterial::sReflection, 2);
    assignTexture(FbxSurfaceMaterial::sShininess, 3);
    assignTexture(FbxSurfaceMaterial::sEmissive, 5);
  }
  const int materialIndex = static_cast<int>(m_materialInfos.size());
  m_materialInfos.push_back(info);
  return materialIndex;
}

std::string
Model3D::GetBinaryCachePath() const {
  return m_filePath + ".wvmesh";
}

bool
Model3D::IsBinaryCacheUpToDate(const std::string &sourcePath,
                               const std::string &cachePath) const {
  ULONGLONG sourceWriteTime = 0;
  ULONGLONG cacheWriteTime = 0;

  if (!GetFileWriteTime(sourcePath, sourceWriteTime)) {
    return false;
  }

  if (!GetFileWriteTime(cachePath, cacheWriteTime)) {
    return false;
  }

  return cacheWriteTime >= sourceWriteTime;
}

bool
Model3D::LoadBinaryCache(const std::string &cachePath) {
  std::ifstream stream(cachePath, std::ios::binary);
  if (!stream.is_open()) {
    return false;
  }

  uint32_t magic = 0;
  uint32_t version = 0;
  uint32_t meshCount = 0;
  uint32_t textureCount = 0;
  uint32_t embeddedTextureCount = 0;
  uint32_t materialInfoCount = 0;

  stream.read(reinterpret_cast<char *>(&magic), sizeof(magic));
  stream.read(reinterpret_cast<char *>(&version), sizeof(version));
  stream.read(reinterpret_cast<char *>(&meshCount), sizeof(meshCount));
  stream.read(reinterpret_cast<char *>(&textureCount), sizeof(textureCount));
  stream.read(reinterpret_cast<char *>(&embeddedTextureCount),
              sizeof(embeddedTextureCount));
  stream.read(reinterpret_cast<char *>(&materialInfoCount), sizeof(materialInfoCount));

  if (!stream.good() || magic != kModelCacheMagic || version != kModelCacheVersion) {
    return false;
  }

  std::vector<MeshComponent> loadedMeshes;
  std::vector<std::string> loadedTextures;
  std::vector<EmbeddedTexture> loadedEmbeddedTextures;
  std::vector<ImportedMaterialInfo> loadedMaterialInfos;
  loadedMeshes.reserve(meshCount);
  loadedTextures.reserve(textureCount);
  loadedEmbeddedTextures.reserve(embeddedTextureCount);
  loadedMaterialInfos.reserve(materialInfoCount);

  for (uint32_t i = 0; i < textureCount; ++i) {
    std::string textureName;
    if (!ReadString(stream, textureName)) {
      return false;
    }
    loadedTextures.push_back(std::move(textureName));
  }

  for (uint32_t i = 0; i < embeddedTextureCount; ++i) {
    EmbeddedTexture embeddedTexture;
    if (!ReadString(stream, embeddedTexture.name)) {
      return false;
    }

    uint32_t dataSize = 0;
    stream.read(reinterpret_cast<char *>(&embeddedTexture.materialIndex),
                sizeof(embeddedTexture.materialIndex));
    stream.read(reinterpret_cast<char *>(&embeddedTexture.textureSlot),
                sizeof(embeddedTexture.textureSlot));
    stream.read(reinterpret_cast<char *>(&dataSize), sizeof(dataSize));
    if (!stream.good()) {
      return false;
    }

    embeddedTexture.data.resize(dataSize);
    if (dataSize > 0) {
      stream.read(reinterpret_cast<char *>(embeddedTexture.data.data()), dataSize);
    }
    if (!stream.good()) {
      return false;
    }
    loadedEmbeddedTextures.push_back(std::move(embeddedTexture));
  }

  for (uint32_t i = 0; i < materialInfoCount; ++i) {
    ImportedMaterialInfo materialInfo;
    stream.read(reinterpret_cast<char *>(&materialInfo.baseColor),
                sizeof(materialInfo.baseColor));
    stream.read(reinterpret_cast<char *>(&materialInfo.metallic),
                sizeof(materialInfo.metallic));
    stream.read(reinterpret_cast<char *>(&materialInfo.roughness),
                sizeof(materialInfo.roughness));
    stream.read(reinterpret_cast<char *>(&materialInfo.alphaBlend),
                sizeof(materialInfo.alphaBlend));
    if (!stream.good()) {
      return false;
    }
    for (std::string &texturePath : materialInfo.texturePaths) {
      if (!ReadString(stream, texturePath)) {
        return false;
      }
    }
    loadedMaterialInfos.push_back(materialInfo);
  }

  for (uint32_t i = 0; i < meshCount; ++i) {
    MeshComponent mesh;
    if (!ReadString(stream, mesh.m_name)) {
      return false;
    }

    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;
    stream.read(reinterpret_cast<char *>(&mesh.m_materialIndex),
                sizeof(mesh.m_materialIndex));
    stream.read(reinterpret_cast<char *>(&mesh.m_localTransform),
                sizeof(mesh.m_localTransform));
    stream.read(reinterpret_cast<char *>(&vertexCount), sizeof(vertexCount));
    stream.read(reinterpret_cast<char *>(&indexCount), sizeof(indexCount));
    if (!stream.good()) {
      return false;
    }

    mesh.m_vertex.resize(vertexCount);
    mesh.m_index.resize(indexCount);
    if (vertexCount > 0) {
      stream.read(reinterpret_cast<char *>(mesh.m_vertex.data()),
                  sizeof(SimpleVertex) * vertexCount);
    }
    if (indexCount > 0) {
      stream.read(reinterpret_cast<char *>(mesh.m_index.data()),
                  sizeof(unsigned int) * indexCount);
    }
    if (!stream.good()) {
      return false;
    }

    mesh.m_numVertex = static_cast<int>(vertexCount);
    mesh.m_numIndex = static_cast<int>(indexCount);
    loadedMeshes.push_back(std::move(mesh));
  }

  m_meshes = std::move(loadedMeshes);
  textureFileNames = std::move(loadedTextures);
  m_embeddedTextures = std::move(loadedEmbeddedTextures);
  m_materialInfos = std::move(loadedMaterialInfos);

  const std::wstring cachePathW(cachePath.begin(), cachePath.end());
  MESSAGE("ModelLoader", "BinaryCache", L"Loaded binary cache '" << cachePathW << L"'")
  return true;
}

bool
Model3D::SaveBinaryCache(const std::string &cachePath) const {
  std::ofstream stream(cachePath, std::ios::binary | std::ios::trunc);
  if (!stream.is_open()) {
    return false;
  }

  const uint32_t meshCount = static_cast<uint32_t>(m_meshes.size());
  const uint32_t textureCount = static_cast<uint32_t>(textureFileNames.size());
  const uint32_t embeddedTextureCount = static_cast<uint32_t>(m_embeddedTextures.size());
  const uint32_t materialInfoCount = static_cast<uint32_t>(m_materialInfos.size());

  stream.write(reinterpret_cast<const char *>(&kModelCacheMagic),
               sizeof(kModelCacheMagic));
  stream.write(reinterpret_cast<const char *>(&kModelCacheVersion),
               sizeof(kModelCacheVersion));
  stream.write(reinterpret_cast<const char *>(&meshCount), sizeof(meshCount));
  stream.write(reinterpret_cast<const char *>(&textureCount), sizeof(textureCount));
  stream.write(reinterpret_cast<const char *>(&embeddedTextureCount),
               sizeof(embeddedTextureCount));
  stream.write(reinterpret_cast<const char *>(&materialInfoCount),
               sizeof(materialInfoCount));

  for (const std::string &textureName : textureFileNames) {
    if (!WriteString(stream, textureName)) {
      return false;
    }
  }

  for (const EmbeddedTexture &embeddedTexture : m_embeddedTextures) {
    if (!WriteString(stream, embeddedTexture.name)) {
      return false;
    }

    const uint32_t dataSize = static_cast<uint32_t>(embeddedTexture.data.size());
    stream.write(reinterpret_cast<const char *>(&embeddedTexture.materialIndex),
                 sizeof(embeddedTexture.materialIndex));
    stream.write(reinterpret_cast<const char *>(&embeddedTexture.textureSlot),
                 sizeof(embeddedTexture.textureSlot));
    stream.write(reinterpret_cast<const char *>(&dataSize), sizeof(dataSize));
    if (dataSize > 0) {
      stream.write(reinterpret_cast<const char *>(embeddedTexture.data.data()), dataSize);
    }
    if (!stream.good()) {
      return false;
    }
  }

  for (const ImportedMaterialInfo &materialInfo : m_materialInfos) {
    stream.write(reinterpret_cast<const char *>(&materialInfo.baseColor),
                 sizeof(materialInfo.baseColor));
    stream.write(reinterpret_cast<const char *>(&materialInfo.metallic),
                 sizeof(materialInfo.metallic));
    stream.write(reinterpret_cast<const char *>(&materialInfo.roughness),
                 sizeof(materialInfo.roughness));
    stream.write(reinterpret_cast<const char *>(&materialInfo.alphaBlend),
                 sizeof(materialInfo.alphaBlend));
    if (!stream.good()) {
      return false;
    }
    for (const std::string &texturePath : materialInfo.texturePaths) {
      if (!WriteString(stream, texturePath)) {
        return false;
      }
    }
  }

  for (const MeshComponent &mesh : m_meshes) {
    if (!WriteString(stream, mesh.m_name)) {
      return false;
    }

    const uint32_t vertexCount = static_cast<uint32_t>(mesh.m_vertex.size());
    const uint32_t indexCount = static_cast<uint32_t>(mesh.m_index.size());
    stream.write(reinterpret_cast<const char *>(&mesh.m_materialIndex),
                 sizeof(mesh.m_materialIndex));
    stream.write(reinterpret_cast<const char *>(&mesh.m_localTransform),
                 sizeof(mesh.m_localTransform));
    stream.write(reinterpret_cast<const char *>(&vertexCount), sizeof(vertexCount));
    stream.write(reinterpret_cast<const char *>(&indexCount), sizeof(indexCount));

    if (vertexCount > 0) {
      stream.write(reinterpret_cast<const char *>(mesh.m_vertex.data()),
                   sizeof(SimpleVertex) * vertexCount);
    }
    if (indexCount > 0) {
      stream.write(reinterpret_cast<const char *>(mesh.m_index.data()),
                   sizeof(unsigned int) * indexCount);
    }

    if (!stream.good()) {
      return false;
    }
  }

  return stream.good();
}
