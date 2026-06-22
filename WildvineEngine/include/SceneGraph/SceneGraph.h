#pragma once
#include "Prerequisites.h"


class Entity;
class DeviceContext;
class Camera;
class RenderScene;

/**
 * @class SceneGraph
 * @brief Grafo de escena jerárquico del motor.
 *
 * El SceneGraph administra la relación padre-hijo entre entidades,
 * permite la propagación de transformaciones y controla la actualización
 * y renderizado jerárquico de la escena.
 */
class
SceneGraph {
public:
	/**
	 * @brief Constructor por defecto.
	 */
	SceneGraph() = default;

	/**
	 * @brief Destructor por defecto.
	 */
	~SceneGraph() = default;

	/**
	 * @brief Inicializa el grafo de escena.
	 *
	 * Prepara estructuras internas necesarias para operar el grafo.
	 */
	void
	init();

	/**
	 * @brief Registra una entidad dentro del grafo de escena.
	 *
	 * @param e Puntero a la entidad a registrar.
	 */
	void
	addEntity(Entity* e);  // registra en el grafo

	/**
	 * @brief Elimina una entidad del grafo de escena.
	 *
	 * @param e Puntero a la entidad a remover.
	 */
	void
	removeEntity(Entity* e);

	/**
	 * @brief Comprueba si una entidad es ancestro de otra.
	 *
	 * @param possibleAncestor Entidad candidata a ser ancestro.
	 * @param node Entidad hija a comprobar.
	 * @return true si possibleAncestor es ancestro de node.
	 */
	bool
	isAncestor(Entity* possibleAncestor, Entity* node) const;

	/**
	 * @brief Adjunta una entidad hija a una entidad padre.
	 *
	 * @param child Entidad que será hija.
	 * @param parent Entidad que será el padre.
	 * @return true si la operación fue exitosa.
	 */
	bool
	attach(Entity* child, Entity* parent);

	/**
	 * @brief Desacopla una entidad de su padre.
	 *
	 * @param child Entidad a desacoplar.
	 * @return true si la operación fue exitosa.
	 */
	bool
	detach(Entity* child);

	/**
	 * @brief Actualiza el estado del grafo de escena.
	 *
	 * Propaga transformaciones y ejecuta la lógica de actualización
	 * de cada entidad registrada.
	 *
	 * @param deltaTime Tiempo transcurrido desde el último frame.
	 * @param deviceContext Contexto del dispositivo para la actualización.
	 */
	void
	update(float deltaTime, DeviceContext& deviceContext);

	/**
	 * @brief Renderiza todas las entidades del grafo de escena.
	 *
	 * El render se realiza respetando la jerarquía del grafo.
	 *
	 * @param deviceContext Contexto del dispositivo para renderizado.
	 */
	void
	render(DeviceContext& deviceContext);

	void
	gatherRenderScene(RenderScene& outScene, const Camera& camera);

	/**
	 * @brief Libera los recursos asociados al grafo de escena.
	 */
	void
	destroy();

private:
	/**
	 * @brief Actualiza recursivamente la matriz de mundo de una entidad.
	 *
	 * @param node Entidad actual a actualizar.
	 * @param parentWorld Matriz de mundo del padre.
	 */
	void
	updateWorldRecursive(Entity* node, const XMMATRIX& parentWorld);

	/**
	 * @brief Comprueba si una entidad es raíz del grafo.
	 *
	 * @param e Entidad a comprobar.
	 * @return true si la entidad no tiene padre.
	 */
	bool
	isRoot(Entity* e) const;

	/**
	 * @brief Comprueba si una entidad está registrada en el grafo.
	 *
	 * @param e Entidad a comprobar.
	 * @return true si la entidad está registrada.
	 */
	bool
  isRegistered(Entity* e) const;

private:
	//std::vector<EU::TSharedPointer<Entity>> m_entities;
public:
	/**
	 * @brief Lista de entidades registradas en el grafo de escena.
	 *
	 * Contiene todas las entidades administradas por el SceneGraph.
	 */
	std::vector<Entity*> m_entities;
};
