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

#define GAME_PLAYER_SIZE 1.5f
#define GAME_PLAYER_INIT_MOVE_VELOCITY 20.0f
#define GAME_PLAYER_INIT_STRAFE_VELOCITY 15.0f
#define GAME_PLAYER_INIT_VERTICAL_ACCEL 10.0f
#define GAME_PLAYER_JUMP_ACCEL_FACTOR 5.0f
#define GAME_PLAYER_FALL_ACCEL_FACTOR 4.0f
#define GAME_PLAYER_JUMP_HEIGHT 1.5f

#define GAME_OBSTACLE_HEIGHT 1.5f
#define GAME_OBSTACLE_DEPTH 1.5f
#define GAME_OBSTACLE_PADDING 0.4f
#define GAME_SPAWN_SAFE_MARGIN_Z 320.0f
#define GAME_OBSTACLE_BIG_HEIGHT_MULTIPLIER 2.5f
#define GAME_OBSTACLE_BIG_DEPTH_MULTIPLIER 2.5f
#define GAME_OBSTACLE_INIT_NEXT_SPAWN_Z -50.0f
#define GAME_OBSTACLE_MIN_SPAWN_INTERVAL_EASY 30.0f
#define GAME_OBSTACLE_MAX_SPAWN_INTERVAL_EASY 70.0f
#define GAME_OBSTACLE_MIN_SPAWN_INTERVAL_HARD 10.0f
#define GAME_OBSTACLE_MAX_SPAWN_INTERVAL_HARD 25.0f
#define GAME_OBSTACLE_AIR_Y 5.0f
#define GAME_OBSTACLE_POOL_SIZE 10

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
  const unsigned int width, height;
  const char *title;

  const float roadWidth;       // the full width of the road
  const float roadStrafeLimit; // keep player on the road

  glm::vec3 cameraOffset{0.0f, 3.0f, 7.0f};

  float deltaTime = 0.0f;
  float lastFrameTime = 0.0f;

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

  void processInput();
  void update();
  void render();

  void recycleRoadSegments();
  void handleCollision();
};
