#include "SceneGraph/Octree.h"
#include "EngineUtilities/Utilities/Camera.h"

#include <algorithm>

namespace {
constexpr unsigned int kMaxDepth = 5;
constexpr size_t kNodeCapacity = 8;
}

void Octree::build(const std::vector<RenderObject>& objects) {
  m_root.reset();
  if (objects.empty()) return;

  EU::Vector3 minimum = objects.front().boundsMin;
  EU::Vector3 maximum = objects.front().boundsMax;
  for (const RenderObject& object : objects) {
    minimum.x = (std::min)(minimum.x, object.boundsMin.x);
    minimum.y = (std::min)(minimum.y, object.boundsMin.y);
    minimum.z = (std::min)(minimum.z, object.boundsMin.z);
    maximum.x = (std::max)(maximum.x, object.boundsMax.x);
    maximum.y = (std::max)(maximum.y, object.boundsMax.y);
    maximum.z = (std::max)(maximum.z, object.boundsMax.z);
  }

  m_root = std::make_unique<Node>();
  m_root->minimum = minimum;
  m_root->maximum = maximum;
  for (const RenderObject& object : objects) insert(*m_root, object, 0);
}

void Octree::queryVisible(const Camera& camera,
  std::vector<RenderObject>& output,
  CullingStats& stats) const {
  output.clear();
  stats = {};
  if (!m_root) return;
  XMMATRIX viewProjection = camera.getView() * camera.getProj();
  queryNode(*m_root, viewProjection, output, stats);
  stats.visible = static_cast<unsigned int>(output.size());
  stats.culled = stats.submitted - stats.visible;
}

bool Octree::isVisible(const EU::Vector3& minimum,
  const EU::Vector3& maximum,
  const XMMATRIX& viewProjection) {
  bool outsideLeft = true, outsideRight = true;
  bool outsideBottom = true, outsideTop = true;
  bool outsideNear = true, outsideFar = true;
  for (int corner = 0; corner < 8; ++corner) {
    XMVECTOR point = XMVectorSet(
      (corner & 1) ? maximum.x : minimum.x,
      (corner & 2) ? maximum.y : minimum.y,
      (corner & 4) ? maximum.z : minimum.z, 1.0f);
    XMVECTOR clip = XMVector4Transform(point, viewProjection);
    XMFLOAT4 value;
    XMStoreFloat4(&value, clip);
    outsideLeft = outsideLeft && value.x < -value.w;
    outsideRight = outsideRight && value.x > value.w;
    outsideBottom = outsideBottom && value.y < -value.w;
    outsideTop = outsideTop && value.y > value.w;
    outsideNear = outsideNear && value.z < 0.0f;
    outsideFar = outsideFar && value.z > value.w;
  }
  return !(outsideLeft || outsideRight || outsideBottom || outsideTop ||
    outsideNear || outsideFar);
}

bool Octree::fitsInside(const RenderObject& object, const Node& node) {
  return object.boundsMin.x >= node.minimum.x &&
    object.boundsMin.y >= node.minimum.y &&
    object.boundsMin.z >= node.minimum.z &&
    object.boundsMax.x <= node.maximum.x &&
    object.boundsMax.y <= node.maximum.y &&
    object.boundsMax.z <= node.maximum.z;
}

void Octree::subdivide(Node& node) {
  const EU::Vector3 center(
    (node.minimum.x + node.maximum.x) * 0.5f,
    (node.minimum.y + node.maximum.y) * 0.5f,
    (node.minimum.z + node.maximum.z) * 0.5f);
  for (int index = 0; index < 8; ++index) {
    std::unique_ptr<Node> child = std::make_unique<Node>();
    child->minimum = EU::Vector3(
      (index & 1) ? center.x : node.minimum.x,
      (index & 2) ? center.y : node.minimum.y,
      (index & 4) ? center.z : node.minimum.z);
    child->maximum = EU::Vector3(
      (index & 1) ? node.maximum.x : center.x,
      (index & 2) ? node.maximum.y : center.y,
      (index & 4) ? node.maximum.z : center.z);
    node.children[index] = std::move(child);
  }
}

void Octree::insert(Node& node, const RenderObject& object,
  unsigned int depth) {
  if (depth >= kMaxDepth) { node.objects.push_back(object); return; }
  if (!node.children[0] && node.objects.size() >= kNodeCapacity) {
    subdivide(node);
    std::vector<RenderObject> pending = std::move(node.objects);
    for (const RenderObject& current : pending) insert(node, current, depth);
  }
  if (node.children[0]) {
    for (const std::unique_ptr<Node>& child : node.children) {
      if (fitsInside(object, *child)) {
        insert(*child, object, depth + 1);
        return;
      }
    }
  }
  node.objects.push_back(object);
}

void Octree::queryNode(const Node& node, const XMMATRIX& viewProjection,
  std::vector<RenderObject>& output, CullingStats& stats) {
  ++stats.visitedNodes;
  if (!isVisible(node.minimum, node.maximum, viewProjection)) {
    stats.submitted += countObjects(node);
    return;
  }
  for (const RenderObject& object : node.objects) {
    ++stats.submitted;
    if (isVisible(object.boundsMin, object.boundsMax, viewProjection)) {
      output.push_back(object);
    }
  }
  for (const std::unique_ptr<Node>& child : node.children) {
    if (child) queryNode(*child, viewProjection, output, stats);
  }
}

unsigned int Octree::countObjects(const Node& node) {
  unsigned int count = static_cast<unsigned int>(node.objects.size());
  for (const std::unique_ptr<Node>& child : node.children) {
    if (child) count += countObjects(*child);
  }
  return count;
}
