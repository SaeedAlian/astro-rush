#include "player.hpp"

#include <glm/ext/vector_float3.hpp>

#include <algorithm>
#include <vector>

namespace {

float getAltitudeAcceleration(float y, float maxAccel,
                              float peakHeight) {
  float sigma = 2.0f;
  float curveCenter = peakHeight * 0.6f;
  float diff = y - curveCenter;
  float factor = std::exp(-(diff * diff) / (2.0f * sigma * sigma));
  return maxAccel * factor;
}

} // namespace

Player::Player(const PlayerOptions options, Shader *shader,
               const float strafeLimit)
    : opts(options), strafeLimit(strafeLimit),
      moveVel(options.initMoveVelocity), vertVel(0.0f),
      altitudeAccel(options.initAltitudeAcceleration),
      strafeVel(options.initStrafeVelocity) {

  bounds = std::make_unique<MeshBounds>();

  auto loadedParts = Mesh::loadObj(PLAYER_OBJ_PATH, bounds.get());

  glm::vec3 size = bounds->size();
  glm::vec3 center = bounds->center();

  auto scale = glm::vec3(opts.scale, opts.scale,
                         opts.flipZ ? -opts.scale : opts.scale);

  scaledSize = glm::abs(size * scale);
  glm::vec3 scaledCenter = center * scale;

  lowAltY = scaledSize.y * 0.5f;
  highAltY = opts.maxAltitude - scaledSize.y * 0.5f;

  std::vector<ObjectPart> objParts;

  for (auto &part : loadedParts) {
    meshes.push_back(std::make_unique<Mesh>(std::move(part.mesh)));

    Texture *tex = nullptr;
    if (!part.material.diffuseTexPath.empty()) {
      tex = Texture::load(part.material.diffuseTexPath);
    }

    objParts.push_back(
        {meshes.back().get(), part.material.diffuseColor, tex});
  }

  // setup collider parts
  {
    glm::vec3 colliderSize =
        scaledSize * glm::vec3(PLAYER_COLLIDER_SIZE_SAFE_SCALE);

    float wingDepth =
        colliderSize.z * PLAYER_COLLIDER_WING_DEPTH_FACTOR;
    float wingWidth =
        colliderSize.x * PLAYER_COLLIDER_WING_WIDTH_FACTOR;
    float wingHeight =
        colliderSize.y * PLAYER_COLLIDER_WING_HEIGHT_FACTOR;

    float bodyWidth =
        colliderSize.x * PLAYER_COLLIDER_BODY_WIDTH_FACTOR;
    float bodyHeight =
        colliderSize.y * PLAYER_COLLIDER_BODY_HEIGHT_FACTOR;
    float bodyDepth =
        colliderSize.z * PLAYER_COLLIDER_BODY_DEPTH_FACTOR;

    auto wingCenterOffset =
        glm::vec3(0.0f, 0.0f,
                  (colliderSize.z * 0.5f) -
                      (wingDepth *
                       PLAYER_COLLIDER_WING_CENTER_Z_OFFSET_FACTOR));

    auto bodyHalfExtents = glm::vec3(
        bodyWidth * 0.5f, bodyHeight * 0.5f, bodyDepth * 0.5f);

    auto wingHalfExtents = glm::vec3(
        wingWidth * 0.5f, wingHeight * 0.5f, wingDepth * 0.5f);

    collider = std::make_unique<CrossBoxCollider>(
        scaledCenter, wingCenterOffset, bodyHalfExtents,
        wingHalfExtents);
  }

  x = 0.0f;
  prevX = x;
  y = lowAltY;
  prevY = y;

  object = std::make_unique<Object>(std::move(objParts), shader);
  object->rotationZ = 0.0f;
  object->pos = glm::vec3(0.0f, y, 0.0f);
  object->scale = scale;
}

CrossBoxCollider Player::getCollider() const {
  return CrossBoxCollider(collider->center + object->pos,
                          collider->secondaryBoxCenterOffset,
                          collider->primaryBoxHalfExtents,
                          collider->secondaryBoxHalfExtents);
}

void Player::moveRight(float deltaTime) {
  x += strafeVel * deltaTime;
  if (x > strafeLimit) {
    x = strafeLimit;
  }
}

void Player::moveLeft(float deltaTime) {
  x -= strafeVel * deltaTime;
  if (x < -strafeLimit) {
    x = -strafeLimit;
  }
}

void Player::changeAltitude() {
  if (altChangeTimer > 0.0f) {
    return;
  }

  if (altitudeStatus == PlayerAltitudeStatus::GAINING ||
      altitudeStatus == PlayerAltitudeStatus::ON_HIGH) {
    altitudeStatus = PlayerAltitudeStatus::LOSING;
  } else {
    altitudeStatus = PlayerAltitudeStatus::GAINING;
  }

  vertVel = 0.0f;
  altChangeTimer = noAltChangeDuration;
}

void Player::accelerate() {
  if (accelerationTimer > 0.0f) {
    return;
  }

  moveAccel = PLAYER_MOVE_ACCELERATION;
  strafeAccel = PLAYER_STRAFE_ACCELERATION;
  accelerationTimer = accelerationDuration;
}

void Player::decelerate() {
  if (accelerationTimer > 0.0f) {
    return;
  }

  moveAccel = PLAYER_MOVE_DECELERATION;
  strafeAccel = PLAYER_STRAFE_DECELERATION;
  accelerationTimer = accelerationDuration;
}

void Player::draw(const glm::mat4 &view,
                  const glm::mat4 &projection) const {
  object->draw(view, projection);
}

void Player::updateAltitude(float deltaTime) {
  float currAltAccel = getAltitudeAcceleration(
      y,
      altitudeAccel +
          (moveVel * PLAYER_ALTITUDE_ACCEL_FACTOR_WITH_MOVE_VELOCITY),
      highAltY);

  switch (altitudeStatus) {
  case PlayerAltitudeStatus::GAINING: {
    if (y >= highAltY) {
      altitudeStatus = PlayerAltitudeStatus::ON_HIGH;
    } else {
      vertVel += currAltAccel * deltaTime;
    }
    break;
  }

  case PlayerAltitudeStatus::LOSING: {
    if (y <= lowAltY) {
      altitudeStatus = PlayerAltitudeStatus::ON_LOW;
    } else {
      vertVel -= currAltAccel * deltaTime;
    }
    break;
  }

  case PlayerAltitudeStatus::ON_HIGH: {
    if (y > highAltY) {
      vertVel = 0.0f;
    }
    break;
  }

  case PlayerAltitudeStatus::ON_LOW: {
    if (y < lowAltY) {
      vertVel = 0.0f;
    }
    break;
  }
  }

  y += vertVel * deltaTime;

  if (y < lowAltY) {
    y = lowAltY;
  }
  if (y > highAltY) {
    y = highAltY;
  }

  if (altChangeTimer > 0.0f) {
    altChangeTimer = std::max(altChangeTimer - deltaTime, 0.0f);
  }
}

void Player::updateRotationZ(float deltaTime) {
  float diffX = x - prevX;

  if (diffX < 0.0f) {
    rotationAngleZ =
        std::min(rotationAngleZ + (rotationSpeed * deltaTime),
                 maxRotationAngleZ);
  } else if (diffX > 0.0f) {
    rotationAngleZ =
        std::max(rotationAngleZ - (rotationSpeed * deltaTime),
                 -maxRotationAngleZ);
  } else {
    if (rotationAngleZ > 0.0f) {
      rotationAngleZ = std::max(
          rotationAngleZ - (rotationSpeed * deltaTime), 0.0f);
    } else {
      rotationAngleZ = std::min(
          rotationAngleZ + (rotationSpeed * deltaTime), 0.0f);
    }
  }

  object->rotationZ = rotationAngleZ;
}

void Player::updateRotationX(float deltaTime) {
  float diffY = y - prevY;

  if (diffY > 0.0f || moveAccel < 0.0f || strafeAccel < 0.0f) {
    rotationAngleX =
        std::min(rotationAngleX + (rotationSpeed * deltaTime),
                 maxRotationAngleX);
  } else if (diffY < 0.0f || moveAccel > 0.0f || strafeAccel > 0.0f) {
    rotationAngleX =
        std::max(rotationAngleX - (rotationSpeed * deltaTime),
                 -maxRotationAngleX);
  } else {
    if (rotationAngleX > 0.0f) {
      rotationAngleX = std::max(
          rotationAngleX - (rotationSpeed * deltaTime), 0.0f);
    } else {
      rotationAngleX = std::min(
          rotationAngleX + (rotationSpeed * deltaTime), 0.0f);
    }
  }

  object->rotationX = rotationAngleX;
}

void Player::updateVelocities(float deltaTime) {
  if (accelerationTimer <= 0.0f) {
    moveAccel = 0.0f;
    strafeAccel = 0.0f;
  }

  moveVel = std::clamp(moveVel + (moveAccel * deltaTime),
                       PLAYER_MIN_STRAFE_VELOCITY,
                       PLAYER_MAX_MOVE_VELOCITY);
  strafeVel = std::clamp(strafeVel + (strafeAccel * deltaTime),
                         PLAYER_MIN_STRAFE_VELOCITY,
                         PLAYER_MAX_STRAFE_VELOCITY);

  if (accelerationTimer > 0.0f) {
    accelerationTimer = std::max(accelerationTimer - deltaTime, 0.0f);
  }
}

void Player::update(float deltaTime) {
  updateVelocities(deltaTime);
  updateAltitude(deltaTime);
  updateRotationZ(deltaTime);
  updateRotationX(deltaTime);

  object->pos.z -= moveVel * deltaTime;
  object->pos.x = x;
  object->pos.y = y;

  prevX = x;
  prevY = y;
}
