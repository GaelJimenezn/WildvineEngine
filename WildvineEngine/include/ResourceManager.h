#pragma once
#include "Prerequisites.h"
#include "IResource.h"

/**
 * @class ResourceManager
 * @brief Singleton cache for loading, reusing, and unloading engine resources.
 *
 * Resources are indexed by string key and stored as @c std::shared_ptr<IResource>.
 * Typed accessors use @c std::dynamic_pointer_cast so callers can request concrete
 * resource types while the manager keeps a single heterogeneous cache.
 */
class
	ResourceManager {
public:
	/** @brief Creates an empty resource cache. */
	ResourceManager()  = default;
	/**
	 * @brief Does not automatically unload resources; call UnloadAll() for deterministic
	 * cleanup.
	 */
	~ResourceManager() = default;

	// Singleton
	/** @brief Returns the process-local resource manager singleton. */
	static ResourceManager&
		getInstance() {
		static ResourceManager instance;
		return instance;
	}

	/** @brief Copying is disabled because the manager owns a singleton cache. */
	ResourceManager(const ResourceManager&) = delete;
	/** @brief Assignment is disabled because the manager owns a singleton cache. */
	ResourceManager& operator=(const ResourceManager&) = delete;

	/**
	 * @brief Returns an already loaded resource or creates, loads, initializes, and caches
	 * it.
	 * @tparam T Concrete resource type derived from @c IResource.
	 * @tparam Args Extra constructor arguments forwarded after the key.
	 * @param key Cache key and resource name.
	 * @param filename Source filename passed to @c T::load().
	 * @param args Extra construction arguments for @c T.
	 * @return Loaded resource instance, or @c nullptr if load/init fails.
	 */
	template<typename T, typename... Args> std::shared_ptr<T>
		GetOrLoad(const std::string& key,
                               const std::string& filename,
                               Args&&... args) {
		static_assert(std::is_base_of<IResource, T>::value,
                      "T debe heredar de IResource");
		// 1. ¿Ya existe el recurso en el caché?
		auto it = m_resources.find(key);
		if (it != m_resources.end()) {
			// Intentar castear al tipo correcto
			auto existing = std::dynamic_pointer_cast<T>(it->second);
			if (existing && existing->GetState() == ResourceState::Loaded) {
				return existing; // Flyweight: reutilizamos la instancia
			}
		}

		// 2. No existe o no está cargado -> crearlo y cargarlo
		std::shared_ptr<T> resource = std::make_shared<T>(key, std::forward<Args>(args)...);

		if (!resource->load(filename)) {
			// Puedes manejar errores más fino aquí
			return nullptr;
		}

		if (!resource->init()) {
			return nullptr;
		}

		// 3. Guardar en el caché y devolver
		m_resources[key] = resource;
		return resource;
	}

	/**
	 * @brief Returns a cached resource without loading a missing one.
	 * @tparam T Concrete resource type expected by the caller.
	 * @param key Cache key to search.
	 * @return Cached resource cast to @c T, or @c nullptr if absent/incompatible.
	 */
	template<typename T> std::shared_ptr<T>
		Get(const std::string& key) const
	{
		auto it = m_resources.find(key);
		if (it == m_resources.end()) return nullptr;

		return std::dynamic_pointer_cast<T>(it->second);
	}

	/**
	 * @brief Unloads and erases one cached resource.
	 * @param key Cache key to remove.
	 */
	void
		Unload(const std::string& key)
	{
		auto it = m_resources.find(key);
		if (it != m_resources.end()) {
			it->second->unload();
			m_resources.erase(it);
		}
	}

	/** @brief Unloads every cached resource and clears the cache. */
	void
		UnloadAll()
	{
		for (auto& [key, res] : m_resources) {
			if (res) {
				res->unload();
			}
		}
		m_resources.clear();
	}

private:
	/** @brief Heterogeneous resource cache indexed by stable string key. */
	std::unordered_map<std::string, std::shared_ptr<IResource>> m_resources;
};
