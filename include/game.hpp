#pragma once

#include "mesh.hpp"
#include "object.hpp"
#include "obstacle-spawner.hpp"
#include "player.hpp"
#include "shader.hpp"

#include <GLFW/glfw3.h>

#include <memory>

#define GAME_INIT_FAILED -1
#define GAME_INIT_SUCCESS 1

#define GAME_LANE_COUNT 3
#define GAME_LANE_WIDTH 2.5f

#define GAME_DIFFICULTY_RAMP_DURATION 240.0f

#define GAME_ROAD_SEGMENT_COUNT 15
#define GAME_ROAD_SEGMENT_LENGTH 30.0f

struct GameOptions {
  const char *title = "Endless Runner";
  unsigned int width = 1920;
  unsigned int height = 1080;
};

class Game {
public:
  Game(GameOptions opts);
  ~Game();

  void run();

private:
  static constexpr int maxHealth = 100;
  static constexpr int damagePerHit = 25;
  static constexpr float invulDuration = 1.5f;

  const unsigned int width, height;
  const char *title;

  const float roadWidth;       // the full width of the road
  const float roadStrafeLimit; // keep player on the road

  glm::vec3 cameraOffset{0.0f, 3.0f, 7.0f};

  float deltaTime = 0.0f;
  float lastFrameTime = 0.0f;

  int score = 0;
  int health = maxHealth;
  float invulTimer = 0.0f;

  GLFWwindow *wnd = nullptr;

  std::unique_ptr<Shader> shader;

  std::unique_ptr<Player> player;

  std::unique_ptr<Mesh> roadSegmentMesh;
  std::vector<std::unique_ptr<Object>> roadSegments;

  std::unique_ptr<Mesh> smallObstacleMesh;

  std::unique_ptr<ObstacleSpawner> obstacleSpawner;

  int initialize();
  void initPlayer();
  void initRoad();
  void initObstacles();
  void initHud();

  void processInput();
  void update();
  void render();
  void renderHud();
  void shutdownHud();

  void recycleRoadSegments();
  void handleCollision();
};
