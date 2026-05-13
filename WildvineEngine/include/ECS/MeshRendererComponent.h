#pragma once
#include "Prerequisites.h"
#include "ECS/Component.h"

class Mesh;
class MaterialInstance;
class DeviceContext;

class
MeshRendererComponent : public Component {
public:
	MeshRendererComponent()
		: Component(ComponentType::MESH) {}

	void init() override {}
	void update(float deltaTime) override {}
	void render(DeviceContext& deviceContext) override {}
	void destroy() override {}

	void setMesh(Mesh* mesh) { m_mesh = mesh; }
	Mesh* getMesh() const { return m_mesh; }

	void setMaterialInstance(MaterialInstance* materialInstance) {
		m_materialInstance = materialInstance;
		m_materialInstances.clear();
		if (materialInstance) {
			m_materialInstances.push_back(materialInstance);
		}
	}
	MaterialInstance* getMaterialInstance() const { return m_materialInstance; }

	void setMaterialInstances(const std::vector<MaterialInstance*>& materialInstances) {
		m_materialInstances = materialInstances;
		m_materialInstance = m_materialInstances.empty() ? nullptr : m_materialInstances.front();
	}

	void addMaterialInstance(MaterialInstance* materialInstance) {
		if (!materialInstance) {
			return;
		}
		if (!m_materialInstance) {
			m_materialInstance = materialInstance;
		}
		m_materialInstances.push_back(materialInstance);
	}

	const std::vector<MaterialInstance*>& getMaterialInstances() const { return m_materialInstances; }

	bool isVisible() const { return m_visible; }
	void setVisible(bool visible) { m_visible = visible; }

	bool canCastShadow() const { return m_castShadow; }
	void setCastShadow(bool value) { m_castShadow = value; }

private:
	Mesh* m_mesh = nullptr;
	MaterialInstance* m_materialInstance = nullptr;
	std::vector<MaterialInstance*> m_materialInstances;
	bool m_visible = true;
	bool m_castShadow = true;
};
