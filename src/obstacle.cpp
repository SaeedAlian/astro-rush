#include "obstacle.hpp"

#include <glm/ext/vector_float3.hpp>

Obstacle::Obstacle(Mesh *mesh, Shader *shader,
                   std::unique_ptr<Collider> collider)
    : object(mesh, glm::vec3(0.7f, 0.4f, 0.4f), nullptr, shader),
      collider(std::move(collider)) {}
