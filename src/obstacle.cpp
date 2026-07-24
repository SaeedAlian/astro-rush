#include "obstacle.hpp"

Obstacle::Obstacle(Mesh *mesh, Shader *shader,
                   std::unique_ptr<Collider> collider)
    : object(mesh, shader), collider(std::move(collider)) {}
