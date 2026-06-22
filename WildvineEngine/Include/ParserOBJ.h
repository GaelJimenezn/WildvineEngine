#pragma once
#include "Prerequisites.h" 
#include <string>
#include <vector>

/**
 * @file ParserOBJ.h
 * @brief Define una interfaz de cargador .obj personalizada que simula la
 * interfaz de la biblioteca 'objl' para ser compatible con ModelLoader.cpp.
 * @author [Jose Sebastian/Usuario]
 * @date 5 de noviembre de 2025
 */

 /**
  * @namespace objl
  * @brief Namespace que simula la biblioteca 'objl' para que ModelLoader.cpp
  * pueda compilar sin cambios.
  */
namespace 
objl
{
  /**
   * @struct Vector3
   * @brief Estructura de 3 componentes (X, Y, Z) compatible con lo que
   * ModelLoader.cpp espera leer de objl::Vertex::Position.
   */
  struct 
  Vector3 {
    float X; /**< Componente X */
    float Y; /**< Componente Y */
    float Z; /**< Componente Z */
  };

  /**
   * @struct Vector2
   * @brief Estructura de 2 componentes (X, Y) compatible con lo que
   * ModelLoader.cpp espera leer de objl::Vertex::TextureCoordinate.
   */
  struct 
  Vector2 {
    float X; /**< Componente X (o U) */
    float Y; /**< Componente Y (o V) */
  };

  /**
   * @struct Vertex
   * @brief Estructura de vértice compatible con lo que ModelLoader.cpp
   * espera leer de objl::Vertex. Contiene Posición, Normal y Textura.
   */
  struct 
  Vertex {
    Vector3 Position;        /**< Posición del vértice (X, Y, Z) */
    Vector3 Normal;          /**< Normal del vértice (X, Y, Z) */
    Vector2 TextureCoordinate; /**< Coordenada de textura del vértice (U, V) */
  };

  /**
   * @class Loader
   * @brief Clase principal que simula la interfaz 'objl::Loader'.
   * ModelLoader.cpp crea una instancia de esta clase. Internamente,
   * utiliza nuestro propio código de parsing (en ParserOBJ.cpp)
   * para llenar los datos que ModelLoader.cpp espera.
   */
  class 
  Loader
  {
  public:
    // --- Miembros Públicos (Leídos por ModelLoader.cpp) ---

    /**
     * @brief Vector que almacena los vértices únicos cargados.
     * Nuestro parser personalizado llena este vector para que
     * ModelLoader.cpp pueda leerlo.
     */
    std::vector<Vertex> LoadedVertices;

    /**
     * @brief Vector que almacena los índices del modelo.
     * Nuestro parser personalizado llena este vector para que
     * ModelLoader.cpp pueda leerlo.
     */
    std::vector<unsigned int> LoadedIndices;


    // --- Métodos Públicos (Llamados por ModelLoader.cpp) ---

    /**
     * @brief Carga y parsea un archivo .obj desde una ruta.
     * Esta es la función principal que ModelLoader.cpp llama.
     * @param fileName Ruta al archivo .obj (ej. "Assets/Link.obj").
     * @return true si la carga fue exitosa (se encontraron vértices),
     * @return false si la carga falló (archivo no encontrado o vacío).
     */
    bool 
    LoadFile(std::string fileName);

  private:
    /**
     * @brief Función de parsing interna y privada.
     * Contiene la lógica de nuestro parser personalizado
     * (leer el archivo, manejar el caché de vértices, etc.).
     * @note Esta función es llamada por LoadFile() y llena
     * LoadedVertices y LoadedIndices.
     * @param fileName Ruta al archivo .obj.
     */
    void 
    Parse(std::string fileName);
  };
}