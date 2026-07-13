/**
 * @file LightComponent.h
 * @brief Declara la API de LightComponent dentro del subsistema ECS.
 * @ingroup ecs
 */
#pragma once
#include "Prerequisites.h"
#include "ECS/Component.h"
#include "Rendering/RenderTypes.h"

class DeviceContext;

/**
 * @class LightComponent
 * @brief Componente ECS que dota a una entidad de propiedades de iluminación.
 *
 * Almacena un LightData con todos los parámetros de la luz (tipo, color,
 * intensidad, rango, ángulo de cono, dirección, posición) y un flag de
 * proyección de sombras. El SceneGraph lee este componente para poblar
 * el RenderScene con las listas de luces por tipo.
 */
class
LightComponent : public Component {
public:
	/** @brief Constructor. Inicializa el componente sin tipo específico (NONE). */
	LightComponent()
		: Component(ComponentType::NONE) {
	}

	/** @brief Inicialización del componente (sin operación). */
	void 
	init() override {}

	/** @brief Actualización por frame (sin lógica propia). */
	void
	update(float deltaTime) override {}

	/** @brief Render delegado (no tiene representación visual propia). */
	void 
	render(DeviceContext& deviceContext) override {}

	/** @brief Libera recursos (sin recursos GPU propios). */
	void 
	destroy() override {}

	/**
	 * @brief Accede de forma mutable a los datos de la luz.
	 *
	 * Permite modificar tipo, color, intensidad, rango, dirección, etc.
	 *
	 * @return Referencia mutable a LightData.
	 */
	LightData& getLightData() { return m_light; }

	/**
	 * @brief Accede de forma constante a los datos de la luz.
	 * @return Referencia constante a LightData.
	 */
	const LightData& getLightData() const { return m_light; }

	/**
	 * @brief Habilita o deshabilita la proyección de sombras de esta luz.
	 *
	 * Solo la luz direccional principal proyecta sombras en el pipeline actual.
	 *
	 * @param value true para activar la proyección de sombras.
	 */
	void
	setCastShadow(bool value) { m_castShadow = value; }

	/**
	 * @brief Indica si esta luz proyecta sombras.
	 * @return true si la luz genera shadow map.
	 */
	bool 
	canCastShadow() const { return m_castShadow; }

private:
	LightData m_light;          /**< @brief Parámetros completos de la luz (tipo, color, rango, ángulo, etc.). */
	bool      m_castShadow = false; /**< @brief true si esta luz genera un shadow map. */
};
