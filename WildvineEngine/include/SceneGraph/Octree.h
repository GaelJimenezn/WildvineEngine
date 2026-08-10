/**
 * @file Octree.h
 * @brief Declara el Ã­ndice espacial usado para frustum culling.
 */
#pragma once

#include "Prerequisites.h"
#include "Rendering/RenderTypes.h"

class Camera;

/** @brief Contadores producidos por una consulta de culling. */
struct CullingStats {
  unsigned int submitted = 0;
  unsigned int visible = 0;
  unsigned int culled = 0;
  unsigned int visitedNodes = 0;
};

/**
 * @class Octree
 * @brief Particiona objetos renderizables para consultar sÃ³lo zonas visibles.
 */
class Octree {
public:
  /** @brief Reconstruye el Ã¡rbol con los objetos del frame actual. */
  void build(const std::vector<RenderObject> &objects);

  /** @brief ReÃºne los objetos que intersectan el frustum de la cÃ¡mara. */
  void queryVisible(const Camera &camera,
                    std::vector<RenderObject> &output,
                    CullingStats &stats) const;

private:
  struct Node {
    EU::Vector3 minimum;
    EU::Vector3 maximum;
    std::vector<RenderObject> objects;
    std::unique_ptr<Node> children[8];
  };

  static bool isVisible(const EU::Vector3 &minimum,
                        const EU::Vector3 &maximum,
                        const XMMATRIX &viewProjection);
  static bool fitsInside(const RenderObject &object, const Node &node);
  static void insert(Node &node, const RenderObject &object, unsigned int depth);
  static void queryNode(const Node &node,
                        const XMMATRIX &viewProjection,
                        std::vector<RenderObject> &output,
                        CullingStats &stats);
  static unsigned int countObjects(const Node &node);
  static void subdivide(Node &node);

  std::unique_ptr<Node> m_root;
};
