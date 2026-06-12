/**
 * @file ECS/Transform.h
 * @brief Declares the ECS transform component using Unreal-style world units, degree
 * rotations, and unit scale.
 * @ingroup ecs
 */
#pragma once
#include "Prerequisites.h"
#include "EngineUtilities/Vectors/Vector3.h"
#include "Component.h"

/**
 * @class Transform
 * @brief ECS transform storing world-space position, degree rotation, unit scale, and
 * cached matrices.
 *
 * Position uses engine world units, rotation is stored in degrees for editor/Unreal-style
 * workflows,
 * and scale is multiplicative with (1, 1, 1) as the neutral value. The Direct3D matrix is
 * rebuilt
 * during update() by converting degrees to radians immediately before calling
 * DirectXMath.
 */
class
	Transform : public Component {
public:
    /**
     * @brief Creates a transform with zero position/rotation and default-initialized
     * scale.
     */
    Transform() : position(),
        rotation(),
        scale(),
        matrix(),
        worldMatrix(),
        Component(ComponentType::TRANSFORM) {
    }

    /** @brief Resets scale to one and initializes local/world matrices to identity. */
    void
    	init() {
        scale.one();
        matrix = XMMatrixIdentity();
        worldMatrix = XMMatrixIdentity();
    }

    /**
     * @brief Rebuilds local/world matrices from scale, degree rotation, and position.
     * @param deltaTime Frame delta in seconds; currently unused by Transform itself.
     */
    void
    	update(float deltaTime) override {
        // Aplicar escala
        XMMATRIX scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);
        // Rotacion almacenada en grados, como en Unreal. DirectX espera radianes.
        XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(
            XMConvertToRadians(rotation.x),
            XMConvertToRadians(rotation.y),
            XMConvertToRadians(rotation.z));
        // Aplicar traslacion
        XMMATRIX translationMatrix =
        		XMMatrixTranslation(position.x, position.y, position.z);

        // Componer la matriz final en el orden: scale -> rotation -> translation
        matrix = scaleMatrix * rotationMatrix * translationMatrix;
        worldMatrix = matrix;
    }

    /** @brief Transform has no render work; present to satisfy Component. */
    void
    	render(DeviceContext& deviceContext) override {}

    /** @brief Transform owns no external resources. */
    void
    	destroy() {}

    /** @brief Returns the world-space position in engine units. */
    const EU::Vector3&
    	getPosition() const { return position; }

    /** @brief Sets the world-space position in engine units. */
    void
    	setPosition(const EU::Vector3& newPos) { position = newPos; }

    /** @brief Returns Euler rotation in degrees. */
    const EU::Vector3&
    	getRotation() const { return rotation; }

    /** @brief Sets Euler rotation in degrees. */
    void
    	setRotation(const EU::Vector3& newRot) { rotation = newRot; }

    /** @brief Returns multiplicative local scale; (1,1,1) is neutral. */
    const EU::Vector3&
    	getScale() const { return scale; }

    /** @brief Sets multiplicative local scale. */
    void
    	setScale(const EU::Vector3& newScale) { scale = newScale; }

    /** @brief Sets position, degree rotation, and scale in one operation. */
    void
    	setTransform(const EU::Vector3& newPos,
            const EU::Vector3& newRot,
            const EU::Vector3& newSca) {
        position = newPos;
        rotation = newRot;
        scale = newSca;
    }

    /** @brief Adds a world-space translation delta to the current position. */
    void
    	translate(const EU::Vector3& translation);

private:
    EU::Vector3 position;  ///< World-space position in engine units.
    EU::Vector3 rotation;  ///< Euler rotation in degrees.
    EU::Vector3 scale;     ///< Multiplicative local scale.

public:
    XMMATRIX matrix;      ///< Cached local transform matrix.
    XMMATRIX worldMatrix; ///< Cached world transform matrix.
};
