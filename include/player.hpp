#pragma once

#include "collider.hpp"
#include "mesh.hpp"
#include "object.hpp"
#include "shader.hpp"

#include <memory>

enum class PlayerStatus {
  ON_GROUND = 0,
  JUMPING = 1,
  FALLING = 2,
};

struct PlayerOptions {
  float initMoveVelocity;
  float initStrafeVelocity;

  float initVerticalAcceleration;
  float jumpAccelerationFactor;
  float fallAccelerationFactor;

  float jumpHeight;
  float size;
};

class Player {
public:
  Player(const PlayerOptions options, Shader *shader,
         const float laneLimit);

  void moveLeft(float deltaTime);
  void moveRight(float deltaTime);
  void jump();

  void update(float deltaTime);

  void draw(const glm::mat4 &view, const glm::mat4 &projection) const;

  BoxCollider getCollider() const;

  Object &getObject() { return *object.get(); }
  const Object &getObject() const { return *object.get(); }

private:
  const PlayerOptions opts;
  const float groundY, laneLimit;
  float moveVel, vertVel, jumpAccel, fallAccel, strafeVel, x, y;

  PlayerStatus status = PlayerStatus::ON_GROUND;

  std::unique_ptr<Mesh> mesh;
  std::unique_ptr<Object> object;
};
