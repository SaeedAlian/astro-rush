#pragma once

#include <glm/glm.hpp>
#include <string>

enum class ColliderKind { Box, CrossBox };

inline std::string colliderKindToString(ColliderKind k) {
  switch (k) {
  case ColliderKind::Box:
    return "Box";
  case ColliderKind::CrossBox:
    return "CrossBox";
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

class CrossBoxCollider : public Collider {
public:
  glm::vec3 primaryBoxHalfExtents;
  glm::vec3 secondaryBoxHalfExtents;
  glm::vec3 secondaryBoxCenterOffset{0.0f};

  CrossBoxCollider(const glm::vec3 &center,
                   const glm::vec3 &secondaryBoxCenterOffset,
                   const glm::vec3 &primaryBoxHalfExtents,
                   const glm::vec3 &secondaryBoxHalfExtents)
      : Collider(center),
        secondaryBoxCenterOffset(secondaryBoxCenterOffset),
        primaryBoxHalfExtents(primaryBoxHalfExtents),
        secondaryBoxHalfExtents(secondaryBoxHalfExtents) {}

  ColliderKind getType() const override {
    return ColliderKind::CrossBox;
  }

  glm::vec3 getPrimaryMin() const {
    return center - primaryBoxHalfExtents;
  }
  glm::vec3 getPrimaryMax() const {
    return center + primaryBoxHalfExtents;
  }

  glm::vec3 getSecondaryMin() const {
    return (center + secondaryBoxCenterOffset) -
           secondaryBoxHalfExtents;
  }
  glm::vec3 getSecondaryMax() const {
    return (center + secondaryBoxCenterOffset) +
           secondaryBoxHalfExtents;
  }
};
