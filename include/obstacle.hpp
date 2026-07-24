#pragma once

#include "collider.hpp"
#include "mesh.hpp"
#include "object.hpp"
#include "shader.hpp"

#include <memory>

class Obstacle {
public:
  Obstacle(Mesh *mesh, Shader *shader,
           std::unique_ptr<Collider> collider);

  Object &getObject() { return object; }
  const Object &getObject() const { return object; }

  Collider &getCollider() { return *collider; }
  const Collider &getCollider() const { return *collider; }

  void syncCollider() { collider->center = object.pos; }

private:
  Object object;
  std::unique_ptr<Collider> collider;
};
