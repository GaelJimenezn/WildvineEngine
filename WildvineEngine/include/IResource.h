#pragma once
#include "Prerequisites.h"

enum class
	ResourceType {
	Unknown,
	Model3D,
	Texture,
	Sound,
	Shader,
	Material
};

enum class
	ResourceState {
	Unloaded,
	Loading,
	Loaded,
	Failed
};

class IResource {
public:
	IResource(const std::string& name)
		: m_name(name)
		, m_filePath("")
		, m_type(ResourceType::Unknown)
		, m_state(ResourceState::Unloaded)
		, m_id(GenerateID())
	{
	}
	virtual ~IResource() = default;

	// Crear recurso GPU
	virtual bool init() = 0;
	// Carga desde disco
	virtual bool load(const std::string& filename) = 0;
	// Liberar memoria
	virtual void unload() = 0;
	// Para profiler
	virtual size_t getSizeInBytes() const = 0;

	void SetPath(const std::string& path) { m_filePath = path; }
	void SetType(ResourceType t) { m_type = t; }
	void SetState(ResourceState s) { m_state = s; }


	const std::string& GetName() const { return m_name; }
	const std::string& GetPath() const { return m_filePath; }
	ResourceType GetType() const { return m_type; }
	ResourceState GetState() const { return m_state; }
	uint64_t GetID() const { return m_id; }

protected:
	std::string m_name;
	std::string m_filePath;
	ResourceType m_type;
	ResourceState m_state;
	uint64_t m_id;

private:
	static uint64_t GenerateID()
	{
		static uint64_t nextID = 1;
		return nextID++;
	}
};