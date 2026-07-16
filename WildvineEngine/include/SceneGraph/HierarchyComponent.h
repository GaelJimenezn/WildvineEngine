/**
 * @file HierarchyComponent.h
 * @brief Declara el componente de jerarquía de entidades para el SceneGraph.
 * @ingroup scenegraph
 *
 * HierarchyComponent permite estructurar entidades en un árbol padre-hijo,
 * habilitando transformaciones encadenadas, visibilidad heredada y
 * agrupaciones lógicas de objetos de la escena.
 */
#pragma once
#include "Prerequisites.h"
#include "ECS/Component.h"

/** @brief Declara class DeviceContext. */
class 
DeviceContext;

/** @brief Declara class Entity. */
class 
Entity;

/**
 * @class HierarchyComponent
 * @brief Componente que permite estructurar entidades en una jerarquía tipo Scene Graph.
 *
 * Gestiona relaciones padre-hijo entre entidades,
 * permitiendo construir estructuras jerárquicas
 * como transformaciones encadenadas o agrupaciones lógicas.
 */
class
HierarchyComponent : public Component {
public:

	/**
	 * @brief Constructor por defecto.
	 *
	 * Inicializa el componente como tipo HIERARCHY.
	 */
	HierarchyComponent() : Component(ComponentType::HIERARCHY) {}

	/**
	 * @brief Destructor por defecto.
	 */
	~HierarchyComponent() = default;

	/**
	 * @brief Inicialización del componente.
	 *
	 * Actualmente no realiza ninguna operación.
	 */
	void
	init() override {}

	/**
	 * @brief Actualización por frame.
	 *
	 * @param Tiempo delta (no utilizado).
	 */
	void
	update(float) override {}

	/**
	 * @brief Render del componente.
	 *
	 * @param deviceContext Contexto de dispositivo gráfico.
	 */
	void
	render(DeviceContext& deviceContext) override {}

	/**
	 * @brief Limpia la jerarquía del componente.
	 *
	 * Elimina todos los hijos y desvincula el padre.
	 */
	void
	destroy() override {
		m_children.clear();
		m_parent = nullptr;
	}

	// API SceneGraph

	/**
	 * @brief Establece el padre de la entidad actual.
	 *
	 * @param parent Puntero a la entidad padre.
	 */
	void
	setParent(Entity* parent) {
		m_parent = parent;
	}

	/**
	 * @brief Indica si la entidad es raíz en la jerarquía.
	 *
	 * @return true si no tiene padre.
	 */
	bool
	isRoot() const {
		return m_parent == nullptr;
	}

	/**
	 * @brief Indica si la entidad tiene hijos.
	 *
	 * @return true si existen entidades hijas.
	 */
	bool
	hasChildren() const {
		return !m_children.empty();
	}

	/**
	 * @brief Agrega una entidad como hija.
	 *
	 * Evita punteros nulos y duplicados.
	 *
	 * @param child Entidad a agregar como hija.
	 */
	void
	addChild(Entity* child) {
		if (!child) {
			return;
		}

		if (std::find(m_children.begin(), m_children.end(), child) != m_children.end()) {
			return;
		}
		m_children.push_back(child);
	}

	/**
	 * @brief Elimina una entidad hija.
	 *
	 * @param child Entidad a remover de la lista de hijos.
	 */
	void
	removeChild(Entity* child) {
		if (!child) return;

		m_children.erase(
			std::remove(m_children.begin(), m_children.end(), child),
			m_children.end()
		);
	}

public:

	/**
	 * @brief Puntero a la entidad padre.
	 *
	 * Es nullptr si la entidad es raíz.
	 */
	Entity* m_parent = nullptr;

	/**
	 * @brief Lista de entidades hijas.
	 */
	std::vector<Entity*> m_children;
};
