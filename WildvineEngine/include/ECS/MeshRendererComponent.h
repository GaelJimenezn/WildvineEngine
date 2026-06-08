#pragma once
#include "Prerequisites.h"
#include "ECS/Component.h"

class Mesh;
class MaterialInstance;
class DeviceContext;

/**
 * @class MeshRendererComponent
 * @brief ECS component that links an entity with renderable mesh and material data.
 *
 * The component stores non-owning pointers to the mesh and material instances used by
 * the renderer when building a render scene. It also exposes visibility and shadow flags
 * so render passes can skip hidden objects or shadow casters without changing ownership.
 *
 * @note This class does not create or destroy GPU resources; referenced assets are owned elsewhere.
 */
class
MeshRendererComponent : public Component {
public:
	/** @brief Creates a mesh renderer component with @c ComponentType::MESH. */
	MeshRendererComponent()
		: Component(ComponentType::MESH) {}

	/** @brief Mesh renderer has no standalone initialization work. */
	void init() override {}
	/** @brief Mesh renderer has no per-frame simulation work by itself. */
	void update(float deltaTime) override {}
	/** @brief Rendering is performed by scene renderers after gathering this component. */
	void render(DeviceContext& deviceContext) override {}
	/** @brief Does not release referenced mesh or material instances. */
	void destroy() override {}

	/** @brief Assigns the mesh used by this renderer. */
	void setMesh(Mesh* mesh) { m_mesh = mesh; }
	/** @brief Returns the assigned mesh, or @c nullptr when none is bound. */
	Mesh* getMesh() const { return m_mesh; }

	/**
	 * @brief Sets a single material instance and makes it the first render material.
	 * @param materialInstance Non-owning material instance pointer; may be @c nullptr.
	 */
	void setMaterialInstance(MaterialInstance* materialInstance) {
		m_materialInstance = materialInstance;
		m_materialInstances.clear();
		if (materialInstance) {
			m_materialInstances.push_back(materialInstance);
		}
	}
	/** @brief Returns the primary material instance, or @c nullptr when none is set. */
	MaterialInstance* getMaterialInstance() const { return m_materialInstance; }

	/**
	 * @brief Replaces the full material list used by submeshes.
	 * @param materialInstances Non-owning material instance pointers ordered by material slot.
	 */
	void setMaterialInstances(const std::vector<MaterialInstance*>& materialInstances) {
		m_materialInstances = materialInstances;
		m_materialInstance = m_materialInstances.empty() ? nullptr : m_materialInstances.front();
	}

	/**
	 * @brief Appends a material instance to the material slot list.
	 * @param materialInstance Non-owning material instance pointer; ignored when @c nullptr.
	 */
	void addMaterialInstance(MaterialInstance* materialInstance) {
		if (!materialInstance) {
			return;
		}
		if (!m_materialInstance) {
			m_materialInstance = materialInstance;
		}
		m_materialInstances.push_back(materialInstance);
	}

	/** @brief Returns all material instances currently assigned to this renderer. */
	const std::vector<MaterialInstance*>& getMaterialInstances() const { return m_materialInstances; }

	/** @brief Returns whether this component should contribute visible render objects. */
	bool isVisible() const { return m_visible; }
	/** @brief Enables or disables contribution to visible render queues. */
	void setVisible(bool visible) { m_visible = visible; }

	/** @brief Returns whether this renderer should be considered by shadow passes. */
	bool canCastShadow() const { return m_castShadow; }
	/** @brief Enables or disables shadow casting for this renderer. */
	void setCastShadow(bool value) { m_castShadow = value; }

private:
	/** @brief Non-owning mesh consumed by render scene gathering. */
	Mesh* m_mesh = nullptr;
	/** @brief Primary non-owning material instance used as a fallback/default slot. */
	MaterialInstance* m_materialInstance = nullptr;
	/** @brief Non-owning material instances indexed by submesh material slot. */
	std::vector<MaterialInstance*> m_materialInstances;
	/** @brief Visibility flag used by render scene collection. */
	bool m_visible = true;
	/** @brief Shadow-casting flag used by shadow render passes. */
	bool m_castShadow = true;
};
