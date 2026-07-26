#include "obstacle-spawner.hpp"
#include "collider.hpp"
#include "collision.hpp"

#include <glm/ext/vector_float3.hpp>

#include <limits>
#include <random>

ObstacleSpawner::ObstacleSpawner(ObstacleOptions obstacleOptions,
                                 ObstacleSpawnOptions spawnOptions,
                                 Shader *shader)
    : obstacleOpts(obstacleOptions), spawnOpts(spawnOptions),
      nextSpawnZ(spawnOptions.initNextSpawnZ) {

  for (const auto &path : obstacleObjs) {
    loadObstacle(path);
  }

  maxLaneObsSizeZ = -std::numeric_limits<float>::max();
  maxBigObsSizeZ = -std::numeric_limits<float>::max();

  for (auto &prop : obstacleProps) {
    if (prop->bigScaledSize.z > maxBigObsSizeZ) {
      maxBigObsSizeZ = prop->bigScaledSize.z;
    }

    if (prop->laneScaledSize.z > maxLaneObsSizeZ) {
      maxLaneObsSizeZ = prop->laneScaledSize.z;
    }
  }

  for (const auto type : patternTypes) {
    auto &pool = poolByPattern[type];
    pool.reserve(spawnOpts.poolSizePerPattern);

    for (int i = 0; i < spawnOpts.poolSizePerPattern; ++i) {
      pool.push_back(
          std::make_unique<SpawnPattern>(makePattern(type, shader)));
    }
  }
}

void ObstacleSpawner::loadObstacle(const std::string &path) {
  auto bounds = std::make_unique<MeshBounds>();
  auto loaded = Mesh::loadObj(path, bounds.get());

  glm::vec3 size = bounds->size();

  float laneWidth =
      spawnOpts.laneWidth - obstacleOpts.obstaclePadding;

  float bigWidth =
      (2.0f * spawnOpts.laneWidth) - obstacleOpts.obstaclePadding;

  float laneX = laneWidth / size.x;
  float bigX = bigWidth / size.x;

  glm::vec3 laneScale(laneX, obstacleOpts.obstacleScale,
                      obstacleOpts.obstacleScale);

  glm::vec3 bigScale(bigX,
                     obstacleOpts.obstacleScale *
                         obstacleOpts.bigObstacleScaleMultiplier,
                     obstacleOpts.obstacleScale *
                         obstacleOpts.bigObstacleScaleMultiplier);

  glm::vec3 laneSize = glm::abs(size * laneScale);
  glm::vec3 bigSize = glm::abs(size * bigScale);

  std::vector<std::unique_ptr<Mesh>> meshes;
  std::vector<ObjectPart> parts;

  meshes.reserve(loaded.size());

  for (auto &part : loaded) {
    meshes.push_back(std::make_unique<Mesh>(std::move(part.mesh)));

    Texture *tex = nullptr;

    if (!part.material.diffuseTexPath.empty()) {
      tex = Texture::load(part.material.diffuseTexPath);
    }

    parts.push_back(
        {meshes.back().get(), part.material.diffuseColor, tex});
  }

  obstacleProps.push_back(std::make_unique<ObstacleProp>(
      std::move(meshes), std::move(parts), laneScale, bigScale,
      laneSize, bigSize));
}

SpawnPattern ObstacleSpawner::makePattern(SpawnPatternType type,
                                          Shader *shader) {
  SpawnPattern pattern;

  std::uniform_int_distribution<int> dist(0,
                                          obstacleProps.size() - 1);
  int propIdx = dist(rng);

  auto &prop = *obstacleProps[propIdx];

  float laneHalfHeight = prop.laneScaledSize.y * 0.5;
  float bigHalfHeight = prop.bigScaledSize.y * 0.5;

  switch (type) {

  case SpawnPatternType::SINGLE_GROUND: {
    addEntityToPattern(&pattern, prop, false, shader, laneHalfHeight);
    break;
  }

  case SpawnPatternType::BIG_GROUND: {
    addEntityToPattern(&pattern, prop, true, shader, bigHalfHeight);
    break;
  }

  case SpawnPatternType::BIG_AIR: {
    addEntityToPattern(&pattern, prop, true, shader,
                       spawnOpts.airY - bigHalfHeight);
    break;
  }

  case SpawnPatternType::SINGLE_AIR: {
    addEntityToPattern(&pattern, prop, false, shader,
                       spawnOpts.airY - laneHalfHeight);
    break;
  }

  case SpawnPatternType::WALL_GROUND: {
    int segmentCount = spawnOpts.laneCount / 2 + 1;
    for (int i = 0; i < segmentCount; i++) {
      addEntityToPattern(&pattern, prop, false, shader,
                         laneHalfHeight);
    }

    break;
  }

  case SpawnPatternType::WALL_AIR: {
    int segmentCount = spawnOpts.laneCount / 2 + 1;
    for (int i = 0; i < segmentCount; i++) {
      addEntityToPattern(&pattern, prop, false, shader,
                         spawnOpts.airY - laneHalfHeight);
    }

    break;
  }

  case SpawnPatternType::DOUBLE_WALL: {
    int segmentCount = spawnOpts.laneCount / 2 + 1;
    for (int i = 0; i < segmentCount; i++) {
      addEntityToPattern(&pattern, prop, false, shader,
                         laneHalfHeight);
      addEntityToPattern(&pattern, prop, false, shader,
                         spawnOpts.airY - laneHalfHeight);
    }

    break;
  }
  }

  return pattern;
}

void ObstacleSpawner::addEntityToPattern(SpawnPattern *pattern,
                                         const ObstacleProp &prop,
                                         bool big, Shader *shader,
                                         float yOffset) {

  auto size = big ? prop.bigScaledSize : prop.laneScaledSize;
  auto scale = big ? prop.bigScale : prop.laneScale;

  auto obstacle = std::make_unique<Obstacle>(
      prop.parts, shader,
      std::make_unique<BoxCollider>(
          glm::vec3(0.0f, 0.0f, 0.0f),
          glm::vec3(size.x * 0.5f, size.y * 0.5f, size.z * 0.5f)),
      rng);

  obstacle->getObject().scale = scale;

  pattern->entities.push_back(
      SpawnEntity{std::move(obstacle), 0.0f, yOffset, 0.0f});
}

SpawnPattern *ObstacleSpawner::findInactive(SpawnPatternType type) {
  for (auto &pattern : poolByPattern[type]) {
    if (!pattern->active) {
      return pattern.get();
    }
  }

  return nullptr;
}

SpawnPatternType ObstacleSpawner::pickPatternType() {
  float singleWeight = 1.0f - 0.5f * difficulty;
  float bigWeight = 0.4f + 0.3f * difficulty;
  float wallWeight = 0.2f + 0.4f * difficulty;
  float doubleWallWeight = 0.05f + 0.3f * difficulty;

  float total =
      singleWeight + bigWeight + wallWeight + doubleWallWeight;
  std::uniform_real_distribution<float> dist(0.0f, total);
  float roll = dist(rng);

  if (roll < singleWeight) {
    float airChance = 0.3f + 0.3f * difficulty;
    std::uniform_real_distribution<float> airDist(0.0f, 1.0f);
    return airDist(rng) < airChance ? SpawnPatternType::SINGLE_AIR
                                    : SpawnPatternType::SINGLE_GROUND;
  }

  roll -= singleWeight;
  if (roll < bigWeight) {
    float airChance = 0.2f + 0.3f * difficulty;
    std::uniform_real_distribution<float> airDist(0.0f, 1.0f);
    return airDist(rng) < airChance ? SpawnPatternType::BIG_AIR
                                    : SpawnPatternType::BIG_GROUND;
  }

  roll -= bigWeight;
  if (roll < wallWeight) {
    float airChance = 0.1f + 0.3f * difficulty;
    std::uniform_real_distribution<float> airDist(0.0f, 1.0f);
    return airDist(rng) < airChance ? SpawnPatternType::WALL_AIR
                                    : SpawnPatternType::WALL_GROUND;
  }

  return SpawnPatternType::DOUBLE_WALL;
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
  float distance = despawnDistance;

  switch (type) {
  case SpawnPatternType::SINGLE_GROUND:
  case SpawnPatternType::SINGLE_AIR:
  case SpawnPatternType::WALL_GROUND:
  case SpawnPatternType::WALL_AIR:
  case SpawnPatternType::DOUBLE_WALL: {
    distance += maxLaneObsSizeZ;
  }

  case SpawnPatternType::BIG_GROUND:
  case SpawnPatternType::BIG_AIR: {
    distance += maxBigObsSizeZ;
  }
  }

  return distance;
}

void ObstacleSpawner::spawnAt(float z) {
  SpawnPatternType type = pickPatternType();

  SpawnPattern *pattern = findInactive(type);
  if (!pattern)
    return;

  int segmentCount = spawnOpts.laneCount / 2 + 1;

  switch (type) {
  case SpawnPatternType::SINGLE_GROUND:
  case SpawnPatternType::SINGLE_AIR: {
    std::uniform_int_distribution<int> laneDist(
        0, spawnOpts.laneCount - 1);
    pattern->entities[0].xOffset = laneCenterX(laneDist(rng));
    break;
  }

  case SpawnPatternType::WALL_GROUND:
  case SpawnPatternType::WALL_AIR: {
    int maxStart = std::max(0, spawnOpts.laneCount - segmentCount);
    std::uniform_int_distribution<int> startDist(0, maxStart);
    int startLane = startDist(rng);

    for (int i = 0; i < segmentCount; ++i) {
      pattern->entities[i].xOffset = laneCenterX(startLane + i);
    }
    break;
  }

  case SpawnPatternType::DOUBLE_WALL: {
    int maxStart = std::max(0, spawnOpts.laneCount - segmentCount);
    std::uniform_int_distribution<int> startDist(0, maxStart);
    int startLane = startDist(rng);

    for (int i = 0; i < segmentCount; ++i) {
      pattern->entities[2 * i].xOffset = laneCenterX(startLane + i);
      pattern->entities[(2 * i) + 1].xOffset =
          laneCenterX(startLane + i);
    }

    break;
  }

  case SpawnPatternType::BIG_GROUND:
  case SpawnPatternType::BIG_AIR: {
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

  pattern->active = true;
}

void ObstacleSpawner::render(const glm::mat4 &view,
                             const glm::mat4 &projection) const {
  for (const auto type : patternTypes) {
    for (const auto &pattern : poolByPattern.at(type)) {
      if (!pattern->active)
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
      if (pattern->active) {
        if (pattern->anchorZ > playerZ + getDespawnDistance(type)) {
          pattern->active = false;
          continue;
        }

        for (auto &ent : pattern->entities) {
          ent.obstacle->rotateInPlace(deltaTime);
        }
      }
    }
  }
}

const Obstacle *ObstacleSpawner::checkCollision(
    const Collider &playerCollider) const {
  for (const auto type : patternTypes) {
    for (const auto &pattern : poolByPattern.at(type)) {
      if (!pattern->active)
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
