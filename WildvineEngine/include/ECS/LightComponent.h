#pragma once
#include "Prerequisites.h"
#include "ECS/Component.h"
#include "Rendering/RenderTypes.h"

class DeviceContext;

class
LightComponent : public Component {
public:
	LightComponent()
		: Component(ComponentType::NONE) {}

	void init() override {}
	void update(float deltaTime) override {}
	void render(DeviceContext& deviceContext) override {}
	void destroy() override {}

	LightData& getLightData() { return m_light; }
	const LightData& getLightData() const { return m_light; }

	void setCastShadow(bool value) { m_castShadow = value; }
	bool canCastShadow() const { return m_castShadow; }

private:
	LightData m_light;
	bool m_castShadow = false;
};
