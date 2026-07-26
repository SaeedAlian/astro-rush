#pragma once

#include "collider.hpp"
#include "mesh.hpp"
#include "object.hpp"
#include "shader.hpp"

#include <memory>
#include <random>

#define OBSTACLE_MAX_ROTATION_SPEED 0.5f
#define OBSTACLE_MIN_ROTATION_SPEED 0.1f

enum class RotationDirection { X, Y, Z, Count };

class Obstacle {
public:
  Obstacle(Mesh *mesh, Shader *shader,
           std::unique_ptr<Collider> collider, std::mt19937 &rng);
  Obstacle(std::vector<ObjectPart> parts, Shader *shader,
           std::unique_ptr<Collider> collider, std::mt19937 &rng);

  Object &getObject() { return object; }
  const Object &getObject() const { return object; }

  Collider &getCollider() { return *collider; }
  const Collider &getCollider() const { return *collider; }

  void syncCollider() { collider->center = object.pos; }

  void rotateInPlace(float deltaTime);

private:
  float rotationSpeed;
  float rotationAngle = glm::radians(0.0f);

  RotationDirection rotationFirstDir;
  RotationDirection rotationSecondDir;

  Object object;
  std::unique_ptr<Collider> collider;

  void pickRotationDir(std::mt19937 &rng);
  void pickRotationSpeed(std::mt19937 &rng);

  void rotateObjInDir(RotationDirection dir);
};
