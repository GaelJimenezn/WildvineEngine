#pragma once
#include "Prerequisites.h"

/**
 * @enum ResourceType
 * @brief Broad engine resource categories used for caches and editor filtering.
 */
enum class
	ResourceType {
	/** @brief Resource type has not been assigned. */
	Unknown,
	/** @brief 3D model resource. */
	Model3D,
	/** @brief Texture or shader-resource-view resource. */
	Texture,
	/** @brief Audio resource. */
	Sound,
	/** @brief Shader program resource. */
	Shader,
	/** @brief Material resource. */
	Material
};

/**
 * @enum ResourceState
 * @brief Runtime lifecycle state for loadable resources.
 */
enum class
	ResourceState {
	/** @brief Resource has no loaded payload. */
	Unloaded,
	/** @brief Resource is currently loading. */
	Loading,
	/** @brief Resource has successfully loaded and initialized. */
	Loaded,
	/** @brief Resource failed to load or initialize. */
	Failed
};

/**
 * @class IResource
 * @brief Abstract base class for named loadable engine resources.
 *
 * Resources expose a shared lifecycle contract for disk loading, runtime initialization,
 * unloading, memory reporting, and metadata used by @c ResourceManager.
 */
class
	IResource {
public:
	/**
	 * @brief Creates resource metadata and assigns a unique process-local id.
	 * @param name Human-readable/cache name for the resource.
	 */
	IResource(const std::string& name)
		: m_name(name)
		/**
		 * @brief Método m_filePath.
		 *
		 * @return Retorna el resultado de la operación.
		 */
		,
			m_filePath("")
		/**
		 * @brief Método m_type.
		 *
		 * @param Unknown Parámetro del método.
		 * @return Retorna el resultado de la operación.
		 */
		,
			m_type(ResourceType::Unknown)
		/**
		 * @brief Método m_state.
		 *
		 * @param Unloaded Parámetro del método.
		 * @return Retorna el resultado de la operación.
		 */
		,
			m_state(ResourceState::Unloaded)
		/**
		 * @brief Método m_id.
		 *
		 * @param GenerateID Parámetro del método.
		 * @return Retorna el resultado de la operación.
		 */
		,
			m_id(GenerateID())
	{
	}
	/** @brief Virtual destructor for polymorphic resource cleanup. */
	virtual
		~IResource() = default;

	/** @brief Creates runtime or GPU payload after source data is loaded. */
	virtual bool
		init() = 0;
	/** @brief Loads source data from disk. */
	virtual bool
		load(const std::string& filename) = 0;
	/** @brief Releases CPU/GPU payload while keeping metadata valid. */
	virtual void
		unload() = 0;
	/** @brief Reports approximate payload memory for profiling. */
	virtual size_t
		getSizeInBytes() const = 0;

	/** @brief Sets the canonical source path for this resource. */
	void
		SetPath(const std::string& path) { m_filePath = path; }
	/** @brief Sets the resource category. */
	void
		SetType(ResourceType t) { m_type = t; }
	/** @brief Sets the current lifecycle state. */
	void
		SetState(ResourceState s) { m_state = s; }


	/** @brief Returns the resource cache/display name. */
	const std::string&
		GetName() const { return m_name; }
	/** @brief Returns the source path assigned to the resource. */
	const std::string&
		GetPath() const { return m_filePath; }
	/** @brief Returns the resource category. */
	ResourceType
		GetType() const { return m_type; }
	/** @brief Returns the current lifecycle state. */
	ResourceState
		GetState() const { return m_state; }
	/** @brief Returns the unique process-local resource id. */
	uint64_t
		GetID() const { return m_id; }

protected:
	/** @brief Human-readable/cache name. */
	std::string m_name;
	/** @brief Canonical source path, when available. */
	std::string m_filePath;
	/** @brief Broad resource category. */
	ResourceType m_type;
	/** @brief Current loading/runtime state. */
	ResourceState m_state;
	/** @brief Unique process-local resource id. */
	uint64_t m_id;

private:
	/** @brief Generates monotonically increasing process-local ids. */
	static uint64_t
		GenerateID()
	{
		static uint64_t nextID = 1;
		return nextID++;
	}
};
