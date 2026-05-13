#pragma once
#include "Prerequisites.h"

class Entity;
class DeviceContext;
class Camera;
class RenderScene;

class 
SceneGraph {
public:
	SceneGraph()  = default;
	~SceneGraph() = default;

	void 
	init();

	void 
	addEntity(Entity* e);  // registra en el grafo

	void 
	removeEntity(Entity* e);

	bool 
	isAncestor(Entity* possibleAncestor, Entity* node) const;

	bool
	attach(Entity* child, Entity* parent);

	bool
	detach(Entity* child);

	void 
	update(float deltaTime, DeviceContext& deviceContext);
	
	void 
	render(DeviceContext& deviceContext);

	void
	gatherRenderScene(RenderScene& outScene, const Camera& camera);

	void
	destroy();
private:
	void 
	updateWorldRecursive(Entity* node, const XMMATRIX& parentWorld);

	bool 
	isRoot(Entity* e) const;

	bool
	isRegistered(Entity* e) const;

private:
	//std::vector<EU::TSharedPointer<Entity>> m_entities;
public:
	std::vector<Entity*> m_entities;
};
