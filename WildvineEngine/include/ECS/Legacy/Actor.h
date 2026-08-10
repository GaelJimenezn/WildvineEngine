/**
 * @file Actor.h
 * @brief Declara la clase Actor â€” alias de la capa de compatibilidad con el ECS legado.
 * @ingroup core
 *
 * Este archivo actÃºa como header de transiciÃ³n: la clase Actor derivaba del
 * sistema ECS legado antes de migrarse a ECS/Actor.h. Se mantiene para no romper
 * cÃ³digo existente que incluya directamente "Actor.h".
 *
 * @note Para cÃ³digo nuevo, usar ECS/Actor.h directamente.
 */
#pragma once
#include "Prerequisites.h"
#include "Entity.h"
#include "Buffer.h"
#include "Texture.h"
// #include "Transform.h"
#include "SamplerState.h"

#include "ShaderProgram.h"

// DepthStencilState.h

/** @brief Declara class Device. */
class Device;
/** @brief Declara class MeshComponent. */
class MeshComponent;

/**
 * @class Actor
 * @brief Entidad de escena con componentes adjuntos (versiÃ³n legado).
 *
 * @deprecated Usar ECS::Actor (Include/ECS/Actor.h) para cÃ³digo nuevo.
 *
 * Extiende Entity aÃ±adiendo soporte para inicializaciÃ³n con un Device
 * de DirectX y lÃ³gica de actualizaciÃ³n por frame vÃ­a DeviceContext.
 */
class Actor : public Actor {
public:
  /** @brief Constructor por defecto. */
  Actor() = default;

  /**
   * @brief Constructor con dispositivo D3D11.
   * @param device Referencia al dispositivo para crear recursos GPU.
   */
  Actor(Device &device);

  /** @brief Destructor virtual por defecto. */
  virtual ~Actor() = default;

  /**
   * @brief InicializaciÃ³n del actor (sin implementaciÃ³n en la clase base).
   */
  void
  init() override {
  }

  /**
   * @brief Actualiza el actor cada frame.
   * @param deltaTime     Tiempo transcurrido desde el Ãºltimo frame, en segundos.
   * @param deviceContext Contexto D3D11 para operaciones GPU.
   */
  void udpdate(float deltaTime, DeviceContext &deviceContext) override;
};
