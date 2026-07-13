/**
 * @file Transform.h
 * @brief Declara el componente de transformación TRS del sistema ECS.
 * @ingroup ecs
 *
 * Transform almacena posición, rotación y escala de una entidad y construye
 * la matriz de transformación local/mundo. Es consumido por el renderer para
 * la matriz World y por ImGuizmo para la manipulación visual de actores.
 */
#pragma once

#include "Prerequisites.h"
#include "EngineUtilities/Vectors/Vector3.h"
#include "Component.h"

/**
 * @class Transform
 * @brief Componente que almacena la transformación espacial de una entidad.
 *
 * Gestiona posición, rotación y escala, además de su matriz de transformación.
 * Puede ser controlado externamente por herramientas como ImGuizmo o
 * reconstruido manualmente desde sus vectores.
 */
class
Transform : public Component {
public:

  /**
   * @brief Constructor por defecto.
   *
   * Inicializa vectores y asigna el tipo de componente TRANSFORM.
   */
  Transform() : position(),
                rotation(),
                scale(), 
                matrix(), 
                worldMatrix(),                                                       //<----------------
                Component(ComponentType::TRANSFORM) {}

  /**
   * @brief Inicializa valores por defecto del Transform.
   *
   * Establece la escala en uno y la matriz como identidad.
   */
  void
  init() {
    scale.one();
    matrix = XMMatrixIdentity();
    worldMatrix = XMMatrixIdentity();                                                //<----------------
  }

  /**
   * @brief Actualización por frame.
   *
   * Actualmente vacío intencionalmente para evitar conflictos
   * con herramientas externas como ImGuizmo que controlan la matriz.
   *
   * @param deltaTime Tiempo transcurrido desde el último frame.
   */
  void
  update(float deltaTime) override {                                                  //<----------------
    // Aplicar escala
    XMMATRIX scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);

    // Aplicar rotacion
    XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);

    // Aplicar traslacion
    XMMATRIX translationMatrix = XMMatrixTranslation(position.x, position.y, position.z);

    // Componer la matriz final en el orden: scale -> rotation -> translation
    matrix = scaleMatrix * rotationMatrix * translationMatrix;
    worldMatrix = matrix;
  }

  /**
   * @brief Render del componente.
   *
   * No realiza ninguna operación visual directamente.
   *
   * @param deviceContext Contexto de dispositivo DirectX.
   */
  void
  render(DeviceContext& deviceContext) override {}

  /**
   * @brief Libera recursos asociados al Transform.
   */
  void
  destroy() {}

  /**
   * @brief Obtiene la posición actual.
   *
   * @return Referencia constante a la posición.
   */
  const EU::Vector3& 
  getPosition() const { return position; }

  /**
   * @brief Establece la posición.
   *
   * @param newPos Nueva posición en espacio mundo.
   */
  void
  setPosition(const EU::Vector3& newPos) { position = newPos; }         //<---------------


  /**
   * @brief Obtiene la rotación actual.
   *
   * @return Referencia constante a la rotación (en grados).
   */
  const EU::Vector3& 
  getRotation() const { return rotation; }

  /**
   * @brief Establece la rotación.
   *
   * @param newRot Nueva rotación en grados (Pitch, Yaw, Roll).
   */
  void
  setRotation(const EU::Vector3& newRot) { rotation = newRot; }        //<---------------

  /**
   * @brief Obtiene la escala actual.
   *
   * @return Referencia constante a la escala.
   */
  const EU::Vector3& 
  getScale() const { return scale; }

  /**
   * @brief Establece la escala.
   *
   * @param newScale Nueva escala por eje.
   */
  void
  setScale(const EU::Vector3& newScale) { scale = newScale; }        //<---------------

  /**
   * @brief Establece posición, rotación y escala simultáneamente.
   *
   * @param newPos Nueva posición.
   * @param newRot Nueva rotación.
   * @param newSca Nueva escala.
   */
  void
  setTransform(const EU::Vector3& newPos, 
                const EU::Vector3& newRot, 
                const EU::Vector3& newSca) {
    position = newPos;
    rotation = newRot;
    scale = newSca;
  }

  // Método para trasladar la posición del objeto
  // @param translation: Vector que representa la cantidad de traslado en cada eje
  void
  translate(const EU::Vector3& translation);

  /**
 * @brief Reconstruye la matriz de transformación local a partir de los vectores de posición, rotación y escala.
 *
 * Este método compone la matriz en el orden: escala -> rotación -> traslación.
 * Es útil cuando se modifican los vectores manualmente y se requiere actualizar la matriz.
 */
  void rebuildMatrixFromVectors() {
    // Aplicar escala
    XMMATRIX scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);

    // Aplicar rotación (en radianes)
    XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);

    // Aplicar traslación
    XMMATRIX translationMatrix = XMMatrixTranslation(position.x, position.y, position.z);

    // Componer la matriz final en el orden: scale -> rotation -> translation
    matrix = scaleMatrix * rotationMatrix * translationMatrix;
    worldMatrix = matrix;
  }


private:
  EU::Vector3 position;  // Posición del objeto
  EU::Vector3 rotation;  // Rotación del objeto
  EU::Vector3 scale;     // Escala del objeto

public:
  XMMATRIX matrix;    // Matriz de transformación local
  XMMATRIX worldMatrix; // Matriz de transformación world
};
