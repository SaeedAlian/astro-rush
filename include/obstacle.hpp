#pragma once

#include "collider.hpp"
#include "mesh.hpp"
#include "object.hpp"
#include "shader.hpp"

#include <memory>
#include <random>

#define OBSTACLE_MAX_ROTATION_SPEED 0.5f
#define OBSTACLE_MIN_ROTATION_SPEED 0.1f

/*
 * Determines the rotation axis of the obstacle.
 *
 * Count will determine the number of directions in this enum.
 */
enum class RotationDirection { X, Y, Z, Count };

/*
 * Represents an obstacle entity in the game world.
 *
 * An obstacle combines a renderable Object with a collision shape
 * used for collision detection. It controls the automatic rotation
 * and keeps the collider synchronized with the object's world
 * position.
 *
 * The obstacle owns its Collider instance but does not own the mesh,
 * textures, or shader resources used by the underlying Object.
 */
class Obstacle {
public:
  /*
   * Creates an obstacle with a single mesh.
   *
   * @param mesh
   * @param shader
   * @param collider
   * @param rng | random number generator used for random rotation
   * settings
   */
  Obstacle(Mesh *mesh, Shader *shader,
           std::unique_ptr<Collider> collider, std::mt19937 &rng);

  /*
   * Creates an obstacle with multiple object parts.
   *
   * @param parts
   * @param shader
   * @param collider
   * @param rng | random number generator used for random rotation
   * settings
   */
  Obstacle(std::vector<ObjectPart> parts, Shader *shader,
           std::unique_ptr<Collider> collider, std::mt19937 &rng);

  /*
   * @return reference to the underlying object.
   */
  Object &getObject() { return object; }

  /*
   * @return constant reference to the underlying object.
   */
  const Object &getObject() const { return object; }

  /*
   * @return reference to the collider object.
   */
  Collider &getCollider() { return *collider; }

  /*
   * @return constant reference to the collider object.
   */
  const Collider &getCollider() const { return *collider; }

  /*
   * Syncs the collider center coords with the object's position.
   */
  void syncCollider() { collider->center = object.pos; }

  /*
   * Rotates the obstacle overtime.
   *
   * Applies continuous rotation around the randomly selected axes
   * with randomly selected speed.
   *
   * @param deltaTime
   */
  void rotateInPlace(float deltaTime);

private:
  float rotationSpeed;
  float rotationAngle = glm::radians(0.0f);

  Object object;
  std::unique_ptr<Collider> collider;

  /*
   * First rotation axis (will be selected randomly)
   */
  RotationDirection rotationFirstDir;

  /*
   * Second rotation axis (will be selected randomly)
   */
  RotationDirection rotationSecondDir;

  /*
   * Randomly picks a direction (axis) for the rotation of the
   * obstacle. First and second axis will be selected from
   * RotationDirection.
   *
   * @param rng | reference to the rng passed from constructor
   */
  void pickRotationDir(std::mt19937 &rng);

  /*
   * Randomly picks a rotation speed for the obstacle.
   *
   * @param rng | reference to the rng passed from constructor
   */
  void pickRotationSpeed(std::mt19937 &rng);

  /*
   * Sets the rotation angle of the obstacle based on the input axis
   * (direction).
   *
   * @param dir | axis of the rotation
   */
  void rotateObjInDir(RotationDirection dir);
};
