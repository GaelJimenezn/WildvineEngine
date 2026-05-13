#include "SceneGraph\SceneGraph.h"
#include "SceneGraph\HierarchyComponent.h"
#include "ECS\Entity.h"
#include "ECS\Transform.h"
#include "ECS\LightComponent.h"
#include "ECS\MeshRendererComponent.h"
#include "DeviceContext.h"
#include "EngineUtilities/Utilities/Camera.h"
#include "Rendering/Material.h"
#include "Rendering/MaterialInstance.h"
#include "Rendering/RenderScene.h"

void SceneGraph::init() {
	m_entities.clear();
}

void SceneGraph::destroy() {
	for (Entity* e : m_entities)
	{
		if (!e) continue;
		auto h = e->getComponent<HierarchyComponent>();
		if (h)
		{
			h->m_parent = nullptr;
			h->m_children.clear();
		}
	}

	m_entities.clear();
}

void 
SceneGraph::addEntity(Entity* e) {
	if (!e) {
		return;
	}
	if (isRegistered(e)) {
		return;
	}

	//	// Validar que existen los componentes minimos
	if (!e->getComponent<Transform>()) {
		e->addComponent(EU::MakeShared<Transform>());
		e->getComponent<Transform>()->init();
	}
	if (!e->getComponent<HierarchyComponent>()) {
		e->addComponent(EU::MakeShared<HierarchyComponent>());
		e->getComponent<HierarchyComponent>()->init();
	}

	m_entities.push_back(e);
}

void 
SceneGraph::removeEntity(Entity* e) {
	if (!e) return;
	if (!isRegistered(e)) return;

	// 1) Detach de su padre (si tiene)
	detach(e);

	// 2) Reparent de hijos a null (roots) o detach total
	auto h = e->getComponent<HierarchyComponent>();
	if (h)
	{
		// Copia local para no invalidar mientras iteras
		auto childrenCopy = h->m_children;
		for (Entity* c : childrenCopy)
		{
			if (!c) continue;
			// detach del padre (que es e)
			auto hc = c->getComponent<HierarchyComponent>();
			if (hc && hc->m_parent == e)
				hc->m_parent = nullptr;

			// quitar referencia en e
			h->removeChild(c);

			// marcar dirty para recalcular world
			auto wt = c->getComponent<Transform>();
			//if (wt) wt->dirty = true;
			//markWorldDirtyRecursive(wt);
		}

		h->m_children.clear();
	}

	// 3) eliminar del registro
	m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), e), m_entities.end());
}

bool 
SceneGraph::isAncestor(Entity* possibleAncestor, Entity* node) const {
	// Recorre hacia arriba desde node: si encuentra possibleAncestor, hay ciclo
	if (!possibleAncestor || !node) return false;

	auto h = node->getComponent<HierarchyComponent>();
	while (h && h->m_parent)
	{
		if (h->m_parent == possibleAncestor) return true;
		node = h->m_parent;
		EU::TSharedPointer<HierarchyComponent> h;
		if (node)
			h = node->getComponent<HierarchyComponent>();

	}
	return false;
}

bool
SceneGraph::isRoot(Entity* e) const {
	
	if (!e) return false;
	auto h = e->getComponent<HierarchyComponent>();
	return (!h || h->m_parent == nullptr);
}

bool 
SceneGraph::isRegistered(Entity* e) const {
	return std::find(m_entities.begin(), m_entities.end(), e) != m_entities.end();
}

bool 
SceneGraph::attach(Entity* child, Entity* parent)
{
	if (!child || !parent) return false;
	if (child == parent) return false;

	// Registro automático
	addEntity(child);
	addEntity(parent);

	// Evita ciclos: parent no puede estar debajo de child
	if (isAncestor(child, parent)) return false;

	// Si child ya tiene padre, detach
	detach(child);

	auto hc = child->getComponent<HierarchyComponent>();
	auto hp = parent->getComponent<HierarchyComponent>();
	if (!hc || !hp) return false;

	hc->m_parent = parent;
	hp->addChild(child);

	//markWorldDirtyRecursive(wt);
	return true;
}

bool 
SceneGraph::detach(Entity* child) {
	if (!child) return false;

	auto hc = child->getComponent<HierarchyComponent>();
	if (!hc) return false;

	Entity* parent = hc->m_parent;
	if (!parent) return true; // ya estaba root

	auto hp = parent->getComponent<HierarchyComponent>();
	if (hp) hp->removeChild(child);

	hc->m_parent = nullptr;

	//markWorldDirtyRecursive(wt);
	return true;
}

void
SceneGraph::update(float deltaTime, DeviceContext& deviceContext) {
	// Actualiza todas las entidades
	for (Entity* e : m_entities)
	{
		if (!e) continue;
		e->update(deltaTime, deviceContext);
	}

	// 2) Propagación World: procesa roots
	for (Entity* e : m_entities)
	{
		if (!e) continue;
		if (isRoot(e))
		{
			updateWorldRecursive(e, XMMatrixIdentity());
		}
	}
}

void 
SceneGraph::updateWorldRecursive(Entity* node, const XMMATRIX& parentWorld) {
	auto t = node->getComponent<Transform>();
	// Dirty Matrix?
	auto h = node->getComponent<HierarchyComponent>();

	if (!t || !h) {
		return;
	}
	// Tu Transform::matrix es LOCAL (S*R*T)
	// World = Local * ParentWorld
	auto worldMatrix = t->matrix * parentWorld;
	t->worldMatrix = worldMatrix;

	for (Entity* c : h->m_children) {
			updateWorldRecursive(c, worldMatrix);
	}
}

void SceneGraph::render(DeviceContext& deviceContext) {
	// Render all entities
	for (auto& e : m_entities) {
		if (e) {
			e->render(deviceContext);
		}
	}
}

void
SceneGraph::gatherRenderScene(RenderScene& outScene, const Camera& camera) {
	for (Entity* entity : m_entities)
	{
		if (!entity) {
			continue;
		}

		auto lightComponent = entity->getComponent<LightComponent>();
		if (lightComponent) {
			outScene.directionalLights.push_back(lightComponent->getLightData());
		}

		auto meshRenderer = entity->getComponent<MeshRendererComponent>();
		auto transform = entity->getComponent<Transform>();
		if (!meshRenderer || !transform || !meshRenderer->isVisible()) {
			continue;
		}

		RenderObject renderObject{};
		renderObject.mesh = meshRenderer->getMesh();
		renderObject.materialInstance = meshRenderer->getMaterialInstance();
		renderObject.materialInstances = meshRenderer->getMaterialInstances();
		renderObject.world = transform->worldMatrix;
		renderObject.castShadow = meshRenderer->canCastShadow();

		EU::Vector3 cameraPos = camera.getPosition();
		XMFLOAT4X4 worldMatrix{};
		XMStoreFloat4x4(&worldMatrix, transform->worldMatrix);
		EU::Vector3 objectPos = EU::Vector3(worldMatrix._41, worldMatrix._42, worldMatrix._43);
		float dx = objectPos.x - cameraPos.x;
		float dy = objectPos.y - cameraPos.y;
		float dz = objectPos.z - cameraPos.z;
		renderObject.distanceToCamera = dx * dx + dy * dy + dz * dz;

		MaterialDomain domain = MaterialDomain::Opaque;
		if (renderObject.materialInstance &&
			renderObject.materialInstance->getMaterial()) {
			domain = renderObject.materialInstance->getMaterial()->getDomain();
		}

		renderObject.transparent = (domain == MaterialDomain::Transparent);
		if (renderObject.transparent) {
			outScene.transparentObjects.push_back(renderObject);
		}
		else {
			outScene.opaqueObjects.push_back(renderObject);
		}
	}
}

