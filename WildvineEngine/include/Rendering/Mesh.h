#pragma once
#include "Prerequisites.h"
#include "Buffer.h"

struct
Submesh {
	Buffer vertexBuffer;
	Buffer indexBuffer;
	unsigned int indexCount = 0;
	unsigned int startIndex = 0;
	unsigned int materialSlot = 0;
};

class
Mesh {
public:
	std::vector<Submesh>& getSubmeshes() { return m_submeshes; }
	const std::vector<Submesh>& getSubmeshes() const { return m_submeshes; }

	void
	destroy() {
		for (Submesh& submesh : m_submeshes) {
			submesh.vertexBuffer.destroy();
			submesh.indexBuffer.destroy();
		}
		m_submeshes.clear();
	}

private:
	std::vector<Submesh> m_submeshes;
};
