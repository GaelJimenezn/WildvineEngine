#pragma once
#include "Prerequisites.h"

class Mesh;
class MaterialInstance;

/**
 * @enum MaterialDomain
 * @brief Define el comportamiento de renderizado de un material.
 */
enum class
MaterialDomain {
  /**
   * @brief Material completamente opaco.
   */
  Opaque = 0,

  /**
   * @brief Material con descarte de píxeles mediante alpha cutoff.
   */
  Masked,

  /**
   * @brief Material con transparencia.
   */
  Transparent
};

/**
 * @enum BlendMode
 * @brief Modos de mezcla utilizados durante el renderizado.
 */
enum class
BlendMode {

  /**
   * @brief Sin mezcla. El píxel sobrescribe el contenido existente.
   */
  Opaque = 0,

  /**
   * @brief Mezcla alfa tradicional.
   */
  Alpha,

  /**
   * @brief Mezcla aditiva utilizada en efectos de energía, fuego o partículas.
   */
  Additive,

  /**
   * @brief Mezcla con alpha premultiplicado.
   */
  PremultipliedAlpha
};

/**
 * @enum RenderPassType
 * @brief Tipos de pases de renderizado soportados por el pipeline.
 */
enum class
RenderPassType {

  /**
   * @brief Pase de generación de sombras.
   */
  Shadow = 0,

  /**
   * @brief Pase principal para geometría opaca.
   */
  Opaque,

  /**
   * @brief Pase de renderizado del skybox.
   */
  Skybox,

  /**
   * @brief Pase para geometría transparente.
   */
  Transparent,

  /**
   * @brief Pase utilizado por herramientas del editor.
   */
  Editor
};

/**
 * @enum LightType
 * @brief Tipos de luces soportadas por el motor.
 */
enum class
LightType {

  /**
   * @brief Luz direccional infinita.
   */
  Directional = 0,

  /**
   * @brief Luz puntual con posición y rango.
   */
  Point,

  /**
   * @brief Luz tipo spotlight con cono de influencia.
   */
  Spot
};

/**
 * @brief Número máximo de luces soportadas por escena.
 */
constexpr int kMaxSceneLights = 8;

/**
 * @struct LightData
 * @brief Describe una fuente de luz utilizada por el sistema de renderizado.
 */
struct
LightData {

  /**
   * @brief Tipo de luz.
   */
  LightType type = LightType::Directional;

  /**
   * @brief Color de la luz.
   */
  EU::Vector3 color = EU::Vector3(1.0f, 1.0f, 1.0f);

  /**
   * @brief Intensidad de emisión.
   */
  float intensity = 1.0f;

  /**
   * @brief Dirección utilizada por luces direccionales y spot.
   */
  EU::Vector3 direction = EU::Vector3(0.0f, -1.0f, 0.0f);

  /**
   * @brief Distancia máxima de influencia.
   */
  float range = 0.0f;

  /**
   * @brief Posición de la luz.
   */
  EU::Vector3 position = EU::Vector3(0.0f, 0.0f, 0.0f);

  /**
   * @brief Ángulo del cono para luces Spot.
   */
  float spotAngle = 0.0f;
};

/**
 * @struct MaterialParams
 * @brief Parámetros físicos utilizados por materiales PBR.
 */
struct
MaterialParams {

  /**
   * @brief Color base del material.
   */
  XMFLOAT4 baseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

  /**
   * @brief Nivel de metalicidad.
   */
  float metallic = 1.0f;

  /**
   * @brief Rugosidad superficial.
   */
  float roughness = 1.0f;

  /**
   * @brief Intensidad de Ambient Occlusion.
   */
  float ao = 1.0f;

  /**
   * @brief Escala aplicada al Normal Map.
   */
  float normalScale = 1.0f;

  /**
   * @brief Intensidad de emisión.
   */
  float emissiveStrength = 1.0f;

  /**
   * @brief Umbral utilizado por materiales Masked.
   */
  float alphaCutoff = 0.5f;
};

/**
 * @struct CBPerFrame
 * @brief Constant Buffer actualizado una vez por frame.
 *
 * Contiene información global utilizada por shaders de vértices
 * y píxeles durante el renderizado.
 */
struct
CBPerFrame {

  /**
   * @brief Matriz de vista de la cámara.
   */
  XMFLOAT4X4 View{};

  /**
   * @brief Matriz de proyección.
   */
  XMFLOAT4X4 Projection{};

  /**
   * @brief Matriz Light View Projection utilizada para sombras.
   */
  XMFLOAT4X4 LightViewProjection{};

  /**
   * @brief Posición de la cámara en espacio mundial.
   */
  EU::Vector3 CameraPos{};

  float pad0 = 0.0f;

  /**
   * @brief Dirección de la luz principal.
   */
  EU::Vector3 LightDir = EU::Vector3(0.0f, -1.0f, 0.0f);

  float pad1 = 0.0f;

  /**
   * @brief Color de la luz principal.
   */
  EU::Vector3 LightColor = EU::Vector3(1.0f, 1.0f, 1.0f);

  /**
   * @brief Alcance de la luz principal.
   */
  float LightRange = 10.0f;

  /**
   * @brief Posición de la luz principal.
   */
  EU::Vector3 LightPosition = EU::Vector3(0.0f, 3.0f, 0.0f);

  /**
   * @brief Tipo de la luz principal.
   */
  int LightType = 0;

  /**
   * @brief Posiciones y rangos de las luces de escena.
   */
  XMFLOAT4 LightPositionsRanges[kMaxSceneLights]{};

  /**
   * @brief Colores y tipos de las luces de escena.
   */
  XMFLOAT4 LightColorsTypes[kMaxSceneLights]{};

  /**
   * @brief Direcciones e intensidades de las luces de escena.
   */
  XMFLOAT4 LightDirectionsIntensities[kMaxSceneLights]{};

  /**
   * @brief Cantidad de luces activas.
   */
  int LightCount = 0;

  XMFLOAT3 pad2 = XMFLOAT3(0.0f, 0.0f, 0.0f);
};

/**
 * @struct CBPerObject
 * @brief Constant Buffer actualizado por objeto renderizado.
 */
struct
CBPerObject {

  /**
   * @brief Transformación World del objeto.
   */
  XMFLOAT4X4 World{};
};

/**
 * @struct CBPerMaterial
 * @brief Constant Buffer con parámetros de material.
 */
struct
CBPerMaterial {

  XMFLOAT4 BaseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

  float Metallic = 1.0f;
  float Roughness = 1.0f;
  float AO = 1.0f;
  float NormalScale = 1.0f;
  float EmissiveStrength = 1.0f;
  float AlphaCutoff = 0.0f;

  /**
   * @brief Variables de padding para alineación de memoria GPU.
   */
  float pad0 = 0.0f;
  float pad1 = 0.0f;
  float pad2 = 0.0f;
  float pad3 = 0.0f;
  float pad4 = 0.0f;
  float pad5 = 0.0f;
};

/**
 * @struct RenderObject
 * @brief Representa un objeto preparado para ser renderizado.
 *
 * Contiene referencias a la geometría, materiales y datos
 * auxiliares utilizados por el pipeline de renderizado.
 */
struct
RenderObject {

  /**
   * @brief Malla asociada al objeto.
   */
  Mesh* mesh = nullptr;

  /**
   * @brief Material principal.
   */
  MaterialInstance* materialInstance = nullptr;

  /**
   * @brief Lista de materiales utilizados por la malla.
   */
  std::vector<MaterialInstance*> materialInstances;

  /**
   * @brief Transformación mundial del objeto.
   */
  XMMATRIX world = XMMatrixIdentity();

  /**
   * @brief Indica si el objeto proyecta sombras.
   */
  bool castShadow = true;

  /**
   * @brief Indica si debe renderizarse en el pase transparente.
   */
  bool transparent = false;

  /**
   * @brief Distancia desde la cámara.
   *
   * Utilizada principalmente para ordenamiento de transparencias.
   */
  float distanceToCamera = 0.0f;
};