/**
 * @file Transform.h
 * @brief Declara el componente de transformaciÃ³n TRS del sistema ECS.
 * @ingroup ecs
 *
 * Transform almacena posiciÃ³n, rotaciÃ³n y escala de una entidad y construye
 * la matriz de transformaciÃ³n local/mundo. Es consumido por el renderer para
 * la matriz World y por ImGuizmo para la manipulaciÃ³n visual de actores.
 */
#pragma once

#include "Prerequisites.h"
#include "EngineUtilities/Vectors/Vector3.h"
#include "Component.h"

/**
 * @class Transform
 * @brief Componente que almacena la transformaciÃ³n espacial de una entidad.
 *
 * Gestiona posiciÃ³n, rotaciÃ³n y escala, ademÃ¡s de su matriz de transformaciÃ³n.
 * Puede ser controlado externamente por herramientas como ImGuizmo o
 * reconstruido manualmente desde sus vectores.
 */
class Transform : public Component {
public:
  /**
   * @brief Constructor por defecto.
   *
   * Inicializa vectores y asigna el tipo de componente TRANSFORM.
   */
  Transform()
      : position()
      , rotation()
      , scale()
      , matrix()
      , worldMatrix()
      , Component(ComponentType::TRANSFORM) {
  }

  /**
   * @brief Inicializa valores por defecto del Transform.
   *
   * Establece la escala en uno y la matriz como identidad.
   */
  void
  init() {
    scale.one();
    matrix = XMMatrixIdentity();
    worldMatrix = XMMatrixIdentity();
  }

  /**
   * @brief ActualizaciÃ³n por frame.
   *
   * Actualmente vacÃ­o intencionalmente para evitar conflictos
   * con herramientas externas como ImGuizmo que controlan la matriz.
   *
   * @param deltaTime Tiempo transcurrido desde el Ãºltimo frame.
   */
  void
  update(float deltaTime) override {
    // Aplicar escala
    XMMATRIX scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);

    // Aplicar rotacion
    XMMATRIX rotationMatrix =
        XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);

    // Aplicar traslacion
    XMMATRIX translationMatrix = XMMatrixTranslation(position.x, position.y, position.z);

    // Componer la matriz final en el orden: scale -> rotation -> translation
    matrix = scaleMatrix * rotationMatrix * translationMatrix;
    worldMatrix = matrix;
  }

  /**
   * @brief Render del componente.
   *
   * No realiza ninguna operaciÃ³n visual directamente.
   *
   * @param deviceContext Contexto de dispositivo DirectX.
   */
  void
  render(DeviceContext &deviceContext) override {
  }

  /**
   * @brief Libera recursos asociados al Transform.
   */
  void
  destroy() {
  }

  /**
   * @brief Obtiene la posiciÃ³n actual.
   *
   * @return Referencia constante a la posiciÃ³n.
   */
  const EU::Vector3 &
  getPosition() const {
    return position;
  }

  /**
   * @brief Establece la posiciÃ³n.
   *
   * @param newPos Nueva posiciÃ³n en espacio mundo.
   */
  void
  setPosition(const EU::Vector3 &newPos) {
    position = newPos;
  } //<---------------

  /**
   * @brief Obtiene la rotaciÃ³n actual.
   *
   * @return Referencia constante a la rotaciÃ³n (en grados).
   */
  const EU::Vector3 &
  getRotation() const {
    return rotation;
  }

  /**
   * @brief Establece la rotaciÃ³n.
   *
   * @param newRot Nueva rotaciÃ³n en grados (Pitch, Yaw, Roll).
   */
  void
  setRotation(const EU::Vector3 &newRot) {
    rotation = newRot;
  } //<---------------

  /**
   * @brief Obtiene la escala actual.
   *
   * @return Referencia constante a la escala.
   */
  const EU::Vector3 &
  getScale() const {
    return scale;
  }

  /**
   * @brief Establece la escala.
   *
   * @param newScale Nueva escala por eje.
   */
  void
  setScale(const EU::Vector3 &newScale) {
    scale = newScale;
  } //<---------------

  /**
   * @brief Establece posiciÃ³n, rotaciÃ³n y escala simultÃ¡neamente.
   *
   * @param newPos Nueva posiciÃ³n.
   * @param newRot Nueva rotaciÃ³n.
   * @param newSca Nueva escala.
   */
  void
  setTransform(const EU::Vector3 &newPos,
               const EU::Vector3 &newRot,
               const EU::Vector3 &newSca) {
    position = newPos;
    rotation = newRot;
    scale = newSca;
  }

  // MÃ©todo para trasladar la posiciÃ³n del objeto
  // @param translation: Vector que representa la cantidad de traslado en cada eje
  /** @brief Declara o ejecuta translate. */
  void translate(const EU::Vector3 &translation);

  /**
   * @brief Reconstruye la matriz de transformaciÃ³n local a partir de los vectores de
   * posiciÃ³n, rotaciÃ³n y escala.
   *
   * Este mÃ©todo compone la matriz en el orden: escala -> rotaciÃ³n -> traslaciÃ³n.
   * Es Ãºtil cuando se modifican los vectores manualmente y se requiere actualizar la
   * matriz.
   */
  void
  rebuildMatrixFromVectors() {
    // Aplicar escala
    XMMATRIX scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);

    // Aplicar rotaciÃ³n (en radianes)
    XMMATRIX rotationMatrix =
        XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);

    // Aplicar traslaciÃ³n
    XMMATRIX translationMatrix = XMMatrixTranslation(position.x, position.y, position.z);

    // Componer la matriz final en el orden: scale -> rotation -> translation
    matrix = scaleMatrix * rotationMatrix * translationMatrix;
    worldMatrix = matrix;
  }

private:
  EU::Vector3 position; // PosiciÃ³n del objeto
  EU::Vector3 rotation; // RotaciÃ³n del objeto
  EU::Vector3 scale;    // Escala del objeto

public:
  XMMATRIX matrix;      // Matriz de transformaciÃ³n local
  XMMATRIX worldMatrix; // Matriz de transformaciÃ³n world
};
