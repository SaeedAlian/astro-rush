#include "player.hpp"

#include <vector>

Player::Player(const PlayerOptions options, Shader *shader,
               const float strafeLimit)
    : opts(options), groundY(options.size * 0.5f),
      strafeLimit(strafeLimit), moveVel(options.initMoveVelocity),
      vertVel(0.0f), jumpAccel(options.jumpAccelerationFactor *
                               options.initVerticalAcceleration),
      fallAccel(options.fallAccelerationFactor *
                options.initVerticalAcceleration),
      strafeVel(options.initStrafeVelocity) {

  x = 0.0f;
  y = groundY;

  auto loadedParts = Mesh::loadObj(PLAYER_OBJ_PATH);

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

  object = std::make_unique<Object>(std::move(objParts), shader);
  object->pos = glm::vec3(0.0f, y, 0.0f);
  object->scale = glm::vec3(PLAYER_SCALE);
}

BoxCollider Player::getCollider() const {
  float half = opts.size * 0.5f;
  glm::vec3 halfExtents(half, half, half);
  glm::vec3 center = object->pos;

  return BoxCollider(center, halfExtents);
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

void Player::jump() {
  if (status == PlayerStatus::ON_GROUND) {
    status = PlayerStatus::JUMPING;
    vertVel = 0.0f;
  }
}

void Player::draw(const glm::mat4 &view,
                  const glm::mat4 &projection) const {
  object->draw(view, projection);
}

void Player::update(float deltaTime) {
  float maxJumpHeight = opts.jumpHeight + groundY;

  switch (status) {
  case PlayerStatus::JUMPING: {
    if (y > maxJumpHeight) {
      status = PlayerStatus::FALLING;
    } else {
      vertVel += jumpAccel * deltaTime;
    }
    break;
  }

  case PlayerStatus::FALLING: {
    if (object->pos.y <= groundY) {
      status = PlayerStatus::ON_GROUND;
    } else {
      vertVel -= fallAccel * deltaTime;
    }
    break;
  }

  case PlayerStatus::ON_GROUND: {
    if (y < groundY) {
      vertVel = 0.0f;
    }
    break;
  }
  }

  y += vertVel * deltaTime;
  if (y < groundY) {
    y = groundY;
  }

  object->pos.z -= moveVel * deltaTime;
  object->pos.x = x;
  object->pos.y = y;
}
