#pragma once
#include "Prerequisites.h"
#include "ECS/Component.h"

class DeviceContext;
class Entity;

/**
 * @class HierarchyComponent
 * @brief Componente encargado de gestionar las relaciones Padre-Hijo dentro del ECS.
 * * Este componente permite construir una estructura de árbol (Scene Graph), facilitando
 * la organización de entidades y la posterior propagación de transformaciones espaciales.
 */
class HierarchyComponent : public Component {
public:
    /**
     * @brief Constructor que inicializa el componente con el tipo HIERARCHY.
     */
    HierarchyComponent() : Component(ComponentType::HIERARCHY) {}

    /** @brief Destructor por defecto. */
    ~HierarchyComponent() = default;

    /** @brief Inicialización del componente (Sobrescrito de Component). */
    void init() override {}

    /** @brief Actualización lógica por frame. */
    void update(float) override {}

    /** @brief Renderizado de elementos relacionados con la jerarquía si fuera necesario. */
    void render(DeviceContext& deviceContext) override {}

    /**
     * @brief Limpia las relaciones de jerarquía.
     * * Desvincula al padre y vacía la lista de hijos para evitar punteros colgados.
     */
    void destroy() override {
        m_children.clear();
        m_parent = nullptr;
    }

    /** @name API SceneGraph */
    ///@{

    /**
     * @brief Define quién es el padre de esta entidad.
     * @param parent Puntero a la entidad padre.
     */
    void setParent(Entity* parent) {
        m_parent = parent;
    }

    /**
     * @brief Comprueba si la entidad es una raíz (no tiene padre).
     * @return true si m_parent es nullptr.
     */
    bool isRoot() const {
        return m_parent == nullptr;
    }

    /**
     * @brief Comprueba si la entidad tiene hijos vinculados.
     * @return true si la lista de hijos no está vacía.
     */
    bool hasChildren() const {
        return !m_children.empty();
    }

    /**
     * @brief Agrega una entidad a la lista de hijos.
     * @note La función verifica si el hijo ya existe en la lista para evitar duplicados.
     * @param child Puntero a la entidad que será tratada como hijo.
     */
    void addChild(Entity* child) {
        if (!child) {
            return;
        }

        if (std::find(m_children.begin(), m_children.end(), child) != m_children.end()) {
            return;
        }
        m_children.push_back(child);
    }

    /**
     * @brief Elimina una entidad específica de la lista de hijos.
     * @param child Puntero a la entidad que se desea desvincular.
     */
    void removeChild(Entity* child) {
        if (!child) return;

        m_children.erase(
            std::remove(m_children.begin(), m_children.end(), child),
            m_children.end()
        );
    }
    ///@}

public:
    /** @brief Puntero a la entidad padre. Si es nulo, esta entidad está en el nivel raíz. */
    Entity* m_parent = nullptr;

    /** @brief Lista de punteros a entidades hijas. */
    std::vector<Entity*> m_children;
};