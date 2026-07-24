#include "obstacle-spawner.hpp"
#include "collider.hpp"
#include "collision.hpp"

#include <glm/ext/vector_float3.hpp>
#include <random>

ObstacleSpawner::ObstacleSpawner(ObstacleOptions obstacleOptions,
                                 ObstacleSpawnOptions spawnOptions,
                                 Shader *shader)
    : obstacleOpts(obstacleOptions), spawnOpts(spawnOptions),
      nextSpawnZ(spawnOptions.initNextSpawnZ) {

  laneObstacleMesh = std::make_unique<Mesh>(Mesh::createBox(
      spawnOpts.laneWidth - obstacleOpts.obstaclePadding,
      obstacleOpts.obstacleHeight, obstacleOpts.obstacleDepth));

  bigObstacleMesh = std::make_unique<Mesh>(Mesh::createBox(
      (2.0f * spawnOpts.laneWidth) - obstacleOpts.obstaclePadding,
      obstacleOpts.obstacleHeight *
          obstacleOpts.bigObstacleHeightMultiplier,
      obstacleOpts.obstacleDepth *
          obstacleOpts.bigObstacleDepthMultiplier));

  for (const auto type : patternTypes) {
    auto &pool = poolByPattern[type];
    pool.reserve(spawnOpts.poolSizePerPattern);

    for (int i = 0; i < spawnOpts.poolSizePerPattern; ++i) {
      pool.push_back(
          std::make_unique<SpawnPattern>(makePattern(type, shader)));
    }
  }
}

SpawnPattern ObstacleSpawner::makePattern(SpawnPatternType type,
                                          Shader *shader) {
  SpawnPattern pattern;

  const float halfHeight = obstacleOpts.obstacleHeight * 0.5f;
  const float halfDepth = obstacleOpts.obstacleDepth * 0.5f;
  const float laneHalfWidth = spawnOpts.laneWidth * 0.5f;

  const float bigHalfWidth = laneHalfWidth * 2.0f;
  const float bigHalfHeight =
      halfHeight * obstacleOpts.bigObstacleHeightMultiplier;
  const float bigHalfDepth =
      halfDepth * obstacleOpts.bigObstacleDepthMultiplier;

  switch (type) {
  case SpawnPatternType::SINGLE_GROUND:
    addEntityToPattern(&pattern, laneObstacleMesh.get(), shader,
                       laneHalfWidth, halfHeight, halfDepth,
                       halfHeight);
    break;

  case SpawnPatternType::SINGLE_AIR:
    addEntityToPattern(&pattern, laneObstacleMesh.get(), shader,
                       laneHalfWidth, halfHeight, halfDepth,
                       spawnOpts.airY - halfHeight);
    break;

  case SpawnPatternType::WALL: {
    int segmentCount = spawnOpts.laneCount / 2 + 1;
    for (int i = 0; i < segmentCount; ++i) {
      addEntityToPattern(&pattern, laneObstacleMesh.get(), shader,
                         laneHalfWidth, halfHeight, halfDepth,
                         halfHeight);
    }
    break;
  }

  case SpawnPatternType::BIG:
    addEntityToPattern(&pattern, bigObstacleMesh.get(), shader,
                       bigHalfWidth, bigHalfHeight, bigHalfDepth,
                       bigHalfHeight);
    break;
  }

  return pattern;
}

void ObstacleSpawner::addEntityToPattern(SpawnPattern *pattern,
                                         Mesh *mesh, Shader *shader,
                                         float halfWidth,
                                         float halfHeight,
                                         float halfDepth,
                                         float yOffset) {
  auto obstacle = std::make_unique<Obstacle>(
      mesh, shader,
      std::make_unique<BoxCollider>(
          glm::vec3(0.0f, 0.0f, 0.0f),
          glm::vec3(halfWidth, halfHeight, halfDepth)));

  pattern->entities.push_back(
      SpawnEntity{std::move(obstacle), 0.0f, yOffset, 0.0f});
}

SpawnPattern *ObstacleSpawner::findInactive(SpawnPatternType type) {
  for (auto &pattern : poolByPattern[type]) {
    if (!pattern->drawn) {
      return pattern.get();
    }
  }

  return nullptr;
}

SpawnPatternType ObstacleSpawner::pickPatternType() {
  float singleWeight = 1.0f - 0.5f * difficulty; // 1.0 -> 0.5
  float bigWeight = 0.3f + 0.4f * difficulty;    // 0.3 -> 0.7
  float wallWeight = 0.1f + 0.6f * difficulty;   // 0.1 -> 0.7

  float total = singleWeight + bigWeight + wallWeight;
  std::uniform_real_distribution<float> dist(0.0f, total);
  float roll = dist(rng);

  if (roll < singleWeight) {
    float airChance = 0.3f + 0.3f * difficulty; // 0.3 -> 0.6
    std::uniform_real_distribution<float> airDist(0.0f, 1.0f);
    return airDist(rng) < airChance ? SpawnPatternType::SINGLE_AIR
                                    : SpawnPatternType::SINGLE_GROUND;
  }

  roll -= singleWeight;
  if (roll < bigWeight) {
    return SpawnPatternType::BIG;
  }

  return SpawnPatternType::WALL;
}

float ObstacleSpawner::laneCenterX(int laneIndex) const {
  return (static_cast<float>(laneIndex) -
          (spawnOpts.laneCount - 1) * 0.5f) *
         spawnOpts.laneWidth;
}

float ObstacleSpawner::laneBoundaryX(int boundaryIndex) const {
  return laneCenterX(boundaryIndex) + spawnOpts.laneWidth * 0.5f;
}

float ObstacleSpawner::pickSpawnInterval() {
  float minInterval = spawnOpts.minSpawnIntervalEasiest +
                      (spawnOpts.minSpawnIntervalHardest -
                       spawnOpts.minSpawnIntervalEasiest) *
                          difficulty;

  float maxInterval = spawnOpts.maxSpawnIntervalEasiest +
                      (spawnOpts.maxSpawnIntervalHardest -
                       spawnOpts.maxSpawnIntervalEasiest) *
                          difficulty;

  std::uniform_real_distribution<float> dist(minInterval,
                                             maxInterval);
  return std::max(0.01f, dist(rng));
}

float ObstacleSpawner::getDespawnDistance(SpawnPatternType type) {
  const float bigDepth = obstacleOpts.obstacleDepth *
                         obstacleOpts.bigObstacleDepthMultiplier;

  float distance = despawnDistance;

  switch (type) {
  case SpawnPatternType::SINGLE_GROUND:
  case SpawnPatternType::SINGLE_AIR:
  case SpawnPatternType::WALL: {
    distance += obstacleOpts.obstacleDepth;
  }

  case SpawnPatternType::BIG: {
    distance += bigDepth;
  }
  }

  return distance;
}

void ObstacleSpawner::spawnAt(float z) {
  SpawnPatternType type = pickPatternType();

  SpawnPattern *pattern = findInactive(type);
  if (!pattern)
    return;

  switch (type) {
  case SpawnPatternType::SINGLE_GROUND:
  case SpawnPatternType::SINGLE_AIR: {
    std::uniform_int_distribution<int> laneDist(
        0, spawnOpts.laneCount - 1);
    pattern->entities[0].xOffset = laneCenterX(laneDist(rng));
    break;
  }

  case SpawnPatternType::WALL: {
    int segmentCount = static_cast<int>(pattern->entities.size());
    int maxStart = std::max(0, spawnOpts.laneCount - segmentCount);
    std::uniform_int_distribution<int> startDist(0, maxStart);
    int startLane = startDist(rng);

    for (int i = 0; i < segmentCount; ++i) {
      pattern->entities[i].xOffset = laneCenterX(startLane + i);
    }
    break;
  }

  case SpawnPatternType::BIG: {
    std::uniform_int_distribution<int> boundaryDist(
        0, spawnOpts.laneCount - 2);
    pattern->entities[0].xOffset = laneBoundaryX(boundaryDist(rng));
    break;
  }
  }

  pattern->anchorZ = z;

  for (auto &entity : pattern->entities) {
    entity.obstacle->getObject().pos =
        glm::vec3(entity.xOffset, entity.yOffset, z + entity.zOffset);
    entity.obstacle->syncCollider();
  }

  pattern->drawn = true;
}

void ObstacleSpawner::render(const glm::mat4 &view,
                             const glm::mat4 &projection) const {
  for (const auto type : patternTypes) {
    for (const auto &pattern : poolByPattern.at(type)) {
      if (!pattern->drawn)
        continue;

      for (const auto &entity : pattern->entities) {
        entity.obstacle->getObject().draw(view, projection);
      }
    }
  }
}

void ObstacleSpawner::update(float playerZ, float deltaTime) {
  elapsedTime += deltaTime;
  difficulty = std::min(elapsedTime / spawnOpts.rampDuration, 1.0f);

  while (nextSpawnZ > playerZ - spawnOpts.spawnSafeMarginZ) {
    spawnAt(nextSpawnZ);
    nextSpawnZ -= pickSpawnInterval();
  }

  for (const auto type : patternTypes) {

    for (auto &pattern : poolByPattern.at(type)) {
      if (pattern->drawn &&
          pattern->anchorZ > playerZ + getDespawnDistance(type)) {
        pattern->drawn = false;
      }
    }
  }
}

const Obstacle *ObstacleSpawner::checkCollision(
    const Collider &playerCollider) const {
  for (const auto type : patternTypes) {
    for (const auto &pattern : poolByPattern.at(type)) {
      if (!pattern->drawn)
        continue;

      for (const auto &entity : pattern->entities) {
        if (collides(entity.obstacle->getCollider(),
                     playerCollider)) {
          return entity.obstacle.get();
        }
      }
    }
  }

  return nullptr;
}
