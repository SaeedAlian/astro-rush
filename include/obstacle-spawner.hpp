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
  bool drawn = false;
  float anchorZ = 0.0f;
};

enum class SpawnPatternType {
  SINGLE_GROUND,
  SINGLE_AIR,
  WALL,
  BIG,
};

struct ObstacleOptions {
  float obstacleHeight;
  float obstacleDepth;
  float obstaclePadding;

  float bigObstacleHeightMultiplier;
  float bigObstacleDepthMultiplier;
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
  static constexpr float despawnDistance = 2.0f;
  static constexpr std::array patternTypes = {
      SpawnPatternType::SINGLE_GROUND, SpawnPatternType::SINGLE_AIR,
      SpawnPatternType::WALL, SpawnPatternType::BIG};

  const ObstacleOptions obstacleOpts;
  const ObstacleSpawnOptions spawnOpts;

  float nextSpawnZ = 30.0f;

  float elapsedTime = 0.0f;
  float difficulty = 0.0f; // 0 (easiest), 1 (hardest)

  std::mt19937 rng{std::random_device{}()};

  std::unordered_map<SpawnPatternType,
                     std::vector<std::unique_ptr<SpawnPattern>>>
      poolByPattern;

  std::unique_ptr<Mesh>
      laneObstacleMesh; // SINGLE_GROUND / SINGLE_AIR / WALL
  std::unique_ptr<Mesh> bigObstacleMesh; // BIG

  SpawnPattern makePattern(SpawnPatternType type, Shader *shader);
  SpawnPattern *findInactive(SpawnPatternType type);

  SpawnPatternType pickPatternType();

  float laneCenterX(int laneIndex) const;
  float laneBoundaryX(int boundaryIndex) const;
  void spawnAt(float z);

  float pickSpawnInterval();

  float getDespawnDistance(SpawnPatternType type);

  void addEntityToPattern(SpawnPattern *pattern, Mesh *mesh,
                          Shader *shader, float halfWidth,
                          float halfHeight, float halfDepth,
                          float yOffset);
};
