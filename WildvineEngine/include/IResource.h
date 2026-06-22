#pragma once
#include "Prerequisites.h"

/**
 * @enum ResourceType
 * @brief Define los tipos de recursos soportados por el sistema.
 */
enum class
ResourceType {
  Unknow,
  Model3D,
  Texture,
  Sound,
  Shader,
  Material
};

/**
 * @enum ResourceState
 * @brief Define el estado actual de un recurso.
 */
enum class
ResourceState {
  Unloaded,
  Loading,
  Loaded,
  Failed
};

/**
 * @class IResource
 * @brief Interfaz base para todos los recursos del sistema.
 *
 * Define el ciclo de vida de un recurso, incluyendo carga, inicialización,
 * liberación y monitoreo de estado.
 */
class
IResource {
public:
  /**
   * @brief Constructor que inicializa un recurso con un nombre.
   *
   * @param name Nombre del recurso.
   */
  IResource(const std::string& name) :
    m_name(name),
    m_filePath(""),
    m_type(ResourceType::Unknow),
    m_state(ResourceState::Unloaded),
    m_id(GenerateID())
  {
  }

  /**
   * @brief Destructor virtual.
   */
  virtual ~IResource() = default;

  //Cargar recurso CPU
  /**
   * @brief Inicializa el recurso en memoria (CPU/GPU).
   *
   * @return true si la inicialización fue exitosa.
   */
  virtual bool init() = 0;

  //Cargar desde disco duro
  /**
   * @brief Carga el recurso desde disco.
   *
   * @param filename Ruta del archivo.
   * @return true si la carga fue exitosa.
   */
  virtual bool load(const std::string& filename) = 0;

  //Liberar memoria
  /**
   * @brief Libera la memoria asociada al recurso.
   */
  virtual void unload() = 0;

  //Para profiler
  /**
   * @brief Obtiene el tamaño del recurso en bytes.
   *
   * @return size_t Tamaño en memoria.
   */
  virtual size_t getSizeInBytes() const = 0;

  /**
   * @brief Establece la ruta del archivo del recurso.
   *
   * @param path Ruta del archivo.
   */
  void
  SetPath(const std::string& path) { m_filePath = path; }

  /**
   * @brief Establece el tipo del recurso.
   *
   * @param t Tipo de recurso.
   */
  void
  SetType(ResourceType t) { m_type = t; }

  /**
   * @brief Establece el estado del recurso.
   *
   * @param s Estado del recurso.
   */
  void
  SetState(ResourceState s) { m_state = s; }

  /**
   * @brief Obtiene el nombre del recurso.
   * @return Referencia constante al nombre.
   */
  const std::string& GetName() const { return m_name; }

  /**
   * @brief Obtiene la ruta del recurso.
   * @return Referencia constante a la ruta.
   */
  const std::string& GetPath() const { return m_filePath; }

  /**
   * @brief Obtiene el tipo del recurso.
   * @return Tipo del recurso.
   */
  ResourceType GetType() const { return m_type; }

  /**
   * @brief Obtiene el estado del recurso.
   * @return Estado del recurso.
   */
  ResourceState GetState() const { return m_state; }

  /**
   * @brief Obtiene el identificador único del recurso.
   * @return ID del recurso.
   */
  uint64_t GetID() const { return m_id; }

protected:
  /** @brief Nombre del recurso. */
  std::string m_name;

  /** @brief Ruta del archivo del recurso. */
  std::string m_filePath;

  /** @brief Tipo del recurso. */
  ResourceType m_type;

  /** @brief Estado actual del recurso. */
  ResourceState m_state;

  /** @brief Identificador único del recurso. */
  uint64_t m_id;

private:
  /**
   * @brief Genera un ID único incremental.
   *
   * @return uint64_t Nuevo ID generado.
   */
  static uint64_t GenerateID()
  {
    static uint64_t nextID = 1;
    return nextID++;
  }
};