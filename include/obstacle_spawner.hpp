#pragma once

#include "collider.hpp"
#include "mesh.hpp"
#include "obstacle.hpp"
#include "shader.hpp"

#include <memory>
#include <random>
#include <unordered_map>
#include <vector>

#define OBSTACLE_SCALE 1.5f
#define OBSTACLE_PADDING 0.4f
#define OBSTACLE_BIG_SCALE_MULTIPLIER 3.0f

#define OBSTACLE_MIN_SPAWN_INTERVAL_EASY 30.0f
#define OBSTACLE_MAX_SPAWN_INTERVAL_EASY 70.0f
#define OBSTACLE_MIN_SPAWN_INTERVAL_HARD 10.0f
#define OBSTACLE_MAX_SPAWN_INTERVAL_HARD 25.0f

#define OBSTACLE_SPAWN_SAFE_MARGIN_Z 320.0f
#define OBSTACLE_INIT_NEXT_SPAWN_Z -50.0f
#define OBSTACLE_HIGH_ALTITUDE_Y 5.0f

#define OBSTACLE_POOL_SIZE 10

/*
 * Represents a single obstacle instance in a spawn pattern.
 *
 * Stores the obstacle object and its position offsets relative to the
 * pattern anchor position.
 */
struct SpawnEntity {
  std::unique_ptr<Obstacle> obstacle;
  float xOffset = 0.0f;
  float yOffset = 0.0f;
  float zOffset = 0.0f;
};

/*
 * Represents a predefined obstacle spawn formation.
 *
 * A pattern can contain multiple obstacle entities. Patterns are
 * pooled and reused during gameplay.
 */
struct SpawnPattern {
  std::vector<SpawnEntity> entities;

  /*
   * Indicates whether this pattern is currently active in the world.
   */
  bool active = false;

  /*
   * Z coordinate where this pattern was spawned.
   *
   * Used to determine when the pattern should be removed.
   */
  float anchorZ = 0.0f;
};

/*
 * Stores the properties required to create obstacles of a specific
 * type.
 *
 * Contains the loaded meshes, materials, scaling values and
 * calculated sizes for normal lane obstacles and big obstacles.
 */
struct ObstacleProp {
  std::vector<std::unique_ptr<Mesh>> meshes;
  std::vector<ObjectPart> parts;

  glm::vec3 laneScale;
  glm::vec3 bigScale;

  glm::vec3 laneScaledSize;
  glm::vec3 bigScaledSize;

  /*
   * Initializes an obstacle property object.
   *
   * @param meshes | loaded meshes of the obstacle
   * @param parts | mesh parts with materials
   * @param laneScale | scale used for lane obstacles
   * @param bigScale | scale used for big obstacles
   * @param laneScaledSize | final size of lane obstacles
   * @param bigScaledSize | final size of big obstacles
   */
  ObstacleProp(std::vector<std::unique_ptr<Mesh>> meshes,
               std::vector<ObjectPart> parts, glm::vec3 laneScale,
               glm::vec3 bigScale, glm::vec3 laneScaledSize,
               glm::vec3 bigScaledSize)
      : meshes(std::move(meshes)), parts(std::move(parts)),
        laneScale(laneScale), bigScale(bigScale),
        laneScaledSize(laneScaledSize), bigScaledSize(bigScaledSize) {
  }
};

/*
 * Types of obstacle formations that can be spawned.
 */
enum class SpawnPatternType {
  SINGLE_LOW_ALT,
  SINGLE_HIGH_ALT,
  WALL_LOW_ALT,
  WALL_HIGH_ALT,
  BIG_LOW_ALT,
  BIG_HIGH_ALT,
  DOUBLE_WALL,
};

/*
 * Handles loading, spawning, updating and rendering obstacles.
 *
 * The spawner manages obstacle patterns using object pools and
 * creates obstacles ahead of the player based on the current
 * difficulty level.
 *
 * It also performs collision checks between the player collider and
 * active obstacles.
 */
class ObstacleSpawner {
public:
  /*
   * Initializes the obstacle spawner.
   *
   * Loads obstacle models, creates obstacle properties and
   * initializes the spawn pattern pools.
   *
   * @param shader | shader used for rendering obstacles
   * @param rampDuration | duration of difficulty increase
   * @param laneCount | number of available lanes
   * @param laneWidth | width of each lane
   */
  ObstacleSpawner(Shader *shader, float rampDuration, int laneCount,
                  float laneWidth);

  /*
   * Updates the obstacle spawner state.
   *
   * Updates difficulty, spawns new patterns when needed, removes
   * patterns that passed the player and updates obstacle rotations.
   *
   * @param playerZ | current player z coordinate
   * @param deltaTime | elapsed time since previous update
   */
  void update(float playerZ, float deltaTime);

  /*
   * Renders all active obstacle patterns.
   *
   * @param view | view matrix (transforms world coordinates to camera
   * coordinates)
   * @param projection | projection matrix (transforms camera
   * coordinates to screen coordinates)
   */
  void render(const glm::mat4 &view,
              const glm::mat4 &projection) const;

  /*
   * Checks collision between the player and active obstacles.
   *
   * @param playerCollider | player collision shape
   *
   * @return collided obstacle, or nullptr if there is no collision
   */
  const Obstacle *
  checkCollision(const Collider &playerCollider) const;

private:
  static constexpr float despawnDistance = 4.0f;
  static constexpr int maxSegmentCount = 5;
  static constexpr std::array patternTypes = {
      SpawnPatternType::SINGLE_LOW_ALT,
      SpawnPatternType::SINGLE_HIGH_ALT,
      SpawnPatternType::WALL_LOW_ALT,
      SpawnPatternType::WALL_HIGH_ALT,
      SpawnPatternType::BIG_LOW_ALT,
      SpawnPatternType::BIG_HIGH_ALT,
      SpawnPatternType::DOUBLE_WALL};

  /*
   * Obstacle object models used to render obstacles.
   */
  static constexpr std::array obstacleObjs = {
      "models/asteroid1/asteroid.obj",
      "models/asteroid2/asteroid.obj",
      "models/asteroid3/asteroid.obj",
  };

  const int laneCount;
  const float laneWidth, rampDuration;
  const int segmentCount;

  const int poolSizePerPattern = OBSTACLE_POOL_SIZE;

  const float obsScale = OBSTACLE_SCALE;
  const float bigObsScaleMult = OBSTACLE_BIG_SCALE_MULTIPLIER;
  const float obsPadding = OBSTACLE_PADDING;

  const float highAltY = OBSTACLE_HIGH_ALTITUDE_Y;

  const float minSpawnIntervalEasiest =
      OBSTACLE_MIN_SPAWN_INTERVAL_EASY;
  const float minSpawnIntervalHardest =
      OBSTACLE_MIN_SPAWN_INTERVAL_HARD;

  const float maxSpawnIntervalEasiest =
      OBSTACLE_MAX_SPAWN_INTERVAL_EASY;
  const float maxSpawnIntervalHardest =
      OBSTACLE_MAX_SPAWN_INTERVAL_HARD;

  const float spawnSafeMarginZ = OBSTACLE_SPAWN_SAFE_MARGIN_Z;

  float nextSpawnZ = 30.0f;

  float elapsedTime = 0.0f;
  float difficulty = 0.0f; // 0 (easiest), 1 (hardest)

  float maxLaneObsSizeZ, maxBigObsSizeZ;

  std::mt19937 rng{std::random_device{}()};

  std::unordered_map<SpawnPatternType,
                     std::vector<std::unique_ptr<SpawnPattern>>>
      poolByPattern;

  std::vector<std::unique_ptr<ObstacleProp>> obstacleProps;

  /*
   * Loads obstacle model from the object file path, and creates its
   * ObstacleProp and adds it to obstacleProps.
   *
   * @param path | obstacle .obj file path
   */
  void loadObstacle(const std::string &path);

  /*
   * Creates the obstacle entities of a spawn pattern based on its
   * type.
   *
   * @param type | spawn pattern type
   * @param shader | shader used for rendering
   *
   * @return the created SpawnPattern
   */
  SpawnPattern makePattern(SpawnPatternType type, Shader *shader);

  /*
   * Returns an inactive spawn pattern from the pool.
   *
   * @param type | spawn pattern type
   *
   * @return available spawn pattern, or nullptr if pool is exhausted
   */
  SpawnPattern *findInactive(SpawnPatternType type);

  /*
   * Selects a spawn pattern type based on the current difficulty.
   *
   * Higher difficulty increases the chance of larger and more complex
   * patterns.
   *
   * @return selected spawn pattern type
   */
  SpawnPatternType pickPatternType();

  /*
   * Calculates the center x coordinate of a lane.
   *
   * @param laneIndex
   *
   * @return lane center x coordinate
   */
  float laneCenterX(int laneIndex) const;

  /*
   * Calculates the x coordinate of a lane boundary (between two
   * lanes) (boundaryIndex and boundaryIndex + 1).
   *
   * @param boundaryIndex
   *
   * @return lane boundary x coordinate
   */
  float laneBoundaryX(int boundaryIndex) const;

  /*
   * Spawns a pattern at the given z coordinate.
   *
   * Selects a pattern, assigns lane positions and activates it.
   *
   * @param z
   */
  void spawnAt(float z);

  /*
   * Calculates the next spawn interval (z coord) based on difficulty.
   *
   * @return interval until next spawn
   */
  float pickSpawnInterval();

  /*
   * Calculates the distance required before a pattern can be removed.
   *
   * The distance depends on the obstacle size contained in the
   * pattern.
   *
   * @param type | spawn pattern type
   *
   * @return despawn distance
   */
  float getDespawnDistance(SpawnPatternType type);

  /*
   * @return randomly selected obstacle properties
   */
  ObstacleProp &randomObstacleProp();

  /*
   * Creates obstacle entities inside a spawn pattern.
   *
   * @param pattern | target spawn pattern
   * @param shader | shader used for rendering
   */
  void createObstaclesForPattern(SpawnPattern &pattern,
                                 Shader *shader);

  /*
   * Adds an obstacle entity to a spawn pattern.
   *
   * Creates an obstacle using the given properties and applies the
   * correct scale depending on whether it is a big obstacle.
   *
   * @param pattern | target spawn pattern
   * @param prop | obstacle properties
   * @param big | whether the obstacle should use big scale
   * @param shader | shader used for rendering
   * @param yOffset | vertical offset of the obstacle
   */
  void addEntityToPattern(SpawnPattern *pattern,
                          const ObstacleProp &prop, bool big,
                          Shader *shader, float yOffset);
};
