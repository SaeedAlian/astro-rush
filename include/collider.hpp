#pragma once

#include <glm/glm.hpp>

#include <string>

/*
 * Types of collider shapes supported by the collision system.
 */
enum class ColliderKind { Box, CrossBox };

/*
 * Converts a collider type enum to its string representation.
 *
 * @param k | collider type
 *
 * @return string representation of the collider type
 */
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

/*
 * Abstract base class for collision detection shapes.
 *
 * A collider represents the geometric volume used to detect
 * intersections between objects. Each collider has a center position
 * and provides its specific collider type through getType().
 */
class Collider {
public:
  /*
   * Center position of the collider in world coordinates.
   */
  glm::vec3 center{0.0f};

  /*
   * Creates a collider at the given world position.
   *
   * @param center | initial center position of the collider
   */
  explicit Collider(const glm::vec3 &center) : center(center) {}
  virtual ~Collider() = default;

  /*
   * Returns the type of this collider.
   *
   * @return collider type.
   */
  virtual ColliderKind getType() const = 0;
};

/*
 * Axis-Aligned Bounding Box (AABB) collider.
 *
 * Represents a rectangular collision volume aligned with the world
 * axes. The collider is defined by its center position and half
 * extents, where each half-extent represents half of the box size
 * along an axis.
 *
 * This collider can be tested using AABB collision detection.
 */
class BoxCollider : public Collider {
public:
  /*
   * Distance from the center to each face of the box.
   */
  glm::vec3 halfExtents;

  /*
   * Creates an AABB collider.
   *
   * @param center | center position of the box
   * @param halfExtents | half size of the box along each axis
   */
  BoxCollider(const glm::vec3 &center, const glm::vec3 &halfExtents)
      : Collider(center), halfExtents(halfExtents) {}

  /*
   * @return ColliderKind::Box
   */
  ColliderKind getType() const override { return ColliderKind::Box; }

  /*
   * Returns the minimum corner of the bounding box.
   *
   * @return world space minimum coordinates of the box
   */
  glm::vec3 getMin() const { return center - halfExtents; }

  /*
   * Returns the maximum corner of the bounding box.
   *
   * @return world space maximum coordinates of the box
   */
  glm::vec3 getMax() const { return center + halfExtents; }
};

/*
 * Collider composed of two overlapping AABBs.
 *
 * It consists of a primary box centered at the collider position and
 * a secondary box positioned using an offset from the center.
 *
 * Both boxes can be tested independently using AABB collision
 * detection.
 */
class CrossBoxCollider : public Collider {
public:
  /*
   * Half extents of the primary box.
   */
  glm::vec3 primaryBoxHalfExtents;

  /*
   * Half extents of the secondary box.
   */
  glm::vec3 secondaryBoxHalfExtents;

  /*
   * Center offset of the secondary box, relative to the primary box's
   * center.
   */
  glm::vec3 secondaryBoxCenterOffset{0.0f};

  /*
   * Creates a cross box collider.
   *
   * @param center | center position of the collider
   * @param secondaryBoxCenterOffset | offset of the secondary box
   * from the center
   * @param primaryBoxHalfExtents | primary box half extents
   * @param secondaryBoxHalfExtents | secondary box half extents
   */
  CrossBoxCollider(const glm::vec3 &center,
                   const glm::vec3 &secondaryBoxCenterOffset,
                   const glm::vec3 &primaryBoxHalfExtents,
                   const glm::vec3 &secondaryBoxHalfExtents)
      : Collider(center),
        secondaryBoxCenterOffset(secondaryBoxCenterOffset),
        primaryBoxHalfExtents(primaryBoxHalfExtents),
        secondaryBoxHalfExtents(secondaryBoxHalfExtents) {}

  /*
   * @return ColliderKind::CrossBox
   */
  ColliderKind getType() const override {
    return ColliderKind::CrossBox;
  }

  /*
   * Returns the minimum corner of the primary bounding box.
   *
   * @return world space minimum coordinates of the primary box
   */
  glm::vec3 getPrimaryMin() const {
    return center - primaryBoxHalfExtents;
  }

  /*
   * Returns the maximum corner of the primary bounding box.
   *
   * @return world space maximum coordinates of the primary box
   */
  glm::vec3 getPrimaryMax() const {
    return center + primaryBoxHalfExtents;
  }

  /*
   * Returns the minimum corner of the secondary bounding box.
   *
   * @return world space minimum coordinates of the secondary box
   */
  glm::vec3 getSecondaryMin() const {
    return (center + secondaryBoxCenterOffset) -
           secondaryBoxHalfExtents;
  }

  /*
   * Returns the maximum corner of the secondary bounding box.
   *
   * @return world space maximum coordinates of the secondary box
   */
  glm::vec3 getSecondaryMax() const {
    return (center + secondaryBoxCenterOffset) +
           secondaryBoxHalfExtents;
  }
};
