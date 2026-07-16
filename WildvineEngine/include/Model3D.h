/**
 * @file Model3D.h
 * @brief Declara la API pública de Model3D dentro de WildvineEngine.
 */
#pragma once
#include "Prerequisites.h"
#include "fbxsdk.h"
#include "IResource.h"
#include "MeshComponent.h"

/**
 * @enum ModelType
 * @brief Tipos de modelos 3D soportados por el sistema.
 */
enum
  ModelType {
  OBJ,  ///< Modelo en formato OBJ.
  FBX,  ///< Modelo en formato FBX.
  GLTF  ///< Modelo en formato GLTF / GLB.
};

/** @brief Declara struct EmbeddedTexture. */
struct
EmbeddedTexture {
    std::string name;
    std::vector<unsigned char> data;
    int materialIndex = 0;
    int textureSlot = 0;
};

/** @brief Declara struct ImportedMaterialInfo. */
struct
ImportedMaterialInfo {
    XMFLOAT4 baseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    float metallic = 0.0f;
    float roughness = 1.0f;
    bool alphaBlend = false;
    std::string texturePaths[6];
};

/** @brief Declara class Model3D. */
class
Model3D : public IResource {

public:
  /**
   * @brief Constructor que crea un recurso de modelo 3D y lo carga automáticamente.
   * @param name Nombre del recurso/modelo.
   * @param modelType Tipo de modelo (OBJ o FBX).
   */
  Model3D(const std::string& name, ModelType modelType)
        : IResource(name),
        m_modelType(modelType), 
        lSdkManager(nullptr), 
        lScene(nullptr) {
        SetType(ResourceType::Model3D);
        load(name);
  }

  Model3D(const std::string& name,
    const SkyboxVertex vertices[],
    const unsigned int indices[]) : IResource(name) {
    MeshComponent mesh;
    mesh.m_skyVertex.assign(vertices, vertices + 8);
    mesh.m_index.assign(indices, indices + 36);
    mesh.m_numIndex = static_cast<int>(mesh.m_index.size());
    SetType(ResourceType::Model3D);
    m_meshes.push_back(mesh);
  }

  /**
   * @brief Destructor por defecto.
   */
  ~Model3D();
  //~Model3D() = override;

  /**
   * @brief Carga un modelo desde ruta especificada.
   * @param path Ruta del archivo de modelo.
   * @return true si se cargó correctamente, false si falló.
   */
  bool
  load(const std::string& path) override;

  /**
   * @brief Inicializa recursos adicionales necesarios para el modelo.
   * @return true si la inicialización fue exitosa.
   */
  bool
  init() override;

  /**
   * @brief Descarga y libera los recursos del modelo.
   */
  void
  unload() override;

  /**
   * @brief Obtiene el tamaño del modelo en memoria.
   * @return Tamaño estimado en bytes.
   */
  size_t
  getSizeInBytes() const override;

  /**
   * @brief Retorna la lista de mallas procesadas del modelo.
   * @return Vector de MeshComponent.
   */
  const std::vector<MeshComponent>&
  GetMeshes() const { return m_meshes; }

  /**
   * @brief Inicializa el administrador de FBX (FbxManager).
   * @return true si se inicializó correctamente.
   */
  bool
  InitializeFBXManager();

  /**
   * @brief Carga un modelo FBX desde archivo.
   * @param filePath Ruta del archivo FBX.
   * @return Vector de mallas generadas.
   */
  std::vector<MeshComponent>
    LoadFBXModel(const std::string& filePath);

  /** @brief Declara o ejecuta LoadOBJModel. */
  std::vector<MeshComponent>
  LoadOBJModel(const std::string& filePath);

  /** @brief Declara o ejecuta LoadGLTFModel. */
  std::vector<MeshComponent>
  LoadGLTFModel(const std::string& filePath);

  /**
   * @brief Procesa un nodo del archivo FBX.
   * @param node Nodo actual a procesar.
   */
  void
  ProcessFBXNode(FbxNode* node);

  /**
   * @brief Procesa una malla encontrada en un nodo FBX.
   * @param node Nodo que contiene la malla.
   */
  void
  ProcessFBXMesh(FbxNode* node);

  /**
   * @brief Procesa los materiales de una malla FBX.
   * @param material Material de superficie FBX.
   */
  int
  ProcessFBXMaterials(FbxSurfaceMaterial* material);

  /**
   * @brief Obtiene los nombres de las texturas asociadas al modelo.
   * @return Vector de rutas/nombres de texturas.
   */
  std::vector<std::string>
  GetTextureFileNames() const { return textureFileNames; }

  /** @brief Declara o ejecuta GetEmbeddedTextures. */
  const std::vector<EmbeddedTexture>&
  GetEmbeddedTextures() const { return m_embeddedTextures; }

  /** @brief Declara o ejecuta GetMaterialInfos. */
  const std::vector<ImportedMaterialInfo>&
  GetMaterialInfos() const { return m_materialInfos; }

private:
  /** @brief Declara o ejecuta GetBinaryCachePath. */
  std::string
  GetBinaryCachePath() const;

  /** @brief Declara o ejecuta IsBinaryCacheUpToDate. */
  bool
  IsBinaryCacheUpToDate(const std::string& sourcePath,
    const std::string& cachePath) const;

  /** @brief Declara o ejecuta LoadBinaryCache. */
  bool
  LoadBinaryCache(const std::string& cachePath);

  /** @brief Declara o ejecuta SaveBinaryCache. */
  bool
  SaveBinaryCache(const std::string& cachePath) const;

private:
  FbxManager* lSdkManager;            ///< Administrador principal del SDK de FBX.
  FbxScene* lScene;                   ///< Escena cargada desde el archivo FBX.
  ///< Lista de texturas usadas por el modelo.
  std::vector<std::string > textureFileNames;
  std::vector<EmbeddedTexture> m_embeddedTextures;
  std::vector<ImportedMaterialInfo> m_materialInfos;

public:
  ModelType m_modelType;              ///< Tipo del modelo cargado (OBJ o FBX).
  std::vector<MeshComponent> m_meshes; ///< Mallas extraídas del archivo del modelo.
};
