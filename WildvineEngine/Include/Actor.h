#pragma once
#include "Prerequisites.h"
#include "Entity.h"
#include "Buffer.h"
#include "Texture.h"
//#include "Transform.h"
#include "SamplerState.h"

#include "ShaderProgram.h"

//DepthStencilState.h

class Device;
class MeshComponent;


class
Actor : public Actor {
public:
    Actor() = default;

  Actor(Device& device);

  virtual 
  ~Actor() = default;

  void
  init() override {}

  void
  udpdate(float deltaTime, DeviceContext& deviceContext) override;







};