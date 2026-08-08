/**
 * @file Actor.h
 * @brief Declara la clase Actor — alias de la capa de compatibilidad con el ECS legado.
 * @ingroup core
 *
 * Este archivo actúa como header de transición: la clase Actor derivaba del
 * sistema ECS legado antes de migrarse a ECS/Actor.h. Se mantiene para no romper
 * código existente que incluya directamente "Actor.h".
 *
 * @note Para código nuevo, usar ECS/Actor.h directamente.
 */
#pragma once
#include "Prerequisites.h"
#include "Entity.h"
#include "Buffer.h"
#include "Texture.h"
//#include "Transform.h"
#include "SamplerState.h"

#include "ShaderProgram.h"

//DepthStencilState.h

/** @brief Declara class Device. */
class
Device;
/** @brief Declara class MeshComponent. */
class
MeshComponent;

/**
 * @class Actor
 * @brief Entidad de escena con componentes adjuntos (versión legado).
 *
 * @deprecated Usar ECS::Actor (Include/ECS/Actor.h) para código nuevo.
 *
 * Extiende Entity añadiendo soporte para inicialización con un Device
 * de DirectX y lógica de actualización por frame vía DeviceContext.
 */
class
Actor : public Actor {
public:
  /** @brief Constructor por defecto. */
  Actor() = default;

  /**
   * @brief Constructor con dispositivo D3D11.
   * @param device Referencia al dispositivo para crear recursos GPU.
   */
  Actor(Device& device);

  /** @brief Destructor virtual por defecto. */
  virtual 
  ~Actor() = default;

  /**
   * @brief Inicialización del actor (sin implementación en la clase base).
   */
  void
  init() override {}

  /**
   * @brief Actualiza el actor cada frame.
   * @param deltaTime     Tiempo transcurrido desde el último frame, en segundos.
   * @param deviceContext Contexto D3D11 para operaciones GPU.
   */
  void
  udpdate(float deltaTime, DeviceContext& deviceContext) override;
};
