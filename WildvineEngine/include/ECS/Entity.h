#pragma once
/**
 * @file Entity.h
 * @brief Declares the abstract ECS entity contract and component container helpers.
 */
#include "Prerequisites.h"
#include "Component.h"

class DeviceContext;

/**
 * @class Entity
 * @brief Base class for scene objects that own ECS components and participate in update/render flow.
 *
 * Entity exposes lifecycle hooks implemented by concrete objects such as @c Actor and provides
 * templated helpers for attaching and querying component instances stored as shared pointers.
 */
class
    Entity {
public:
    /** @brief Creates an entity with default base state. */
    Entity() = default;

    /**
   * @brief Destructor virtual.
   */
    virtual
        ~Entity() = default;

    /** @brief Early lifecycle hook invoked before init() for object-specific setup. */
    virtual void
        awake() = 0;

    /**
     * @brief Initialize the entity with a device context.
     * @param deviceContext The device context to initialize with.
     * @return True if initialization is successful, false otherwise.
       */
    virtual void
        init() = 0;

    /**
     * @brief Método virtual puro para actualizar el componente.
     * @param deltaTime El tiempo transcurrido desde la última actualización.
     */
    virtual void
        update(float deltaTime, DeviceContext& deviceContext) = 0;

    /**
     * @brief Método virtual puro para renderizar el componente.
     * @param deviceContext Contexto del dispositivo para operaciones gráficas.
     */
    virtual void
        render(DeviceContext& deviceContext) = 0;

    /**
     * @brief Método virtual puro para destruir el componente.
     * Libera los recursos asociados al componente.
       */
    virtual void
        destroy() = 0;

    /**
     * @brief Agrega un componente a la entidad.
     * @tparam T Tipo del componente, debe derivar de Component.
     * @param component Puntero compartido al componente que se va a agregar.
     */
    template <typename T> void
        addComponent(EU::TSharedPointer<T> component) {
        static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");
        m_components.push_back(component.template dynamic_pointer_cast<Component>());
    }

    /**
     * @brief Obtiene un componente de la entidad por su tipo.
     * @tparam T Tipo del componente a obtener.
     * @return Puntero compartido al componente si se encuentra, nullptr en caso contrario.
       */
    template<typename T>
    EU::TSharedPointer<T>
        getComponent() {
        for (auto& component : m_components) {
            EU::TSharedPointer<T> specificComponent = component.template dynamic_pointer_cast<T>();
            if (specificComponent) {
                return specificComponent;
            }
        }
        return EU::TSharedPointer<T>();
    }
private:
protected:
    /** @brief Runtime active flag used by derived entity implementations. */
    bool m_isActive;
    /** @brief Entity identifier used by derived systems/editor code. */
    int m_id;
    /** @brief Components owned by this entity as base-class shared pointers. */
    std::vector<EU::TSharedPointer<Component>> m_components;
};
