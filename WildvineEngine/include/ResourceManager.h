/**
 * @file ResourceManager.h
 * @brief Declara el gestor centralizado de recursos del motor (Singleton + Flyweight).
 * @ingroup core
 *
 * ResourceManager implementa el patrÃ³n Flyweight para evitar cargar el mismo
 * recurso dos veces. Cualquier subsistema puede solicitar un recurso por clave
 * y recibirÃ¡ la instancia existente o la cargarÃ¡ desde disco si no existe.
 */
#pragma once
#include "Prerequisites.h"
#include "IResource.h"

/**
 * @class ResourceManager
 * @brief Gestiona la carga, almacenamiento y liberaciÃ³n de recursos.
 *
 * Implementa el patrÃ³n Singleton y actÃºa como cachÃ© de recursos utilizando
 * el patrÃ³n Flyweight para evitar duplicaciÃ³n de instancias.
 */
class
ResourceManager {
public:
  /**
   * @brief Constructor por defecto.
   */
  ResourceManager() = default;

  /**
   * @brief Destructor por defecto.
   */
  ~ResourceManager() = default;

  // Singleton
  /**
   * @brief Obtiene la instancia Ãºnica del ResourceManager.
   *
   * @return Referencia a la instancia singleton.
   */
  static ResourceManager &
  getInstance() {
    static ResourceManager instance;
    return instance;
  }

  /**
   * @brief Constructor de copia eliminado.
   */
  ResourceManager(const ResourceManager &) = delete;

  /**
   * @brief Operador de asignaciÃ³n eliminado.
   */
  ResourceManager &operator=(const ResourceManager &) = delete;

  /// Obtener o cargar un recurso de tipo T (T debe heredar de IResource).
  /**
   * @brief Obtiene un recurso del cachÃ© o lo carga si no existe.
   *
   * @tparam T Tipo de recurso (debe heredar de IResource).
   * @tparam Args Argumentos adicionales para la construcciÃ³n del recurso.
   * @param key Identificador Ãºnico del recurso.
   * @param filename Ruta del archivo a cargar.
   * @param args Argumentos adicionales.
   * @return std::shared_ptr<T> Recurso cargado o existente.
   */
  template <typename T, typename... Args>
  /** @brief Declara o ejecuta GetOrLoad. */
  std::shared_ptr<T>
  GetOrLoad(const std::string &key, const std::string &filename, Args &&...args) {
    static_assert(std::is_base_of<IResource, T>::value, "T debe heredar de IResource");
    // 1. Â¿Ya existe el recurso en el cachÃ©?
    auto it = m_resources.find(key);
    if (it != m_resources.end()) {
      // Intentar castear al tipo correcto
      auto existing = std::dynamic_pointer_cast<T>(it->second);
      if (existing && existing->GetState() == ResourceState::Loaded) {
        return existing; // Flyweight: reutilizamos la instancia
      }
    }

    // 2. No existe o no estÃ¡ cargado -> crearlo y cargarlo
    std::shared_ptr<T> resource = std::make_shared<T>(key, std::forward<Args>(args)...);

    if (!resource->load(filename)) {
      // Puedes manejar errores mÃ¡s fino aquÃ­
      return nullptr;
    }

    if (!resource->init()) {
      return nullptr;
    }

    // 3. Guardar en el cachÃ© y devolver
    m_resources[key] = resource;
    return resource;
  }

  /// Obtener un recurso ya cargado, sin cargarlo si no existe.
  /**
   * @brief Obtiene un recurso ya cargado desde el cachÃ©.
   *
   * @tparam T Tipo de recurso.
   * @param key Identificador del recurso.
   * @return std::shared_ptr<T> Recurso encontrado o nullptr si no existe.
   */
  template <typename T>
  /** @brief Declara o ejecuta Get. */
  std::shared_ptr<T>
  Get(const std::string &key) const {
    auto it = m_resources.find(key);
    if (it == m_resources.end())
      return nullptr;

    return std::dynamic_pointer_cast<T>(it->second);
  }

  /// Liberar un recurso especÃ­fico
  /**
   * @brief Libera un recurso especÃ­fico del cachÃ©.
   *
   * @param key Identificador del recurso.
   */
  void
  Unload(const std::string &key) {
    auto it = m_resources.find(key);
    if (it != m_resources.end()) {
      it->second->unload();
      m_resources.erase(it);
    }
  }

  /// Liberar todos los recursos
  /**
   * @brief Libera todos los recursos almacenados en el cachÃ©.
   */
  void
  UnloadAll() {
    for (auto &[key, res] : m_resources) {
      if (res) {
        res->unload();
      }
    }
    m_resources.clear();
  }

private:
  /**
   * @brief Contenedor de recursos cargados.
   *
   * Mapea una clave Ãºnica a una instancia compartida de IResource.
   */
  std::unordered_map<std::string, std::shared_ptr<IResource>> m_resources;
};
