#pragma once
#include "Prerequisites.h"
#include "IResource.h"
#include "MeshComponent.h"
#include "fbxsdk.h"

/**
 * @enum ModelType
 * @brief Supported source model formats for @c Model3D.
 */
enum
	ModelType {
	/** @brief Wavefront OBJ source. */
	OBJ,
	/** @brief Autodesk FBX source. */
	FBX
};

/**
 * @class Model3D
 * @brief Loadable 3D model resource that extracts engine mesh components.
 *
 * Model3D implements @c IResource for model files and keeps CPU-side @c MeshComponent
 * data that can later be converted into GPU @c Mesh buffers by the renderer/application.
 */
class
	Model3D : public IResource {
public:
	/**
	 * @brief Creates and immediately loads a model resource from a file path/name.
	 * @param name Resource name and source path used by the current implementation.
	 * @param modelType Source format selector.
	 */
	Model3D(const std::string& name, ModelType modelType)
		: IResource(name), m_modelType(modelType), lSdkManager(nullptr), lScene(nullptr) {
		SetType(ResourceType::Model3D);
		load(name);
	}

	/**
	 * @brief Creates an in-memory cube model from skybox vertex/index arrays.
	 * @param name Resource name.
	 * @param vertices Eight skybox cube vertices.
	 * @param indices Thirty-six cube indices.
	 */
	Model3D(const std::string& name,
		const SkyboxVertex vertices[], const unsigned int indices[]) : IResource(name) {
		MeshComponent mesh;
		mesh.m_skyVertex.assign(vertices, vertices + 8);
		mesh.m_index.assign(indices, indices + 36);
		mesh.m_numIndex = mesh.m_index.size();
		SetType(ResourceType::Model3D);
		m_meshes.push_back(mesh);
	}

	/** @brief Default destructor; call unload() for deterministic resource cleanup. */
	~Model3D() = default;

	/** @brief Loads model data from @p path according to @c m_modelType. */
	bool
		load(const std::string& path) override;

	/** @brief Initializes the model resource after loading source data. */
	bool
		init() override;

	/** @brief Releases model payload and FBX-side state owned by the resource. */
	void
		unload() override;

	/** @brief Returns approximate CPU mesh payload size in bytes. */
	size_t
		getSizeInBytes() const override;

	/** @brief Returns extracted mesh components for GPU mesh construction. */
	const std::vector<MeshComponent>&
		GetMeshes() const { return m_meshes; }

	/* FBX MODEL LOADER*/
	/** @brief Initializes the FBX SDK manager and scene objects used by FBX loading. */
	bool
		InitializeFBXManager();

	/** @brief Loads an FBX file and returns extracted mesh components. */
	std::vector<MeshComponent>
		LoadFBXModel(const std::string& filePath);

	/** @brief Recursively processes an FBX scene node and its children. */
	void
		ProcessFBXNode(FbxNode* node);

	/** @brief Extracts mesh geometry from an FBX node into @c m_meshes. */
	void
		ProcessFBXMesh(FbxNode* node);

	/** @brief Extracts texture/material references from an FBX material. */
	void
		ProcessFBXMaterials(FbxSurfaceMaterial* material);

	/** @brief Returns texture file names discovered during FBX material parsing. */
	std::vector<std::string>
		GetTextureFileNames() const { return textureFileNames; }
private:
	/** @brief FBX SDK manager used while importing FBX files. */
	FbxManager* lSdkManager;
	/** @brief FBX scene loaded by the importer. */
	FbxScene* lScene;
	/** @brief Texture filenames discovered in model materials. */
	std::vector<std::string> textureFileNames;
public:
	/** @brief Source format selected for this model. */
	ModelType m_modelType;
	/** @brief CPU-side meshes extracted from the source model. */
	std::vector<MeshComponent> m_meshes;
};
