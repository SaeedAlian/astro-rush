#pragma once

#include "collider.hpp"
#include "mesh.hpp"
#include "obstacle.hpp"
#include "shader.hpp"

#include <memory>
#include <random>
#include <unordered_map>
#include <vector>

struct SpawnEntity {
  std::unique_ptr<Obstacle> obstacle;
  float xOffset = 0.0f;
  float yOffset = 0.0f;
  float zOffset = 0.0f;
};

struct SpawnPattern {
  std::vector<SpawnEntity> entities;
  bool active = false;
  float anchorZ = 0.0f;
};

struct ObstacleProp {
  std::vector<std::unique_ptr<Mesh>> meshes;
  std::vector<ObjectPart> parts;

  glm::vec3 laneScale;
  glm::vec3 bigScale;

  glm::vec3 laneScaledSize;
  glm::vec3 bigScaledSize;

  ObstacleProp(std::vector<std::unique_ptr<Mesh>> meshes,
               std::vector<ObjectPart> parts, glm::vec3 laneScale,
               glm::vec3 bigScale, glm::vec3 laneScaledSize,
               glm::vec3 bigScaledSize)
      : meshes(std::move(meshes)), parts(std::move(parts)),
        laneScale(laneScale), bigScale(bigScale),
        laneScaledSize(laneScaledSize), bigScaledSize(bigScaledSize) {
  }
};

enum class SpawnPatternType {
  SINGLE_GROUND,
  SINGLE_AIR,
  WALL_GROUND,
  WALL_AIR,
  BIG_GROUND,
  BIG_AIR,
  DOUBLE_WALL,
};

struct ObstacleOptions {
  float obstacleScale;
  float obstaclePadding;
  float bigObstacleScaleMultiplier;
};

struct ObstacleSpawnOptions {
  float airY;
  float rampDuration;

  float initNextSpawnZ;
  float spawnSafeMarginZ;

  float minSpawnIntervalEasiest;
  float maxSpawnIntervalEasiest;
  float minSpawnIntervalHardest;
  float maxSpawnIntervalHardest;

  int laneCount;
  float laneWidth;

  int poolSizePerPattern = 8;
};

class ObstacleSpawner {
public:
  ObstacleSpawner(ObstacleOptions obstacleOptions,
                  ObstacleSpawnOptions spawnOptions, Shader *shader);

  void update(float playerZ, float deltaTime);

  void render(const glm::mat4 &view,
              const glm::mat4 &projection) const;

  const Obstacle *
  checkCollision(const Collider &playerCollider) const;

private:
  static constexpr float despawnDistance = 4.0f;
  static constexpr std::array patternTypes = {
      SpawnPatternType::SINGLE_GROUND, SpawnPatternType::SINGLE_AIR,
      SpawnPatternType::WALL_GROUND,   SpawnPatternType::WALL_AIR,
      SpawnPatternType::BIG_GROUND,    SpawnPatternType::BIG_AIR,
      SpawnPatternType::DOUBLE_WALL};

  static constexpr std::array obstacleObjs = {
      "models/asteroid1/asteroid.obj",
      "models/asteroid2/asteroid.obj",
      "models/asteroid3/asteroid.obj",
  };

  const ObstacleOptions obstacleOpts;
  const ObstacleSpawnOptions spawnOpts;

  float nextSpawnZ = 30.0f;

  float elapsedTime = 0.0f;
  float difficulty = 0.0f; // 0 (easiest), 1 (hardest)

  float maxLaneObsSizeZ, maxBigObsSizeZ;

  std::mt19937 rng{std::random_device{}()};

  std::unordered_map<SpawnPatternType,
                     std::vector<std::unique_ptr<SpawnPattern>>>
      poolByPattern;

  std::vector<std::unique_ptr<ObstacleProp>> obstacleProps;

  void loadObstacle(const std::string &path);
  SpawnPattern makePattern(SpawnPatternType type, Shader *shader);

  SpawnPattern *findInactive(SpawnPatternType type);

  SpawnPatternType pickPatternType();

  float laneCenterX(int laneIndex) const;
  float laneBoundaryX(int boundaryIndex) const;
  void spawnAt(float z);

  float pickSpawnInterval();

  float getDespawnDistance(SpawnPatternType type);

  ObstacleProp &randomObstacleProp();
  void createObstaclesForPattern(SpawnPattern &pattern,
                                 Shader *shader);
  void addEntityToPattern(SpawnPattern *pattern,
                          const ObstacleProp &prop, bool big,
                          Shader *shader, float yOffset);
};
