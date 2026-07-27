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

#define PLAYER_SCALE 0.5f
#define PLAYER_INIT_MOVE_VELOCITY 22.0f
#define PLAYER_INIT_STRAFE_VELOCITY 16.0f
#define PLAYER_INIT_ALTITUDE_ACCEL 30.0f
#define PLAYER_MAX_ALTITUDE 4.5f

#define PLAYER_FLIP_Z false

#define PLAYER_MIN_MOVE_VELOCITY 15.0f
#define PLAYER_MAX_MOVE_VELOCITY 40.0f
#define PLAYER_MIN_STRAFE_VELOCITY 10.0f
#define PLAYER_MAX_STRAFE_VELOCITY 30.0f

#define PLAYER_ROTATION_SPEED 60.0f
#define PLAYER_MAX_ROTATION_ANGLE_Z 20.0f
#define PLAYER_MAX_ROTATION_ANGLE_X 20.0f

#define PLAYER_NO_ALTITUDE_CHANGE_DURATION 0.2f
#define PLAYER_ACCELERATION_DURATION 0.1f

enum class PlayerAltitudeStatus {
  /*
   * Player is at the lower altitude level.
   */
  ON_LOW = 0,

  /*
   * Player is currently moving upward toward the higher altitude
   * level.
   */
  GAINING = 1,

  /*
   * Player is currently moving downward toward the lower altitude
   * level.
   */
  LOSING = 2,

  /*
   * Player is at the higher altitude level.
   */
  ON_HIGH = 3,
};

/*
 * Represents the player entity in the game.
 * It is responsible for drawing and updating the player's
 * coordinates, velocities, accelerations and rotations.
 *
 * It is also owns the collider object of the player and can return
 * the updated collider via a method.
 */
class Player {
public:
  /*
   * Initializes the Player object in the world.
   *
   * Loads the player object model and textures, sets up the object,
   * its scaling, sizes and colliders.
   *
   * @param shader
   * @param strafeLimit | the player strafe limit coordinates (in x
   * axis)
   */
  Player(Shader *shader, const float strafeLimit);

  /*
   * Changes the x coordinates of the player (decreases it), so in
   * result player goes left by the strafeVel with respect of the
   * time.
   *
   * @param deltaTime
   */
  void moveLeft(float deltaTime);

  /*
   * Changes the x coordinates of the player (increases it), so in
   * result player goes right by the strafeVel with respect of the
   * time.
   *
   * @param deltaTime
   */
  void moveRight(float deltaTime);

  /*
   * Toggles the altitudeStatus of the player between ON_LOW/ON_HIGH
   * by first changing it to LOSING/GAINING.
   *
   * Sets the vertVel to zero, and altChangeTimer to its duration, so
   * it will prevent the altitude change immediately after previous
   * altitude changing.
   *
   * If the altChangeTimer is not finished, it won't work.
   */
  void changeAltitude();

  /*
   * Accelerates the player by setting the moveAccel and strafeAccel
   * to the default positive values.
   *
   * Also sets the acceleration timer, so it won't accelerate again
   * immediately after previous acceleration.
   */
  void accelerate();

  /*
   * Decelerates the player by setting the moveAccel and strafeAccel
   * to the default negative values.
   *
   * Also sets the acceleration timer, so it won't decelerate again
   * immediately after previous deceleration.
   */
  void decelerate();

  /*
   * Updates all the properties of the player.
   *
   * Updates the velocities, altitude, rotations and position
   * coordinates.
   *
   * All changes are with the respect of the time.
   *
   * @param deltaTime
   */
  void update(float deltaTime);

  /*
   * Draws the player object using the given camera matrices.
   *
   * Sets the required shader uniforms and renders all player meshes.
   *
   * @param view | view matrix (transforms world coordinates to camera
   * coordinates)
   * @param projection | projection matrix (transforms camera
   * coordinates to screen coordinates)
   */
  void draw(const glm::mat4 &view, const glm::mat4 &projection) const;

  /*
   * Creates and returns the current collider of the player.
   *
   * @return player collider
   */
  CrossBoxCollider getCollider() const;

  /*
   * @return reference to the scaled size vector
   */
  const glm::vec3 &getSize() const { return scaledSize; };

  /*
   * @return movement velocity
   */
  float getMoveVelocity() const { return moveVel; };

  /*
   * @return movement acceleration
   */
  float getMoveAcceleration() const { return moveAccel; };

  /*
   * @return reference to the player object
   */
  Object &getObject() { return *object.get(); }

  /*
   * @return constant reference to the player object
   */
  const Object &getObject() const { return *object.get(); }

private:
  static constexpr float rotationSpeed =
      glm::radians(PLAYER_ROTATION_SPEED);
  static constexpr float maxRotationAngleZ =
      glm::radians(PLAYER_MAX_ROTATION_ANGLE_Z);
  static constexpr float maxRotationAngleX =
      glm::radians(PLAYER_MAX_ROTATION_ANGLE_X);

  const float playerScale = PLAYER_SCALE;
  const bool playerFlipZ = PLAYER_FLIP_Z;
  const float maxAltitude = PLAYER_MAX_ALTITUDE;

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

  /*
   * Updates the y coord (altitude) based on the vertVel and
   * altitudeAccel.
   *
   * The velocity will be changed with respect of the time, and the
   * altitudeStatus of the player.
   *
   * //clang-format off
   * If player:
   *     LOSING altitude: velocity decreases.
   *     GAINING altitude: velocity increases.
   *     ON_LOW or ON_HIGH altitude: velocity will be set to 0.
   * //clang-format on
   *
   * Player altitude status will be changed to LOSING or GAINING by
   * the changeAltitude method. If the player reaches highAltY, then
   * status will be set to ON_HIGH. If the player reaches lowAltY,
   * then status will be set to ON_LOW.
   *
   *
   * All velocity changes are with the altitudeAccel.
   *
   * altitudeAccel itself will be determined by a bell curve with
   * respect of the y coords.
   *
   * If the moveVel changes, it will have an effect on
   * the altitudeAccel by a factor
   * (PLAYER_ALTITUDE_ACCEL_FACTOR_WITH_MOVE_VELOCITY).
   *
   * @param deltaTime
   */
  void updateAltitude(float deltaTime);

  /*
   * Updates moveVel and strafeVel with respect of the time, based on
   * the moveAccel and strafeAccel.
   *
   * It will clamp the velocities.
   *
   * accelerationTimer will be counted down until reaches zero.
   *
   * If accelerationTimer reaches zero the acceleration values
   * (moveAccel and strafeAccel) will be set to zero.
   *
   * @param deltaTime
   */
  void updateVelocities(float deltaTime);

  /*
   * Change the z axis rotation based on delta x.
   *
   * If the player moves left or right, the z axis rotation will be
   * changed too.
   *
   * Rotation will be based on rotationSpeed and it will be clamped
   * between 0.0 and maxRotationAngleZ.
   *
   * @param deltaTime
   */
  void updateRotationZ(float deltaTime);

  /*
   * Change the x axis rotation based on delta y or having
   * acceleration.
   *
   * If the player gains or loses altitude, accelerates or
   * decelerates, the z axis rotation will be changed too.
   *
   * Rotation will be based on rotationSpeed and it will be clamped
   * between 0.0 and maxRotationAngleX.
   *
   * @param deltaTime
   */
  void updateRotationX(float deltaTime);
};
