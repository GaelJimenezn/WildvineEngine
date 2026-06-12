#pragma once
#include "Prerequisites.h"
#include "ECS\Component.h"
class DeviceContext;
/**
 * @class MeshComponent
 * @brief Componente ECS que almacena la información de geometría (malla) de un actor.
 *
 * Un @c MeshComponent contiene los vértices e índices que describen la geometría de un
 * objeto.
 * Forma parte del sistema ECS y se asocia a entidades como @c Actor.
 *
 * La malla incluye:
 * - Lista de vértices (posición, normal, UV, etc.).
 * - Lista de índices que definen las primitivas (triángulos, líneas).
 * - Contadores de vértices e índices.
 */
class
	MeshComponent : public Component {
public:
	/**
	 * @brief Constructor por defecto.
	 *
	 * Inicializa el componente de malla con cero vértices e índices
	 * y lo registra como tipo @c MESH en el sistema ECS.
	 */
	MeshComponent() : m_numVertex(0), m_numIndex(0), Component(ComponentType::MESH) {}

	/**
	 * @brief Destructor virtual por defecto.
	 */
	virtual
		~MeshComponent() = default;

	/**
	 * @brief Inicializa el componente de malla.
	 *
	 * Método heredado de @c Component.
	 * Puede usarse para reservar memoria o cargar datos en mallas derivadas.
	 */
	void
		init() override {};

	/**
	 * @brief Actualiza la malla.
	 *
	 * Método heredado de @c Component.
	 * Útil para actualizar animaciones de vértices, morphing u otros procesos relacionados.
	 *
	 * @param deltaTime Tiempo transcurrido desde la última actualización.
	 */
	void
		update(float deltaTime) override {};

	/**
	 * @brief Renderiza la malla.
	 *
	 * Método heredado de @c Component.
	 * Normalmente se usaría junto con @c DeviceContext para dibujar buffers
	 * asociados a la malla.
	 *
	 * @param deviceContext Contexto del dispositivo para operaciones gráficas.
	 */
	void
		render(DeviceContext& deviceContext) override {};

	/**
	 * @brief Libera los recursos asociados al componente de malla.
	 *
	 * Método heredado de @c Component.
	 * En implementaciones más complejas, puede liberar buffers de GPU.
	 */
	void
		destroy() override {};

public:
	/**
	 * @brief Nombre de la malla.
	 */
	std::string m_name;

	/**
	 * @brief Lista de vértices de la malla.
	 */
	std::vector<SimpleVertex> m_vertex;
	std::vector<SkyboxVertex> m_skyVertex;

	/**
	 * @brief Lista de índices que definen las primitivas de la malla.
	 */
	std::vector<unsigned int> m_index;

	/**
	 * @brief Número total de vértices en la malla.
	 */
	int m_numVertex;

	/**
	 * @brief Número total de índices en la malla.
	 */
	int m_numIndex;
};
