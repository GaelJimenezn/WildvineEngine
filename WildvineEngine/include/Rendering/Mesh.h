#pragma once
#include "Prerequisites.h"
#include "Buffer.h"

/**
 * @struct Submesh
 * @brief GPU buffer pair and draw metadata for one mesh section.
 *
 * A submesh maps a vertex/index buffer range to a material slot. Meshes with several
 * materials keep one @c Submesh per independently rendered section.
 */
struct
	Submesh {
	/** @brief Vertex buffer containing this submesh vertex stream. */
	Buffer vertexBuffer;
	/** @brief Index buffer containing this submesh primitive indices. */
	Buffer indexBuffer;
	/** @brief Number of indices to draw for this submesh. */
	unsigned int indexCount = 0;
	/** @brief First index inside @c indexBuffer used for the draw call. */
	unsigned int startIndex = 0;
	/** @brief Material-instance slot used to shade this submesh. */
	unsigned int materialSlot = 0;
};

/**
 * @class Mesh
 * @brief Runtime mesh resource composed of one or more GPU-backed submeshes.
 *
 * The mesh owns the submesh buffers stored in @c m_submeshes and is responsible for
 * destroying them before the mesh is discarded or rebuilt.
 */
class
	Mesh {
public:
	/** @brief Returns mutable submesh storage for mesh construction. */
	std::vector<Submesh>&
		getSubmeshes() { return m_submeshes; }
	/** @brief Returns read-only submesh storage for render traversal. */
	const std::vector<Submesh>&
		getSubmeshes() const { return m_submeshes; }

	/**
	 * @brief Releases all submesh buffers and clears the mesh.
	 *
	 * @post @c m_submeshes is empty and each previous vertex/index buffer has received
	 * destroy().
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
	/** @brief Submesh sections that make up this mesh. */
	std::vector<Submesh> m_submeshes;
};
