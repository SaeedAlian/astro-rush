#pragma once

#include "mesh.hpp"
#include "object.hpp"
#include "player.hpp"
#include "shader.hpp"

#include <GLFW/glfw3.h>

#include <memory>

#define GAME_INIT_FAILED -1
#define GAME_INIT_SUCCESS 1

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

  const int roadSegCnt =
      15; // the number of segments of the road in each frame
  const float roadWidth = 10.0f; // the full width of the road
  const float roadSegLen =
      30.0f; // length of each segment of the road
  const float laneLimit =
      (10.0f * 0.5f) - (1.5 * 0.5f); // keep player on the road

  glm::vec3 cameraOffset{0.0f, 6.0f, 7.0f};

  float deltaTime = 0.0f;
  float lastFrameTime = 0.0f;

  GLFWwindow *wnd = nullptr;

  std::unique_ptr<Shader> shader;

  std::unique_ptr<Player> player;

  std::unique_ptr<Mesh> roadSegmentMesh;
  std::vector<std::unique_ptr<Object>> roadSegments;

  std::unique_ptr<Mesh> smallObstacleMesh;

  int initialize();
  void initPlayer();
  void initRoad();

  void processInput();
  void update();
  void render();

  void recycleRoadSegments();
};
