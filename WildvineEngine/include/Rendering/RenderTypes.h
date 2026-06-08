/**
 * @file RenderTypes.h
 * @brief Declara la API de RenderTypes dentro del subsistema Rendering.
 * @ingroup rendering
 */
#pragma once
#include "Prerequisites.h"

class Mesh;
class MaterialInstance;

/**
 * @enum MaterialDomain
 * @brief High-level render category that controls queue placement and pass behavior.
 */
enum class
	MaterialDomain {
	/** @brief Fully opaque material rendered in opaque passes. */
	Opaque = 0,
	/** @brief Alpha-tested material that discards pixels below a cutoff. */
	Masked,
	/** @brief Blended material rendered after opaque geometry. */
	Transparent
};

/**
 * @enum BlendMode
 * @brief Blend-state intent requested by a material.
 */
enum class
	BlendMode {
	/** @brief No blending; output replaces the render target. */
	Opaque = 0,
	/** @brief Standard alpha blending. */
	Alpha,
	/** @brief Additive blending for emissive/light-like effects. */
	Additive,
	/** @brief Premultiplied alpha blending. */
	PremultipliedAlpha
};

/**
 * @enum RenderPassType
 * @brief Renderer pass identifier used when binding pass-specific material state.
 */
enum class
	RenderPassType {
	/** @brief Shadow-map depth pass. */
	Shadow = 0,
	/** @brief Opaque color/depth pass. */
	Opaque,
	/** @brief Skybox/background pass. */
	Skybox,
	/** @brief Transparent blended pass. */
	Transparent,
	/** @brief Editor overlay or viewport-support pass. */
	Editor
};

/**
 * @enum LightType
 * @brief Supported light shapes encoded in @c LightData.
 */
enum class
	LightType {
	/** @brief Infinite directional light. */
	Directional = 0,
	/** @brief Point light with spherical attenuation. */
	Point,
	/** @brief Spot light with cone attenuation. */
	Spot
};

/** @brief Maximum number of lights expected by fixed-size shader constant layouts. */
constexpr int RMaxLights = 8;

/**
 * @struct LightData
 * @brief CPU-side light payload copied into render-scene and shader constants.
 */
struct
	LightData {
	/** @brief Light shape/type. */
	LightType type = LightType::Directional;
	/** @brief RGB light color in linear space. */
	EU::Vector3 color = EU::Vector3(1.0f, 1.0f, 1.0f);
	/** @brief Light intensity multiplier. */
	float intensity = 1.0f;

	/** @brief Direction for directional/spot lights. */
	EU::Vector3 direction = EU::Vector3(0.0f, -1.0f, 0.0f);
	/** @brief Attenuation range for local lights. */
	float range = 0.0f;

	/** @brief World-space light position for point/spot lights. */
	EU::Vector3 position = EU::Vector3(0.0f, 0.0f, 0.0f);
	/** @brief Spot cone angle in renderer-defined units. */
	float spotAngle = 0.0f;
};

/**
 * @struct MaterialParams
 * @brief Scalar/vector PBR parameters owned by a material instance.
 */
struct
	MaterialParams {
	/** @brief Base color and alpha factor. */
	XMFLOAT4 baseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	/** @brief Metallic factor. */
	float metallic = 1.0f;
	/** @brief Roughness factor. */
	float roughness = 1.0f;
	/** @brief Ambient-occlusion factor. */
	float ao = 1.0f;
	/** @brief Normal-map strength multiplier. */
	float normalScale = 1.0f;
	/** @brief Emissive contribution multiplier. */
	float emissiveStrength = 1.0f;
	/** @brief Alpha threshold used by masked materials. */
	float alphaCutoff = 0.5f;
};

/**
 * @struct CBPerFrame
 * @brief Constant-buffer payload shared by scene objects for a single frame.
 */
struct
	CBPerFrame {
	/** @brief View matrix. */
	XMFLOAT4X4 View{};
	/** @brief Projection matrix. */
	XMFLOAT4X4 Projection{};
	/** @brief Light view-projection matrix for shadow mapping. */
	XMFLOAT4X4 LightViewProjection{};
	/** @brief Camera world position. */
	EU::Vector3 CameraPos{};
	/** @brief Padding for 16-byte constant-buffer alignment. */
	float pad0 = 0.0f;
	/** @brief Primary light direction. */
	EU::Vector3 LightDir = EU::Vector3(0.0f, -1.0f, 0.0f);
	/** @brief Padding for 16-byte constant-buffer alignment. */
	float pad1 = 0.0f;
	/** @brief Primary light color. */
	EU::Vector3 LightColor = EU::Vector3(1.0f, 1.0f, 1.0f);
	/** @brief Primary light range. */
	float ligthRange = 10.0f;
	/** @brief Primary light world position. */
	EU::Vector3 LightPosition = EU::Vector3(0.0f, 5.0f, 0.0f);
	/** @brief Number of active lights encoded for the frame. */
	int LightCount = 0;
	/** @brief Padding for 16-byte constant-buffer alignment. */
	XMFLOAT3 pad2 = XMFLOAT3(0.0f, 0.0f, 0.0f);
};

/** @brief Per-object constant-buffer payload containing the world matrix. */
struct
	CBPerObject {
	/** @brief Object-to-world matrix. */
	XMFLOAT4X4 World{};
};

/** @brief Per-material constant-buffer payload copied from @c MaterialParams. */
struct
	CBPerMaterial {
	/** @brief Base color and alpha factor. */
	XMFLOAT4 BaseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	/** @brief Metallic factor. */
	float Metallic = 1.0f;
	/** @brief Roughness factor. */
	float Roughness = 1.0f;
	/** @brief Ambient-occlusion factor. */
	float AO = 1.0f;
	/** @brief Normal-map strength multiplier. */
	float NormalScale = 1.0f;
	/** @brief Emissive contribution multiplier. */
	float EmissiveStrength = 1.0f;
	/** @brief Alpha threshold for masked materials. */
	float AlphaCutoff = 0.0f;
	/** @brief Padding for shader constant alignment. */
	float pad0 = 0.0f;
	/** @brief Padding for shader constant alignment. */
	float pad1 = 0.0f;
	/** @brief Padding for shader constant alignment. */
	float pad2 = 0.0f;
	/** @brief Padding for shader constant alignment. */
	float pad3 = 0.0f;
	/** @brief Padding for shader constant alignment. */
	float pad4 = 0.0f;
	/** @brief Padding for shader constant alignment. */
	float pad5 = 0.0f;
};

/**
 * @struct RenderObject
 * @brief One renderable object entry produced by scene gathering.
 */
struct
	RenderObject {
	/** @brief Mesh to draw; non-owning. */
	Mesh* mesh = nullptr;
	/** @brief Primary material instance fallback; non-owning. */
	MaterialInstance* materialInstance = nullptr;
	/** @brief Material instances indexed by submesh material slot; non-owning. */
	std::vector<MaterialInstance*> materialInstances;
	/** @brief Object-to-world matrix. */
	XMMATRIX world = XMMatrixIdentity();
	/** @brief Whether this object should render into shadow maps. */
	bool castShadow = true;
	/** @brief Whether this object belongs to transparent queues. */
	bool transparent = false;
	/** @brief Camera distance used for transparent sorting. */
	float distanceToCamera = 0.0f;
};
