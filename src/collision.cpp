#include "collision.hpp"
#include <iostream>

namespace {

bool boxAndBox(const BoxCollider &a, const BoxCollider &b) {
  glm::vec3 aMin = a.getMin();
  glm::vec3 aMax = a.getMax();

  glm::vec3 bMin = b.getMin();
  glm::vec3 bMax = b.getMax();

  return (aMin.x < bMax.x && aMax.x > bMin.x) &&
         (aMin.y < bMax.y && aMax.y > bMin.y) &&
         (aMin.z < bMax.z && aMax.z > bMin.z);
}

}; // namespace

bool collides(const Collider &a, const Collider &b) {
  ColliderKind typeA = a.getType();
  ColliderKind typeB = b.getType();

  if (typeA == ColliderKind::Box && typeB == ColliderKind::Box) {
    return boxAndBox(static_cast<const BoxCollider &>(a),
                     static_cast<const BoxCollider &>(b));
  } else {
    std::cerr << "[Collision] invalid collider types"
              << "A: " << colliderKindToString(typeA)
              << " B: " << colliderKindToString(typeB) << "\n";
    return false;
  }
}
