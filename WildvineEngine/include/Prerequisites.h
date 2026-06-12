#pragma once
/**
 * @file Prerequisites.h
 * @brief Central include hub, common macros, vertex formats, constant buffers, and engine
 * enums.
 */

// Librerias STD
#include <string>
#include <sstream>
#include <vector>
#include <windows.h>
#include <xnamath.h>
#include <thread>
#include <memory>
#include <unordered_map>
#include <type_traits>
#include <array>

// Librerias DirectX
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include "Resource.h"
#include "resource.h"

// Third Party Libraries
#include "EngineUtilities/Vectors/Vector2.h"
#include "EngineUtilities/Vectors/Vector3.h"
#include "EngineUtilities\Memory\TSharedPointer.h"
#include "EngineUtilities\Memory\TWeakPointer.h"
#include "EngineUtilities\Memory\TStaticPtr.h"
#include "EngineUtilities\Memory\TUniquePtr.h"

// MACROS
/** @brief Releases a COM pointer when non-null and resets it to @c nullptr. */
#define SAFE_RELEASE(x) if(x != nullptr) x->Release(); x = nullptr;

/** @brief Writes a formatted resource-state message to the Visual Studio debug output. */
#define MESSAGE( classObj, method, state )   \
{                                            \
   std::wostringstream os_;                  \
   os_ \
   		<< classObj \
   		<< "::" \
   		<< method \
   		<< " : " \
   		<< "[CREATION OF RESOURCE " \
   		<< ": " \
   		<< state \
   		<< "] \n"; \
   OutputDebugStringW( os_.str().c_str() );  \
}

/** @brief Writes a formatted error message to the Visual Studio debug output. */
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

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
/**
 * @struct SimpleVertex
 * @brief Standard mesh vertex containing position, basis vectors, and texture
 * coordinates.
 */
struct
	SimpleVertex
{
    /** @brief Object-space vertex position. */
    EU::Vector3 Position;
    /** @brief Object-space vertex normal. */
    EU::Vector3 Normal;
    /** @brief Object-space tangent vector. */
    EU::Vector3 Tangent;
    /** @brief Object-space bitangent vector. */
    EU::Vector3 Bitangent;
    /** @brief Vertex UV coordinate. */
    EU::Vector2 TextureCoordinate;
};

/** @brief Compact position-only vertex used by skybox cube geometry. */
struct
	SkyboxVertex {
    /** @brief Position components in object space. */
    float x, y, z;
};


/** @brief Legacy constant buffer for values that rarely change. */
struct
	CBNeverChanges
{
    /** @brief View matrix. */
    XMMATRIX mView;
};

/** @brief Constant buffer used by skybox rendering. */
struct
	CBSkybox
{
    /** @brief Combined skybox view-projection matrix. */
    XMMATRIX mviewProj;
};

/** @brief Constant buffer updated when swap-chain or viewport size changes. */
struct
	CBChangeOnResize
{
    /** @brief Projection matrix. */
    XMMATRIX mProjection;
};

// Constant buffer used in the vertex and pixel shaders.  Align to
// 16?bytes as required by Direct3D constant buffers.
struct
	CBMain
{
    //XMFLOAT4X4 World;
    /** @brief View matrix. */
    XMFLOAT4X4 View;
    /** @brief Projection matrix. */
    XMFLOAT4X4 Projection;
    /** @brief Camera world position. */
    EU::Vector3 CameraPos;
    /** @brief Padding for constant-buffer alignment. */
    float pad0;
    /** @brief Main light direction. */
    EU::Vector3 LightDir;
    /** @brief Padding for constant-buffer alignment. */
    float pad1;
    /** @brief Main light color. */
    EU::Vector3 LightColor;
    /** @brief Padding for constant-buffer alignment. */
    float pad2;
};

/** @brief Legacy per-object/per-frame constant buffer used by actor rendering. */
struct
	CBChangesEveryFrame
{
    /** @brief World matrix. */
    XMMATRIX mWorld;
    /** @brief Per-mesh color factor. */
    XMFLOAT4 vMeshColor;
};

/**
 * @enum ExtensionType
 * @brief Texture file extension identifiers used by texture loading helpers.
 */
enum
	ExtensionType {
    /** @brief DirectDraw Surface texture. */
    DDS = 0,
    /** @brief PNG texture. */
    PNG = 1,
    /** @brief JPEG texture. */
    JPG = 2
};

/**
 * @enum ShaderType
 * @brief Shader stage identifiers used by shader-program loading.
 */
enum
	ShaderType {
    /** @brief Vertex shader stage. */
    VERTEX_SHADER = 0,
    /** @brief Pixel shader stage. */
    PIXEL_SHADER = 1
};

/**
 * @enum ComponentType
 * @brief Tipos de componentes disponibles en el juego.
 */
enum
	ComponentType {
    NONE = 0,     ///< Tipo de componente no especificado.
    TRANSFORM = 1,///< Componente de transformación.
    MESH = 2,     ///< Componente de malla.
    MATERIAL = 3,  ///< Componente de material.
    HIERARCHY = 4 ///< Componente de jerarquía.
};

