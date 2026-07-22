#pragma once

#include <glm/glm.hpp>
#include <string>

enum class ColliderKind { Box };

inline std::string colliderKindToString(ColliderKind k) {
  switch (k) {
  case ColliderKind::Box:
    return "Box";
  default:
    return "Invalid";
  }
}

class Collider {
public:
  glm::vec3 center{0.0f};

  explicit Collider(const glm::vec3 &center) : center(center) {}
  virtual ~Collider() = default;

  virtual ColliderKind getType() const = 0;
};

class BoxCollider : public Collider {
public:
  glm::vec3 halfExtents;

  BoxCollider(const glm::vec3 &center, const glm::vec3 &halfExtents)
      : Collider(center), halfExtents(halfExtents) {}

  ColliderKind getType() const override { return ColliderKind::Box; }

  glm::vec3 getMin() const { return center - halfExtents; }
  glm::vec3 getMax() const { return center + halfExtents; }
};
