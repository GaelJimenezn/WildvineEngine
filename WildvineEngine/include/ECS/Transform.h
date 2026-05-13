#pragma once
#include "Prerequisites.h"
#include "EngineUtilities/Vectors/Vector3.h"
#include "Component.h"

class 
Transform : public Component {
public:
  // Constructor que inicializa posición, rotación y escala por defecto
  Transform() : position(), 
                rotation(), 
                scale(), 
                matrix(), 
                worldMatrix(),
                Component(ComponentType::TRANSFORM) {}

  // Métodos para inicialización, actualización, renderizado y destrucción
  // Inicializa el objeto Transform
  void 
  init() {
    scale.one();
    matrix = XMMatrixIdentity();
    worldMatrix = XMMatrixIdentity();
  }

  // Actualiza el estado del objeto Transform basado en el tiempo transcurrido
  // @param deltaTime: Tiempo transcurrido desde la última actualización
  void 
  update(float deltaTime) override {
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

  // Renderiza el objeto Transform
  // @param deviceContext: Contexto del dispositivo de renderizado
  void 
  render(DeviceContext& deviceContext) override {}

  // Destruye el objeto Transform y libera recursos
  void 
  destroy() {}

  // Métodos de acceso a los datos de posición
  // Retorna la posición actual
  const EU::Vector3&
  getPosition() const { return position; }

  // Establece una nueva posición
  void 
  setPosition(const EU::Vector3& newPos) { position = newPos; }

  // Métodos de acceso a los datos de rotación
  // Retorna la rotación actual
  const EU::Vector3&
  getRotation() const { return rotation; }

  // Establece una nueva rotación
  void 
  setRotation(const EU::Vector3& newRot) { rotation = newRot; }

  // Métodos de acceso a los datos de escala
  // Retorna la escala actual
  const EU::Vector3&
  getScale() const { return scale; }

  // Establece una nueva escala
  void 
  setScale(const EU::Vector3& newScale) { scale = newScale; }

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

private:
  EU::Vector3 position;  // Posición del objeto
  EU::Vector3 rotation;  // Rotación del objeto
  EU::Vector3 scale;     // Escala del objeto

public:
  XMMATRIX matrix;    // Matriz de transformación local
  XMMATRIX worldMatrix; // Matriz de transformación world
};
