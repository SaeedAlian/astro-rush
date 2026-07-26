#include "collision.hpp"

#include <iostream>

namespace {

/*
 * AABB check between two boxes.
 *
 * @param aMin | minimum bound coords for box a
 * @param aMax | maximum bound coords for box a
 * @param bMin | minimum bound coords for box b
 * @param bMax | maximum bound coords for box b
 */
bool boxCompare(glm::vec3 &aMin, glm::vec3 &aMax, glm::vec3 &bMin,
                glm::vec3 &bMax) {
  return (aMin.x < bMax.x && aMax.x > bMin.x) &&
         (aMin.y < bMax.y && aMax.y > bMin.y) &&
         (aMin.z < bMax.z && aMax.z > bMin.z);
}

bool boxAndBox(const BoxCollider &a, const BoxCollider &b) {
  glm::vec3 aMin = a.getMin();
  glm::vec3 aMax = a.getMax();

  glm::vec3 bMin = b.getMin();
  glm::vec3 bMax = b.getMax();

  return boxCompare(aMin, aMax, bMin, bMax);
}

bool crossBoxAndBox(const CrossBoxCollider &a, const BoxCollider &b) {
  glm::vec3 aPrimMin = a.getPrimaryMin();
  glm::vec3 aPrimMax = a.getPrimaryMax();

  glm::vec3 aSecondMin = a.getSecondaryMin();
  glm::vec3 aSecondMax = a.getSecondaryMax();

  glm::vec3 bMin = b.getMin();
  glm::vec3 bMax = b.getMax();

  return boxCompare(aPrimMin, aPrimMax, bMin, bMax) ||
         boxCompare(aSecondMin, aSecondMax, bMin, bMax);
}

bool crossBoxAndCrossBox(const CrossBoxCollider &a,
                         const CrossBoxCollider &b) {
  glm::vec3 aPrimMin = a.getPrimaryMin();
  glm::vec3 aPrimMax = a.getPrimaryMax();

  glm::vec3 aSecondMin = a.getSecondaryMin();
  glm::vec3 aSecondMax = a.getSecondaryMax();

  glm::vec3 bPrimMin = b.getPrimaryMin();
  glm::vec3 bPrimMax = b.getPrimaryMax();

  glm::vec3 bSecondMin = b.getSecondaryMin();
  glm::vec3 bSecondMax = b.getSecondaryMax();

  return boxCompare(aPrimMin, aPrimMax, bPrimMin, bPrimMax) ||
         boxCompare(aSecondMin, aSecondMax, bPrimMin, bPrimMax) ||
         boxCompare(aPrimMin, aPrimMax, bSecondMin, bSecondMax) ||
         boxCompare(aSecondMin, aSecondMax, bSecondMin, bSecondMax);
}

}; // namespace

bool collides(const Collider &a, const Collider &b) {
  ColliderKind typeA = a.getType();
  ColliderKind typeB = b.getType();

  if (typeA == ColliderKind::Box && typeB == ColliderKind::Box) {
    return boxAndBox(static_cast<const BoxCollider &>(a),
                     static_cast<const BoxCollider &>(b));
  } else if ((typeA == ColliderKind::Box &&
              typeB == ColliderKind::CrossBox)) {
    return crossBoxAndBox(static_cast<const CrossBoxCollider &>(b),
                          static_cast<const BoxCollider &>(a));
  } else if (typeA == ColliderKind::CrossBox &&
             typeB == ColliderKind::Box) {
    return crossBoxAndBox(static_cast<const CrossBoxCollider &>(a),
                          static_cast<const BoxCollider &>(b));
  } else if (typeA == ColliderKind::CrossBox &&
             typeB == ColliderKind::CrossBox) {
    return crossBoxAndCrossBox(
        static_cast<const CrossBoxCollider &>(a),
        static_cast<const CrossBoxCollider &>(b));
  } else {
    std::cerr << "[Collision] invalid collider types"
              << "A: " << colliderKindToString(typeA)
              << " B: " << colliderKindToString(typeB) << "\n";
    return false;
  }
}
