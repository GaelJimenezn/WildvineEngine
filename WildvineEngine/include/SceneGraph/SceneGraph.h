#pragma once
#include "Prerequisites.h"

class Entity;
class DeviceContext;
class Camera;
class RenderScene;

/**
 * @class SceneGraph
 * @brief Maintains entity registration, hierarchy relationships, and render-scene gathering.
 *
 * The scene graph stores non-owning entity pointers, validates parent/child attachment rules,
 * updates hierarchical world transforms, and converts active entities into a @c RenderScene
 * consumed by the renderer.
 */
class
	SceneGraph {
public:
	/** @brief Creates an empty scene graph. */
	SceneGraph() = default;
	/** @brief Does not own registered entities; call destroy() to clear graph state. */
	~SceneGraph() = default;

	/** @brief Initializes internal scene-graph state before entity registration. */
	void
		init();

	/** @brief Registers an entity pointer in the graph without taking ownership. */
	void
		addEntity(Entity* e);  // registra en el grafo

	/** @brief Removes an entity pointer and any graph relationship that references it. */
	void
		removeEntity(Entity* e);

	/** @brief Returns true when @p possibleAncestor is above @p node in the hierarchy. */
	bool
		isAncestor(Entity* possibleAncestor, Entity* node) const;

	/** @brief Attaches @p child under @p parent when both are registered and no cycle is created. */
	bool
		attach(Entity* child, Entity* parent);

	/** @brief Detaches @p child from its current parent and promotes it to root level. */
	bool
		detach(Entity* child);

	/** @brief Updates registered entities and propagates hierarchy transforms. */
	void
		update(float deltaTime, DeviceContext& deviceContext);

	/** @brief Invokes direct entity rendering for graph-managed entities. */
	void
		render(DeviceContext& deviceContext);

	/** @brief Builds render queues and light data for the current camera. */
	void
		gatherRenderScene(RenderScene& outScene, const Camera& camera);

	/** @brief Clears graph registration and relationship data without deleting entities. */
	void
		destroy();
private:
	/** @brief Recursively combines local transforms with @p parentWorld for a hierarchy branch. */
	void
		updateWorldRecursive(Entity* node, const XMMATRIX& parentWorld);

	/** @brief Returns true when an entity has no parent relationship in the graph. */
	bool
		isRoot(Entity* e) const;

	/** @brief Returns true when @p e is present in @c m_entities. */
	bool
		isRegistered(Entity* e) const;

private:
	//std::vector<EU::TSharedPointer<Entity>> m_entities;
public:
	/** @brief Non-owning entity registry used by hierarchy and render-scene traversal. */
	std::vector<Entity*> m_entities;
};
