#pragma once
#include "Prerequisites.h"

class Mesh;
class MaterialInstance;

/**
 * @enum MaterialDomain
 * @brief Define el dominio del material para determinar su orden y forma de renderizado en el pipeline.
 */
enum class
MaterialDomain {
	Opaque = 0,   /**< El material es completamente opaco. */
	Masked,       /**< El material tiene recortes (alpha cutout), útil para follaje o rejas. */
	Transparent   /**< El material es translúcido y requiere mezcla de colores (blending). */
};

/**
 * @enum BlendMode
 * @brief Especifica el tipo de mezcla de color (blending) a utilizar en la GPU.
 */
enum class
BlendMode {
	Opaque = 0,             /**< Sobrescribe el color en el render target. Sin mezcla. */
	Alpha,                  /**< Mezcla estándar usando el canal alfa (transparencia tradicional). */
	Additive,               /**< Suma los colores (útil para efectos de luz, fuego o magia). */
	PremultipliedAlpha      /**< Mezcla usando alfa premultiplicado para evitar bordes oscuros. */
};

/**
 * @enum RenderPassType
 * @brief Identifica el pase de renderizado actual.
 */
enum class
RenderPassType {
	Shadow = 0,   /**< Pase de renderizado para mapas de sombras. */
	Opaque,       /**< Pase de renderizado para la geometría opaca. */
	Skybox,       /**< Pase de renderizado del fondo o cielo. */
	Transparent,  /**< Pase de renderizado para la geometría con transparencia. */
	Editor        /**< Pase de renderizado exclusivo para herramientas del editor (gizmos, grids). */
};

/**
 * @enum LightType
 * @brief Define los distintos tipos de fuentes de luz soportados.
 */
enum class
LightType {
	Directional = 0, /**< Luz direccional infinita (ej. el Sol). */
	Point,           /**< Luz omnidireccional que decae con la distancia (ej. una bombilla). */
	Spot             /**< Luz en forma de cono (ej. una linterna). */
};

/**
 * @struct LightData
 * @brief Contiene toda la información necesaria para procesar la iluminación de una fuente de luz.
 */
struct
LightData {
	LightType type = LightType::Directional;            /**< El tipo de la fuente de luz. */
	EU::Vector3 color = EU::Vector3(1.0f, 1.0f, 1.0f);  /**< El color emitido por la luz. */
	float intensity = 1.0f;                             /**< La fuerza o multiplicador de la iluminación. */

	EU::Vector3 direction = EU::Vector3(0.0f, -1.0f, 0.0f); /**< Vector de dirección (para Directional y Spot). */
	float range = 0.0f;                                 /**< Radio máximo de alcance (para Point y Spot). */

	EU::Vector3 position = EU::Vector3(0.0f, 0.0f, 0.0f);   /**< Coordenada de origen (para Point y Spot). */
	float spotAngle = 0.0f;                             /**< Ángulo de apertura del cono en radianes (para Spot). */
};

/**
 * @struct MaterialParams
 * @brief Parámetros físicos básicos del material que se envían a la GPU para el shading PBR.
 */
struct
MaterialParams {
	XMFLOAT4 baseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); /**< Color base o albedo en formato RGBA. */
	float metallic = 1.0f;                                 /**< Nivel de metalicidad (0.0 a 1.0). */
	float roughness = 1.0f;                                /**< Nivel de rugosidad de la superficie (0.0 a 1.0). */
	float ao = 1.0f;                                       /**< Factor de oclusión ambiental base. */
	float normalScale = 1.0f;                              /**< Multiplicador de intensidad para el mapa de normales. */
	float emissiveStrength = 1.0f;                         /**< Multiplicador de intensidad del color emisivo. */
	float alphaCutoff = 0.5f;                              /**< Umbral de descarte para materiales en dominio Masked. */
};

/**
 * @struct CBPerFrame
 * @brief Constant Buffer con los datos globales de la escena que se actualizan una vez por frame.
 */
struct
CBPerFrame {
	XMFLOAT4X4 View{};                                      /**< Matriz de vista de la cámara activa. */
	XMFLOAT4X4 Projection{};                                /**< Matriz de proyección de la cámara activa. */
	EU::Vector3 CameraPos{};                                /**< Posición de la cámara en coordenadas de mundo. */
	float pad0 = 0.0f;                                      /**< Relleno para alineación de memoria a 16 bytes. */
	EU::Vector3 LightDir = EU::Vector3(0.0f, -1.0f, 0.0f);  /**< Dirección de la luz direccional principal. */
	float pad1 = 0.0f;                                      /**< Relleno para alineación de memoria a 16 bytes. */
	EU::Vector3 LightColor = EU::Vector3(1.0f, 1.0f, 1.0f); /**< Color e intensidad combinados de la luz principal. */
	float pad2 = 0.0f;                                      /**< Relleno para alineación de memoria a 16 bytes. */
};

/**
 * @struct CBPerObject
 * @brief Constant Buffer con los datos específicos de cada objeto a dibujar.
 */
struct
CBPerObject {
	XMFLOAT4X4 World{}; /**< Matriz de transformación (World) del modelo. */
};

/**
 * @struct CBPerMaterial
 * @brief Constant Buffer que almacena los parámetros PBR de un material específico.
 */
struct
CBPerMaterial {
	XMFLOAT4 BaseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); /**< Color base del material. */
	float Metallic = 1.0f;                                 /**< Valor de metalicidad. */
	float Roughness = 1.0f;                                /**< Valor de rugosidad. */
	float AO = 1.0f;                                       /**< Multiplicador de Oclusión Ambiental. */
	float NormalScale = 1.0f;                              /**< Escala de las normales. */
	float EmissiveStrength = 1.0f;                         /**< Multiplicador emisivo. */
	float AlphaCutoff = 0.0f;                              /**< Valor límite de recorte alfa. */
	float pad0 = 0.0f;                                     /**< Alineación de memoria. */
	float pad1 = 0.0f;                                     /**< Alineación de memoria. */
	float pad2 = 0.0f;                                     /**< Alineación de memoria. */
	float pad3 = 0.0f;                                     /**< Alineación de memoria. */
	float pad4 = 0.0f;                                     /**< Alineación de memoria. */
	float pad5 = 0.0f;                                     /**< Alineación de memoria. */
};

/**
 * @struct RenderObject
 * @brief Estructura que encapsula todos los componentes necesarios para dibujar un objeto en la escena.
 */
struct
RenderObject {
	Mesh* mesh = nullptr;                                    /**< Puntero a la malla geométrica del objeto. */
	MaterialInstance* materialInstance = nullptr;            /**< Instancia de material principal (para mallas de 1 submesh). */
	std::vector<MaterialInstance*> materialInstances;        /**< Vector de materiales para mallas con múltiples submeshes. */
	XMMATRIX world = XMMatrixIdentity();                     /**< Matriz de transformación global precalculada. */
	bool castShadow = true;                                  /**< Indica si el objeto proyectará sombras. */
	bool transparent = false;                                /**< Bandera para enviar este objeto al pase de transparencias. */
	float distanceToCamera = 0.0f;                           /**< Distancia al a cámara (usado para ordenar transparentes back-to-front). */
};