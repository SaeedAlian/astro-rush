#include "game.hpp"
#include "mesh.hpp"
#include "object.hpp"
#include "shader.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <memory>

Game::Game(GameOptions opts)
    : title(opts.title), width(opts.width), height(opts.height) {

  if (initialize() != GAME_INIT_SUCCESS)
    return;

  const char *vertexShader = "shaders/vertex.glsl";
  const char *fragmentShader = "shaders/fragment.glsl";

  shader = std::make_unique<Shader>(vertexShader, fragmentShader);

  initPlayer();
  initRoad();
}

Game::~Game() {
  if (wnd) {
    glfwTerminate();
  }
}

int Game::initialize() {
  if (!glfwInit()) {
    std::cerr << "[Game] failed to initialize GLFW\n";
    return GAME_INIT_FAILED;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(width, height, title, nullptr, nullptr);

  if (!window) {
    std::cerr << "[Game] failed to create window\n";
    glfwTerminate();
    return GAME_INIT_FAILED;
  }

  wnd = window;

  glfwMakeContextCurrent(wnd);

  if (glewInit() != GLEW_OK) {
    std::cerr << "[Game] failed to initialize GLEW\n";
    return GAME_INIT_FAILED;
  }

  glEnable(GL_DEPTH_TEST);

  return GAME_INIT_SUCCESS;
}

void Game::initPlayer() {
  PlayerOptions opts;

  opts.initMoveVelocity = 20.0f;
  opts.initStrafeVelocity = 15.0f;

  opts.initVerticalAcceleration = 10.0f;
  opts.jumpAccelerationFactor = 5.0f;
  opts.fallAccelerationFactor = 3.0f;

  opts.jumpHeight = 1.5f;
  opts.size = 1.5f;

  player = std::make_unique<Player>(opts, shader.get(), laneLimit);
}

void Game::initRoad() {
  roadSegmentMesh = std::make_unique<Mesh>(
      Mesh::createPlane(roadWidth, roadSegLen));

  for (int i = 0; i < roadSegCnt; ++i) {
    auto segment =
        std::make_unique<Object>(roadSegmentMesh.get(), shader.get());

    segment->pos =
        glm::vec3(0.0f, 0.0f, -static_cast<float>(i) * roadSegLen);
    segment->color = (i % 2 == 0) ? glm::vec3(0.25f, 0.25f, 0.28f)
                                  : glm::vec3(0.3f, 0.3f, 0.33f);

    roadSegments.push_back(std::move(segment));
  }
}

void Game::recycleRoadSegments() {
  float recycleThreshold = player->getObject().pos.z + roadSegLen;

  for (auto &segment : roadSegments) {
    if (segment->pos.z > recycleThreshold) {
      segment->pos.z -= roadSegCnt * roadSegLen;
    }
  }
}

void Game::update() {
  float currentTime = static_cast<float>(glfwGetTime());
  deltaTime = currentTime - lastFrameTime;
  lastFrameTime = currentTime;

  player->update(deltaTime);
  recycleRoadSegments();
}

void Game::render() {
  glClearColor(0.5f, 0.7f, 0.9f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  auto playerPos = player->getObject().pos;

  glm::vec3 cameraPos =
      glm::vec3(0.0f, 0.0f, playerPos.z) + cameraOffset;
  glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, playerPos.z) +
                           glm::vec3(0.0f, 1.0f, -5.0f);
  glm::mat4 view =
      glm::lookAt(cameraPos, cameraTarget, glm::vec3(0, 1, 0));
  glm::mat4 projection = glm::perspective(
      glm::radians(60.0f), static_cast<float>(width) / height, 0.1f,
      300.0f);

  for (auto &segment : roadSegments) {
    segment->draw(view, projection);
  }
  player->draw(view, projection);

  glfwSwapBuffers(wnd);
}

void Game::processInput() {
  if (glfwGetKey(wnd, GLFW_KEY_Q) == GLFW_PRESS ||
      glfwGetKey(wnd, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(wnd, true);
  }

  if (glfwGetKey(wnd, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    player->moveRight(deltaTime);
  }

  if (glfwGetKey(wnd, GLFW_KEY_LEFT) == GLFW_PRESS) {
    player->moveLeft(deltaTime);
  }

  if (glfwGetKey(wnd, GLFW_KEY_SPACE) == GLFW_PRESS) {
    player->jump();
  }
}

void Game::run() {
  if (wnd == nullptr) {
    std::cerr << "[Game] game is not initialized\n";
    return;
  }

  while (!glfwWindowShouldClose(wnd)) {
    processInput();
    update();
    render();
    glfwPollEvents();
  }
}
