#include "obstacle.hpp"

#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/constants.hpp>

#include <random>

Obstacle::Obstacle(Mesh *mesh, Shader *shader,
                   std::unique_ptr<Collider> collider,
                   std::mt19937 &rng)
    : object(mesh, glm::vec3(0.7f, 0.4f, 0.4f), nullptr, shader),
      collider(std::move(collider)) {
  pickRotationDir(rng);
  pickRotationSpeed(rng);
}

Obstacle::Obstacle(std::vector<ObjectPart> parts, Shader *shader,
                   std::unique_ptr<Collider> collider,
                   std::mt19937 &rng)
    : object(parts, shader), collider(std::move(collider)) {
  pickRotationDir(rng);
  pickRotationSpeed(rng);
}

void Obstacle::pickRotationDir(std::mt19937 &rng) {
  std::uniform_int_distribution<int> dist(
      0, static_cast<int>(RotationDirection::Count) - 1);
  rotationFirstDir = static_cast<RotationDirection>(dist(rng));
  rotationSecondDir = static_cast<RotationDirection>(dist(rng));
}

void Obstacle::pickRotationSpeed(std::mt19937 &rng) {
  std::uniform_real_distribution<float> dist(
      OBSTACLE_MIN_ROTATION_SPEED, OBSTACLE_MAX_ROTATION_SPEED);
  rotationSpeed = dist(rng);
}

void Obstacle::rotateObjInDir(RotationDirection dir) {
  switch (dir) {
  case RotationDirection::X: {
    object.rotationX = rotationAngle;
    break;
  }

  case RotationDirection::Y: {
    object.rotationY = rotationAngle;
    break;
  }

  case RotationDirection::Z: {
    object.rotationZ = rotationAngle;
    break;
  }

  default: {
    break;
  }
  }
}

void Obstacle::rotateInPlace(float deltaTime) {
  rotationAngle += rotationSpeed * deltaTime;

  if (rotationAngle > glm::two_pi<float>()) {
    rotationAngle -= glm::two_pi<float>();
  }

  rotateObjInDir(rotationFirstDir);
  rotateObjInDir(rotationSecondDir);
}
