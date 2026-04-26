#pragma once
#include "Prerequisites.h"
#include "Buffer.h"

/**
 * @struct Submesh
 * @brief Subdivisión de una malla que agrupa la geometría que debe renderizarse con un único material.
 */
struct
Submesh {
	Buffer vertexBuffer;            /**< Buffer de vértices de la GPU. */
	Buffer indexBuffer;             /**< Buffer de índices de la GPU. */
	unsigned int indexCount = 0;    /**< Cantidad total de índices a dibujar en esta sub-malla. */
	unsigned int startIndex = 0;    /**< Desplazamiento de inicio dentro del index buffer principal. */
	unsigned int materialSlot = 0;  /**< Índice que asocia esta geometría a un material en el array del RenderObject. */
};

/**
 * @class Mesh
 * @brief Contenedor geométrico que posee una o múltiples sub-mallas listas para renderizarse.
 */
class
Mesh {

public:
	/**
	 * @brief Obtiene la colección de sub-mallas contenidas en este Mesh.
	 * @return Referencia al vector de Submeshes.
	 */
	std::vector<Submesh>& 
	getSubmeshes() { return m_submeshes; }

	/**
	 * @brief Obtiene de forma constante la colección de sub-mallas contenidas.
	 * @return Referencia constante al vector de Submeshes.
	 */
	const std::vector<Submesh>& 
	getSubmeshes() const { return m_submeshes; }

	/**
	 * @brief Libera la memoria y destruye los buffers de la GPU de todas las sub-mallas internas.
	 */
	void
	destroy() {
		for (Submesh& submesh : m_submeshes) {
			submesh.vertexBuffer.destroy();
			submesh.indexBuffer.destroy();
		}
		m_submeshes.clear();
	}

private:
	std::vector<Submesh> m_submeshes; /**< Lista de todas las sub-mallas que conforman esta geometría. */
};