#pragma once

#include "collider.hpp"
#include "mesh.hpp"
#include "object.hpp"
#include "shader.hpp"

#include <memory>

#define PLAYER_OBJ_PATH "models/ship/neghvar.obj"

#define PLAYER_SCALE 0.50f, 0.50f, 0.50f

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
         const float strafeLimit);

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
  const float groundY, strafeLimit;
  float moveVel, vertVel, jumpAccel, fallAccel, strafeVel, x, y;

  PlayerStatus status = PlayerStatus::ON_GROUND;

  std::vector<std::unique_ptr<Mesh>> meshes;
  std::unique_ptr<Object> object;
};
