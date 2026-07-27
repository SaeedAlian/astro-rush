#pragma once

#include "mesh.hpp"
#include "obstacle_spawner.hpp"
#include "player.hpp"
#include "shader.hpp"

#include <GLFW/glfw3.h>

#include <memory>

#define GAME_INIT_FAILED -1
#define GAME_INIT_SUCCESS 1

#define GAME_LANE_COUNT 3
#define GAME_LANE_WIDTH 2.5f

#define GAME_DIFFICULTY_RAMP_DURATION 240.0f

#define GAME_SKYBOX_INIT_ROTATION_SPEED 0.5f
#define GAME_SKYBOX_MAX_ROTATION_SPEED 1.0f
#define GAME_SKYBOX_MIN_ROTATION_SPEED 0.2f
#define GAME_SKYBOX_ROTATION_SPEED_PLAYER_MOVE_ACCELERATION_MULTIPLIER \
  0.0005f

#define GAME_SKYBOX_MODEL_RIGHT_FACE_PATH "models/skybox/right.png"
#define GAME_SKYBOX_MODEL_LEFT_FACE_PATH "models/skybox/left.png"
#define GAME_SKYBOX_MODEL_UP_FACE_PATH "models/skybox/up.png"
#define GAME_SKYBOX_MODEL_DOWN_FACE_PATH "models/skybox/down.png"
#define GAME_SKYBOX_MODEL_FRONT_FACE_PATH "models/skybox/front.png"
#define GAME_SKYBOX_MODEL_BACK_FACE_PATH "models/skybox/back.png"

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

  static constexpr float minSkyboxRotationSpeed =
      glm::radians(GAME_SKYBOX_MIN_ROTATION_SPEED);
  static constexpr float maxSkyboxRotationSpeed =
      glm::radians(GAME_SKYBOX_MAX_ROTATION_SPEED);

  const unsigned int width, height;
  const char *title;

  const float roadWidth;       // the full width of the road
  const float roadStrafeLimit; // keep player on the road

  glm::vec3 cameraOffset{0.0f, 2.0f, 7.0f};

  float deltaTime = 0.0f;
  float lastFrameTime = 0.0f;

  int score = 0;
  int health = maxHealth;
  float invulTimer = 0.0f;

  float skyboxRotation = 0.0f;
  float skyboxRotationSpeed =
      glm::radians(GAME_SKYBOX_INIT_ROTATION_SPEED);

  GLFWwindow *wnd = nullptr;

  std::unique_ptr<Shader> shader;
  std::unique_ptr<Player> player;
  std::unique_ptr<ObstacleSpawner> obstacleSpawner;
  std::unique_ptr<Shader> skyboxShader;
  std::unique_ptr<Mesh> skyboxMesh;
  Texture *skyboxTexture = nullptr;

  int initialize();
  void initPlayer();
  void initObstacles();
  void initHud();
  void initSkybox();

  void processInput();
  void update();
  void render();
  void renderHud();
  void renderSkybox(const glm::mat4 &view,
                    const glm::mat4 &projection);
  void shutdownHud();

  void handleCollision();
};
