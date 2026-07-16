/**
 * @file Prerequisites.h
 * @brief Cabecera global de dependencias y tipos base del motor WildvineEngine.
 * @ingroup core
 *
 * Incluye todas las librerías de sistema, DirectX, EngineUtilities y define
 * los tipos de datos, estructuras de constant buffers, macros de depuración
 * y enumeraciones que todo el engine usa.
 */
#pragma once
//Librerias STD
#include <string>
#include <sstream>
#include <vector>
#include <windows.h>
#include <xnamath.h>
#include <thread>
#include <array>

#include <memory>
#include <unordered_map>
#include <type_traits>

//Librerias DirectX
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include "Resource.h"
#include "resource.h"


//Third Party Libraries
#include "EngineUtilities/Vectors/Vector2.h"
#include "EngineUtilities/Vectors/Vector3.h"
#include "EngineUtilities\Memory\TSharedPointer.h"
#include "EngineUtilities\Memory\TWeakPointer.h"
#include "EngineUtilities\Memory\TStaticPtr.h"
#include "EngineUtilities\Memory\TUniquePtr.h"
#include "Logger.h"

// MACROS

/**
 * @brief Libera de manera segura un recurso de DirectX.
 *
 * Si el puntero no es nulo, libera la memoria con Release()
 * y lo asigna a nullptr para evitar accesos inválidos.
 *
 * @param x Puntero al recurso que se va a liberar.
 */
#define SAFE_RELEASE(x) if(x != nullptr) x->Release(); x = nullptr;

 /**
  * @brief Macro para mostrar mensajes de creación de recursos en la ventana de
  * depuración.
  *
  * Formatea un mensaje con la clase, el método y el estado actual de la creación.
  *
  * @param classObj Nombre de la clase donde ocurre el evento.
  * @param method Nombre del método donde ocurre el evento.
  * @param state Estado del recurso (ejemplo: "OK", "FAILED").
  */
#define MESSAGE( classObj, method, state )   \
{                                            \
   std::wostringstream os_;                  \
   os_ << classObj << "::" << method << " : " \
       << "[CREATION OF RESOURCE " << ": " << state << "] \n"; \
   OutputDebugStringW( os_.str().c_str() );  \
   Logger::get().addW(LogLevel::Info, os_.str()); \
}

  /**
   * @brief Macro para registrar mensajes de error en la ventana de depuración.
   *
   * Captura información detallada de la clase, método y descripción del error.
   * Si ocurre un fallo durante el registro, captura la excepción y notifica.
   *
   * @param classObj Nombre de la clase donde ocurre el error.
   * @param method Nombre del método donde ocurre el error.
   * @param errorMSG Mensaje descriptivo del error.
   */
#define ERROR(classObj, method, errorMSG)                     \
{                                                             \
    try {                                                     \
        std::wostringstream os_;                              \
        os_ << L"ERROR : " << classObj << L"::" << method     \
            << L" : " << errorMSG << L"\n";                   \
        OutputDebugStringW(os_.str().c_str());                \
        Logger::get().addW(LogLevel::Error, os_.str());       \
    } catch (...) {                                           \
        OutputDebugStringW(L"Failed to log error message.\n");\
    }                                                         \
}

   /**
    * @brief Representa un vértice simple con posición y coordenadas de textura.
    */
struct 
SimpleVertex{
  EU::Vector3 Position;  /**< Coordenadas de posición del vértice (x, y, z). */
  EU::Vector3 Normal;  /**< Coordenadas de textura (u, v). */
  EU::Vector3 Tangent; /**< Vector normal del vértice (para iluminación). */
  EU::Vector3 Bitangent; /**< Vector tangente del vértice (para iluminación). */
  EU::Vector2 TextureCoordinate; /**< Vector bitangente del vértice (para iluminación). */
};

/**
 * @struct SkyboxVertex
 * @brief Vértice mínimo para el cubo del Skybox (solo posición XYZ).
 */
struct
SkyboxVertex {
  float x; /**< @brief Coordenada X del vértice. */
  float y; /**< @brief Coordenada Y del vértice. */
  float z; /**< @brief Coordenada Z del vértice. */
};


/**
 * @struct CBSkybox
 * @brief Constant buffer del shader de Skybox.
 *
 * Contiene la matriz view-projection pre-multiplicada para el renderizado
 * del Skybox. Se envía al shader de Skybox en cada frame.
 */
struct
CBSkybox
{
  XMMATRIX mviewProj; /**< @brief Matriz View*Projection para el shader de Skybox. */
};

/**
 * @struct LoadData
 * @brief Datos de geometría en CPU listos para cargarse a la GPU.
 *
 * Contiene el nombre del submesh, los vértices, los índices y los contadores
 * correspondientes. Es el formato intermedio entre el importador de modelos
 * (OBJ/FBX) y la carga final a los buffers de DirectX.
 */
struct
LoadData {
  std::string name;                /**< @brief Nombre del submesh. */
  std::vector<SimpleVertex> vertex; /**< @brief Lista de vértices. */
  std::vector<unsigned int> index;  /**< @brief Lista de índices. */
  int numVertex;                   /**< @brief Número total de vértices. */
  int numIndex;                    /**< @brief Número total de índices. */
};

/**
 * @brief Constantes que nunca cambian: contiene la matriz de vista.
 */
struct 
CBNeverChanges{
  XMMATRIX mView; /**< Matriz de vista usada en la cámara. */
};

/**
 * @brief Constantes que cambian al redimensionar la ventana.
 */
struct 
CBChangeOnResize{
  XMMATRIX mProjection; /**< Matriz de proyección ajustada al tamaño de la ventana. */
};

/**
 * @struct CBMain
 * @brief Constant buffer del shader PBR forward (legacy).
 *
 * Contiene las matrices de vista y proyección, la posición de la cámara
 * y los parámetros de la luz principal directional para el forward renderer.
 */
struct
CBMain
{
  XMFLOAT4X4 View;        /**< @brief Matriz de vista de la cámara. */
  XMFLOAT4X4 Projection;  /**< @brief Matriz de proyección de la cámara. */
  EU::Vector3 CameraPos;  /**< @brief Posición de la cámara en espacio mundo. */
  float pad0;             /**< @brief Padding para alineación a 16 bytes. */
  EU::Vector3 LightDir;   /**< @brief Dirección de la luz direccional principal. */
  float pad1;             /**< @brief Padding para alineación a 16 bytes. */
  EU::Vector3 LightColor; /**< @brief Color de la luz direccional principal. */
  float pad2;             /**< @brief Padding para alineación a 16 bytes. */
};


/**
 * @brief Constantes que cambian en cada frame.
 */
struct 
CBChangesEveryFrame{
  XMMATRIX mWorld;      /**< Matriz de mundo para transformar los objetos. */
  XMFLOAT4 vMeshColor;  /**< Color aplicado a la malla. */
};

/**
 * @brief Tipos de extensión soportados para las texturas.
 */
enum 
ExtensionType {
  DDS = 0, /**< Textura en formato DDS (DirectDraw Surface). */
  PNG = 1, /**< Textura en formato PNG (Portable Network Graphics). */
  JPG = 2  /**< Textura en formato JPG (Joint Photographic Experts Group). */
};

/**
 * @enum ShaderType
 * @brief Identifica el tipo de shader en el pipeline.
 */
enum 
ShaderType {
  VERTEX_SHADER = 0, /**< @brief Vertex Shader (VS). */
  PIXEL_SHADER  = 1  /**< @brief Pixel Shader (PS). */
};

/**
 * @enum ComponentType
 * @brief Tipos de componentes ECS reconocidos por el motor.
 */
enum
  ComponentType {
  NONE      = 0, /**< @brief Sin tipo de componente asignado. */
  TRANSFORM = 1, /**< @brief Componente de transformación (posición, rotación, escala). */
  MESH      = 2, /**< @brief Componente de malla (MeshRendererComponent). */
  MATERIAL  = 3, /**< @brief Componente de material. */
  HIERARCHY = 4  /**< @brief Componente de jerarquía de escena. */
};
