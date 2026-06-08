#pragma once
#include "Prerequisites.h"
#include "ECS/Component.h"
#include "Rendering/RenderTypes.h"

class DeviceContext;

/**
 * @class LightComponent
 * @brief ECS component that stores light parameters for render scene extraction.
 *
 * The component owns a @c LightData value used by forward/deferred renderers and exposes
 * a shadow flag for future shadow-map selection. It does not own GPU resources.
 */
class
LightComponent : public Component {
public:
	/** @brief Creates a light component with default directional light data. */
	LightComponent()
		: Component(ComponentType::NONE) {}

	/** @brief Light data needs no explicit initialization. */
	void init() override {}
	/** @brief Light data is externally edited; no automatic per-frame update is performed. */
	void update(float deltaTime) override {}
	/** @brief Light components are gathered by render scene code instead of drawing directly. */
	void render(DeviceContext& deviceContext) override {}
	/** @brief Releases no external resources. */
	void destroy() override {}

	/** @brief Returns mutable light parameters for editor or gameplay changes. */
	LightData& getLightData() { return m_light; }
	/** @brief Returns read-only light parameters for render scene collection. */
	const LightData& getLightData() const { return m_light; }

	/** @brief Enables or disables shadow casting for this light. */
	void setCastShadow(bool value) { m_castShadow = value; }
	/** @brief Returns whether this light is allowed to cast shadows. */
	bool canCastShadow() const { return m_castShadow; }

private:
	/** @brief CPU-side light parameters copied into render-scene light arrays. */
	LightData m_light;
	/** @brief Shadow-casting flag consumed by renderers that support shadow maps. */
	bool m_castShadow = false;
};
