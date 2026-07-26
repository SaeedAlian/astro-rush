#pragma once

#include "collider.hpp"
#include "mesh.hpp"
#include "object.hpp"
#include "shader.hpp"

#include <memory>

#define PLAYER_OBJ_PATH "models/ship/neghvar.obj"

#define PLAYER_COLLIDER_SIZE_SAFE_SCALE 0.80f

#define PLAYER_COLLIDER_WING_WIDTH_FACTOR 0.85f
#define PLAYER_COLLIDER_WING_HEIGHT_FACTOR 0.7f
#define PLAYER_COLLIDER_WING_DEPTH_FACTOR 0.45f

#define PLAYER_COLLIDER_BODY_WIDTH_FACTOR 0.25f
#define PLAYER_COLLIDER_BODY_HEIGHT_FACTOR 1.0f
#define PLAYER_COLLIDER_BODY_DEPTH_FACTOR 1.0f

#define PLAYER_COLLIDER_WING_CENTER_Z_OFFSET_FACTOR 0.75f

#define PLAYER_MOVE_ACCELERATION 8.0f
#define PLAYER_STRAFE_ACCELERATION 5.0f
#define PLAYER_MOVE_DECELERATION -15.0f
#define PLAYER_STRAFE_DECELERATION -10.0f
#define PLAYER_ALTITUDE_ACCEL_FACTOR_WITH_MOVE_VELOCITY 0.5f

#define PLAYER_MIN_MOVE_VELOCITY 15.0f
#define PLAYER_MAX_MOVE_VELOCITY 40.0f
#define PLAYER_MIN_STRAFE_VELOCITY 10.0f
#define PLAYER_MAX_STRAFE_VELOCITY 30.0f

#define PLAYER_ROTATION_SPEED 60.0f
#define PLAYER_MAX_ROTATION_ANGLE_Z 20.0f
#define PLAYER_MAX_ROTATION_ANGLE_X 20.0f

enum class PlayerAltitudeStatus {
  ON_LOW = 0,
  GAINING = 1,
  LOSING = 2,
  ON_HIGH = 3,
};

struct PlayerOptions {
  float initMoveVelocity;
  float initStrafeVelocity;
  float initAltitudeAcceleration;
  float maxAltitude;
  float scale;
  bool flipZ;
};

class Player {
public:
  Player(const PlayerOptions options, Shader *shader,
         const float strafeLimit);

  void moveLeft(float deltaTime);
  void moveRight(float deltaTime);
  void changeAltitude();

  void accelerate();
  void decelerate();

  void update(float deltaTime);

  void draw(const glm::mat4 &view, const glm::mat4 &projection) const;

  CrossBoxCollider getCollider() const;

  const glm::vec3 &getSize() const { return scaledSize; };

  float getMoveVelocity() const { return moveVel; };

  Object &getObject() { return *object.get(); }
  const Object &getObject() const { return *object.get(); }

private:
  static constexpr float noAltChangeDuration = 0.2f;
  static constexpr float accelerationDuration = 0.1f;
  static constexpr float rotationSpeed =
      glm::radians(PLAYER_ROTATION_SPEED);
  static constexpr float maxRotationAngleZ =
      glm::radians(PLAYER_MAX_ROTATION_ANGLE_Z);
  static constexpr float maxRotationAngleX =
      glm::radians(PLAYER_MAX_ROTATION_ANGLE_X);

  const PlayerOptions opts;
  const float strafeLimit;

  float x, y, prevX, prevY;
  float moveVel, vertVel, altitudeAccel, strafeVel;
  float lowAltY, highAltY;

  float moveAccel = 0.0f;
  float strafeAccel = 0.0f;

  float rotationAngleZ = 0.0f;
  float rotationAngleX = 0.0f;

  float altChangeTimer = 0.0f;
  float accelerationTimer = 0.0f;

  std::unique_ptr<MeshBounds> bounds;

  glm::vec3 scaledSize;

  PlayerAltitudeStatus altitudeStatus = PlayerAltitudeStatus::ON_LOW;

  std::vector<std::unique_ptr<Mesh>> meshes;
  std::unique_ptr<Object> object;
  std::unique_ptr<CrossBoxCollider> collider;

  void updateAltitude(float deltaTime);
  void updateVelocities(float deltaTime);
  void updateRotationZ(float deltaTime);
  void updateRotationX(float deltaTime);
};
