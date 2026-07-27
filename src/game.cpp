#include "game.hpp"
#include "mesh.hpp"
#include "object.hpp"
#include "obstacle_spawner.hpp"
#include "shader.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include <iostream>
#include <memory>

void drawHeart(ImDrawList *drawList, ImVec2 topLeft, float size,
               ImU32 color) {
  float r = size * 0.25f;

  ImVec2 leftCenter(topLeft.x + r, topLeft.y + r);
  ImVec2 rightCenter(topLeft.x + size - r, topLeft.y + r);

  drawList->AddCircleFilled(leftCenter, r, color, 16);
  drawList->AddCircleFilled(rightCenter, r, color, 16);

  ImVec2 p1(topLeft.x, topLeft.y + r * 0.9f);
  ImVec2 p2(topLeft.x + size, topLeft.y + r * 0.9f);
  ImVec2 p3(topLeft.x + size * 0.5f, topLeft.y + size);

  drawList->AddTriangleFilled(p1, p2, p3, color);
}

Game::Game(GameOptions opts)
    : title(opts.title), width(opts.width), height(opts.height),
      roadWidth(GAME_LANE_COUNT * GAME_LANE_WIDTH),
      roadStrafeLimit((GAME_LANE_WIDTH * GAME_LANE_COUNT * 0.5f)) {

  if (initialize() != GAME_INIT_SUCCESS)
    return;

  const char *vertexShader = "shaders/vertex.glsl";
  const char *fragmentShader = "shaders/fragment.glsl";

  shader = std::make_unique<Shader>(vertexShader, fragmentShader);

  initPlayer();
  initRoad();
  initObstacles();
  initHud();
}

Game::~Game() {
  if (wnd) {
    shutdownHud();
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
  player = std::make_unique<Player>(shader.get(), roadStrafeLimit);
}

void Game::initRoad() {
  roadSegmentMesh = std::make_unique<Mesh>(
      Mesh::createPlane(roadWidth, GAME_ROAD_SEGMENT_LENGTH));

  for (int i = 0; i < GAME_ROAD_SEGMENT_COUNT; ++i) {
    auto color = (i % 2 == 0) ? glm::vec3(0.25f, 0.25f, 0.28f)
                              : glm::vec3(0.3f, 0.3f, 0.33f);

    auto segment = std::make_unique<Object>(
        roadSegmentMesh.get(), color, nullptr, shader.get());

    segment->pos =
        glm::vec3(0.0f, 0.0f,
                  -static_cast<float>(i) * GAME_ROAD_SEGMENT_LENGTH);

    roadSegments.push_back(std::move(segment));
  }
}

void Game::initObstacles() {
  obstacleSpawner = std::make_unique<ObstacleSpawner>(
      shader.get(), GAME_DIFFICULTY_RAMP_DURATION, GAME_LANE_COUNT,
      GAME_LANE_WIDTH);
}

void Game::initHud() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGui::GetIO().FontGlobalScale = 1.4f;

  ImGui_ImplGlfw_InitForOpenGL(wnd, true);
  ImGui_ImplOpenGL3_Init("#version 330");
}

void Game::shutdownHud() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void Game::recycleRoadSegments() {
  float recycleThreshold =
      player->getObject().pos.z + GAME_ROAD_SEGMENT_LENGTH;

  for (auto &segment : roadSegments) {
    if (segment->pos.z > recycleThreshold) {
      segment->pos.z -=
          GAME_ROAD_SEGMENT_COUNT * GAME_ROAD_SEGMENT_LENGTH;
    }
  }
}

void Game::handleCollision() {
  if (invulTimer > 0.0f)
    return;

  const Obstacle *hit =
      obstacleSpawner->checkCollision(player->getCollider());

  if (hit) {
    health -= damagePerHit;

    if (health < 0)
      health = 0;

    invulTimer = invulDuration;

    if (health == 0) {
      // gameover
      std::cerr << "[Game] player died\n";
    }
  }
}

void Game::update() {
  constexpr float scoreFactor = 0.1f;

  float currentTime = static_cast<float>(glfwGetTime());
  deltaTime = currentTime - lastFrameTime;
  lastFrameTime = currentTime;

  player->update(deltaTime);
  obstacleSpawner->update(player->getObject().pos.z, deltaTime);
  recycleRoadSegments();

  if (invulTimer > 0.0f) {
    invulTimer -= deltaTime;

    if (invulTimer < 0.0f)
      invulTimer = 0.0f;
  }

  handleCollision();

  score = static_cast<int>(-player->getObject().pos.z * scoreFactor);
}

void Game::renderHud() {
  constexpr float heartSize = 20.0f;
  constexpr float heartSpacing = 4.0f;
  constexpr int totalHearts = maxHealth / damagePerHit;

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGuiWindowFlags flags =
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoBackground |
      ImGuiWindowFlags_NoSavedSettings |
      ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

  ImGui::SetNextWindowPos(ImVec2(16.0f, 16.0f));
  ImGui::Begin("HUD", nullptr, flags);

  ImGui::TextColored(ImVec4(0.0f, 0.35f, 0.3f, 1.0f), "Score: %d",
                     score);

  ImGui::Spacing();

  int filledHearts = health / damagePerHit;

  ImDrawList *drawList = ImGui::GetWindowDrawList();
  ImVec2 cursor = ImGui::GetCursorScreenPos();

  for (int i = 0; i < totalHearts; ++i) {
    ImVec2 topLeft(cursor.x + i * (heartSize + heartSpacing),
                   cursor.y);
    ImU32 color = (i < filledHearts) ? IM_COL32(220, 40, 60, 255)
                                     : IM_COL32(90, 90, 90, 150);
    drawHeart(drawList, topLeft, heartSize, color);
  }

  ImGui::Dummy(
      ImVec2(totalHearts * (heartSize + heartSpacing), heartSize));

  ImGui::Spacing();

  ImGui::End();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Game::render() {
  glClearColor(0.5f, 0.7f, 0.9f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  auto playerPos = player->getObject().pos;

  glm::vec3 cameraPos =
      glm::vec3(playerPos.x, 0.0f, playerPos.z) + cameraOffset;
  glm::vec3 cameraTarget = glm::vec3(playerPos.x, 0.0f, playerPos.z) +
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
  obstacleSpawner->render(view, projection);

  renderHud();

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

  if (glfwGetKey(wnd, GLFW_KEY_UP) == GLFW_PRESS) {
    player->accelerate();
  }

  if (glfwGetKey(wnd, GLFW_KEY_DOWN) == GLFW_PRESS) {
    player->decelerate();
  }

  if (glfwGetKey(wnd, GLFW_KEY_SPACE) == GLFW_PRESS) {
    player->changeAltitude();
  }
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
