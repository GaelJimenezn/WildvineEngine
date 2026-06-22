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
  * @brief Macro para mostrar mensajes de creación de recursos en la ventana de depuración.
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
   os_ << classObj << "::" << method << " : " << "[CREATION OF RESOURCE " << ": " << state << "] \n"; \
   OutputDebugStringW( os_.str().c_str() );  \
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

struct
SkyboxVertex {
  float x, y, z;
};


struct CBSkybox
{
  XMMATRIX mviewProj;
};

struct
LoadData {
  std::string name;
  std::vector <SimpleVertex> vertex;
  std::vector <unsigned int> index;
  int numVertex;
  int numIndex;
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

struct CBMain
{
  //XMOFLOAT4X4 World;
  XMFLOAT4X4 View;
  XMFLOAT4X4 Projection;
  EU::Vector3 CameraPos;
  float pad0;
  EU::Vector3 LightDir;
  float pad1;
  EU::Vector3 LightColor;
  float pad2;
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

enum 
ShaderType {
  VERTEX_SHADER = 0,
  PIXEL_SHADER = 1
};

enum
  ComponentType {
  NONE = 0,    
  TRANSFORM = 1,
  MESH = 2,     
  MATERIAL = 3, 
  HIERARCHY = 4
};

