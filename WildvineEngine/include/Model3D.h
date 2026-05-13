#pragma once
#include "Prerequisites.h"
#include "IResource.h"
#include "MeshComponent.h"
#include "fbxsdk.h"

enum 
ModelType {
	OBJ,
	FBX
};

class 
Model3D : public IResource {
public:
	Model3D(const std::string& name, ModelType modelType) 
	: IResource(name), m_modelType(modelType), lSdkManager(nullptr), lScene(nullptr) {
		SetType(ResourceType::Model3D);
	}

	Model3D(const std::string& name,
		const SkyboxVertex vertices[],
		const unsigned int indices[]) : IResource(name) {
		MeshComponent mesh;
		mesh.m_skyVertex.assign(vertices, vertices + 8); 
		mesh.m_index.assign(indices, indices + 36); 
		mesh.m_numIndex = mesh.m_index.size();
		SetType(ResourceType::Model3D);
		m_meshes.push_back(mesh);
	}

	~Model3D() override;

	bool 
	load(const std::string& path) override;
	
	bool 
	init() override;
	
	void 
	unload() override;
	
	size_t 
	getSizeInBytes() const override;

	const std::vector<MeshComponent>& 
	GetMeshes() const { return m_meshes; }

	/* FBX MODEL LOADER*/
	bool
	InitializeFBXManager();

	std::vector<MeshComponent>
	LoadFBXModel(const std::string & filePath);

	std::vector<MeshComponent>
	LoadOBJModel(const std::string& filePath);

	void 
  ProcessFBXNode(FbxNode* node);

  void 
  ProcessFBXMesh(FbxNode* node);

  void 
  ProcessFBXMaterials(FbxSurfaceMaterial* material);

	std::vector<std::string> 
  GetTextureFileNames() const { return textureFileNames; }

private:
	std::string GetBinaryCachePath() const;
	bool IsBinaryCacheUpToDate(const std::string& sourcePath, const std::string& cachePath) const;
	bool LoadBinaryCache(const std::string& cachePath);
	bool SaveBinaryCache(const std::string& cachePath) const;

private:
	FbxManager* lSdkManager;
	FbxScene* lScene;
	std::vector<std::string> textureFileNames;
public:
	ModelType m_modelType;
	std::vector<MeshComponent> m_meshes;
};
