#pragma once
#include "Prerequisites.h"

class Mesh;
class MaterialInstance;

enum class
MaterialDomain {
	Opaque = 0,
	Masked,
	Transparent
};

enum class
BlendMode {
	Opaque = 0,
	Alpha,
	Additive,
	PremultipliedAlpha
};

enum class
RenderPassType {
	Shadow = 0,
	Opaque,
	Skybox,
	Transparent,
	Editor
};

enum class
LightType {
	Directional = 0,
	Point,
	Spot
};

struct
LightData {
	LightType type = LightType::Directional;
	EU::Vector3 color = EU::Vector3(1.0f, 1.0f, 1.0f);
	float intensity = 1.0f;

	EU::Vector3 direction = EU::Vector3(0.0f, -1.0f, 0.0f);
	float range = 0.0f;

	EU::Vector3 position = EU::Vector3(0.0f, 0.0f, 0.0f);
	float spotAngle = 0.0f;
};

struct
MaterialParams {
	XMFLOAT4 baseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	float metallic = 1.0f;
	float roughness = 1.0f;
	float ao = 1.0f;
	float normalScale = 1.0f;
	float emissiveStrength = 1.0f;
	float alphaCutoff = 0.5f;
};

struct
CBPerFrame {
	XMFLOAT4X4 View{};
	XMFLOAT4X4 Projection{};
	EU::Vector3 CameraPos{};
	float pad0 = 0.0f;
	EU::Vector3 LightDir = EU::Vector3(0.0f, -1.0f, 0.0f);
	float pad1 = 0.0f;
	EU::Vector3 LightColor = EU::Vector3(1.0f, 1.0f, 1.0f);
	float pad2 = 0.0f;
};

struct
CBPerObject {
	XMFLOAT4X4 World{};
};

struct
CBPerMaterial {
	XMFLOAT4 BaseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	float Metallic = 1.0f;
	float Roughness = 1.0f;
	float AO = 1.0f;
	float NormalScale = 1.0f;
	float EmissiveStrength = 1.0f;
	float AlphaCutoff = 0.0f;
	float pad0 = 0.0f;
	float pad1 = 0.0f;
	float pad2 = 0.0f;
	float pad3 = 0.0f;
	float pad4 = 0.0f;
	float pad5 = 0.0f;
};

struct
RenderObject {
	Mesh* mesh = nullptr;
	MaterialInstance* materialInstance = nullptr;
	std::vector<MaterialInstance*> materialInstances;
	XMMATRIX world = XMMatrixIdentity();
	bool castShadow = true;
	bool transparent = false;
	float distanceToCamera = 0.0f;
};
