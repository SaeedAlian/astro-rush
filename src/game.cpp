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

#include <algorithm>
#include <iostream>
#include <memory>

namespace {

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

void drawSpeedIcon(ImDrawList *drawList, ImVec2 topLeft, float size,
                   ImU32 color) {
  ImVec2 p1(topLeft.x, topLeft.y);
  ImVec2 p2(topLeft.x + size, topLeft.y + size * 0.5f);
  ImVec2 p3(topLeft.x, topLeft.y + size);

  drawList->AddTriangleFilled(p1, p2, p3, color);
}

void drawStarIcon(ImDrawList *drawList, ImVec2 center, float radius,
                  ImU32 color) {
  constexpr int points = 5;
  ImVec2 verts[points * 2];

  for (int i = 0; i < points * 2; ++i) {
    float angle =
        (glm::pi<float>() / points) * i - glm::half_pi<float>();
    float r = (i % 2 == 0) ? radius : radius * 0.4f;
    verts[i] =
        ImVec2(center.x + cos(angle) * r, center.y + sin(angle) * r);
  }

  for (int i = 0; i < points * 2; ++i) {
    int next = (i + 1) % (points * 2);
    drawList->AddTriangleFilled(center, verts[i], verts[next], color);
  }
}

void centerNextItem(float itemWidth, float regionWidth) {
  float offset = (regionWidth - itemWidth) * 0.5f;
  if (offset > 0.0f) {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
  }
}

} // namespace

Game::Game(GameOptions opts)
    : title(opts.title), refWidth(opts.width), width(opts.width),
      refHeight(opts.height), height(opts.height),
      roadWidth(GAME_LANE_COUNT * GAME_LANE_WIDTH),
      roadStrafeLimit((GAME_LANE_WIDTH * GAME_LANE_COUNT * 0.5f)) {

  if (initialize() != GAME_INIT_SUCCESS)
    return;

  const char *vertexShader = "shaders/vertex.glsl";
  const char *fragmentShader = "shaders/fragment.glsl";

  shader = std::make_unique<Shader>(vertexShader, fragmentShader);

  initHud();

  state = GameState::Loading;
  loadProgress = 0.0f;
  loadStatusText = "Loading...";
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

  // vsync
  glfwSwapInterval(1);

  if (glewInit() != GLEW_OK) {
    std::cerr << "[Game] failed to initialize GLEW\n";
    return GAME_INIT_FAILED;
  }

  glEnable(GL_DEPTH_TEST);

  glfwSetWindowUserPointer(wnd, this);
  glfwSetFramebufferSizeCallback(
      wnd, [](GLFWwindow *w, int newWidth, int newHeight) {
        auto *game = static_cast<Game *>(glfwGetWindowUserPointer(w));
        if (game && newWidth > 0 && newHeight > 0) {
          game->width = static_cast<unsigned int>(newWidth);
          game->height = static_cast<unsigned int>(newHeight);
        }
      });

  return GAME_INIT_SUCCESS;
}

void Game::initPlayer() {
  player = std::make_unique<Player>(shader.get(), roadStrafeLimit);
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

  ImGuiIO &io = ImGui::GetIO();
  io.FontGlobalScale = 1.0f;

  hudFont =
      io.Fonts->AddFontFromFileTTF(GAME_FONT_PATH, baseFontSize);

  if (!hudFont) {
    std::cerr << "[Game] failed to load HUD font, falling back to "
                 "default\n";
    hudFont = io.Fonts->AddFontDefault();
  }

  ImGui_ImplGlfw_InitForOpenGL(wnd, true);
  ImGui_ImplOpenGL3_Init("#version 330");
}

void Game::initSkybox() {
  skyboxShader = std::make_unique<Shader>(
      "shaders/skybox_vertex.glsl", "shaders/skybox_fragment.glsl");

  skyboxMesh = std::make_unique<Mesh>(Mesh::createCube(1.0f));

  std::vector<std::string> faces = {
      GAME_SKYBOX_MODEL_RIGHT_FACE_PATH, // +X
      GAME_SKYBOX_MODEL_LEFT_FACE_PATH,  // -X
      GAME_SKYBOX_MODEL_UP_FACE_PATH,    // +Y
      GAME_SKYBOX_MODEL_DOWN_FACE_PATH,  // -Y
      GAME_SKYBOX_MODEL_FRONT_FACE_PATH, // +Z
      GAME_SKYBOX_MODEL_BACK_FACE_PATH,  // -Z
  };

  skyboxTexture = Texture::loadCubemap(faces);
}

void Game::assetsStepLoading() {
  if (loadProgress <= 0.0f) {
    loadProgress = 0.2f;
    loadStatusText = "Loading...";
    return;
  }

  if (!player) {
    initPlayer();
  }

  if (loadProgress <= 0.2f) {
    loadProgress = 0.5f;
    return;
  }

  if (!obstacleSpawner) {
    initObstacles();
  }

  if (loadProgress <= 0.5f) {
    loadProgress = 0.8f;
    return;
  }

  if (!skyboxTexture) {
    initSkybox();
    loadProgress = 1.0f;
    loadStatusText = "Done";
  }

  state = GameState::MainMenu;
  glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Game::shutdownHud() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
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
      finalScore = score;
      state = GameState::GameOver;
      glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
  }
}

void Game::rotateSkybox(float speed) {
  skyboxRotation += speed * deltaTime;

  if (skyboxRotation > glm::two_pi<float>()) {
    skyboxRotation -= glm::two_pi<float>();
  }
}

void Game::startNewGame() {
  score = 0;
  finalScore = 0;
  health = maxHealth;
  invulTimer = 0.0f;

  player->reset();
  obstacleSpawner->reset();

  lastFrameTime = static_cast<float>(glfwGetTime());

  state = GameState::Playing;
  glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Game::togglePause() {
  if (state == GameState::Playing) {
    state = GameState::Paused;
    glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  } else if (state == GameState::Paused) {
    state = GameState::Playing;
    glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // avoid a large deltaTime spike
    lastFrameTime = static_cast<float>(glfwGetTime());
  }
}

void Game::update() {
  constexpr float scoreFactor = 0.1f;

  float currentTime = static_cast<float>(glfwGetTime());
  deltaTime = currentTime - lastFrameTime;
  lastFrameTime = currentTime;

  if (state == GameState::Loading) {
    assetsStepLoading();
    return;
  }

  if (state != GameState::Playing) {
    rotateSkybox(minSkyboxRotationSpeed);
    return;
  }

  player->update(deltaTime);
  obstacleSpawner->update(player->getObject().pos.z, deltaTime);

  if (invulTimer > 0.0f) {
    invulTimer -= deltaTime;

    if (invulTimer < 0.0f)
      invulTimer = 0.0f;
  }

  handleCollision();

  if (state != GameState::Playing) {
    return;
  }

  score = static_cast<int>(-player->getObject().pos.z * scoreFactor);

  skyboxRotationSpeed = std::clamp(
      skyboxRotationSpeed +
          (player->getMoveAcceleration() *
           GAME_SKYBOX_ROTATION_SPEED_PLAYER_MOVE_ACCELERATION_MULTIPLIER *
           deltaTime),
      minSkyboxRotationSpeed, maxSkyboxRotationSpeed);

  rotateSkybox(skyboxRotationSpeed);
}

void Game::renderSkybox(const glm::mat4 &view,
                        const glm::mat4 &projection) {
  if (!skyboxShader || !skyboxMesh || !skyboxTexture)
    return;

  glDepthFunc(GL_LEQUAL);
  glDepthMask(GL_FALSE);

  glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
  glm::mat4 model = glm::rotate(glm::mat4(1.0f), skyboxRotation,
                                glm::vec3(1.0f, 0.0f, 1.0f));

  skyboxShader->use();
  skyboxShader->setMat4("view", skyboxView);
  skyboxShader->setMat4("projection", projection);
  skyboxShader->setMat4("model", model);

  skyboxTexture->bindCubemap(0);
  skyboxShader->setInt("skybox", 0);

  skyboxMesh->draw();

  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LESS);
}

float Game::hudScale() const {
  float scaleX = static_cast<float>(width) / refWidth;
  float scaleY = static_cast<float>(height) / refHeight;
  float scale = std::min(scaleX, scaleY);
  return std::clamp(scale, 0.5f, 2.5f);
}

void Game::renderHudScore(ImGuiWindowFlags flags, ImVec2 position,
                          ImVec2 pivot) {
  float scale = hudScale();

  ImGui::SetNextWindowPos(position, ImGuiCond_Always, pivot);
  ImGui::SetNextWindowSize(ImVec2(250.0f * scale, 50.0f * scale),
                           ImGuiCond_Always);

  ImGui::Begin("Score HUD", nullptr, flags);

  ImDrawList *drawList = ImGui::GetWindowDrawList();
  ImVec2 cursor = ImGui::GetCursorScreenPos();

  float starRadius = 14.0f * scale;

  ImVec2 starCenter(cursor.x + starRadius, cursor.y + starRadius);
  drawStarIcon(drawList, starCenter, starRadius,
               IM_COL32(255, 220, 80, 255));

  ImGui::Dummy(ImVec2(starRadius * 2 + 6.0f * scale, starRadius * 2));
  ImGui::SameLine();
  ImGui::SetWindowFontScale(scale);
  ImGui::TextColored(ImVec4(0.4f, 0.65f, 0.7f, 1.0f), "Score %d",
                     score);
  ImGui::SetWindowFontScale(1.0f);

  ImGui::End();
}

void Game::renderHudPlayerStats(ImGuiWindowFlags flags,
                                ImVec2 position, ImVec2 pivot) {
  float scale = hudScale();

  ImGui::SetNextWindowPos(position, ImGuiCond_Always, pivot);
  ImGui::Begin("Player Stats HUD", nullptr, flags);

  ImDrawList *drawList = ImGui::GetWindowDrawList();
  ImVec2 cursor = ImGui::GetCursorScreenPos();

  float speedIconSize = 18.0f * scale;
  float heartSize = 30.0f * scale;
  float heartSpacing = 5.0f * scale;
  constexpr int totalHearts = maxHealth / damagePerHit;

  int filledHearts = health / damagePerHit;

  drawSpeedIcon(drawList, cursor, speedIconSize,
                IM_COL32(120, 200, 255, 255));

  ImGui::Dummy(ImVec2(speedIconSize + 6.0f * scale, speedIconSize));
  ImGui::SameLine();
  ImGui::SetWindowFontScale(scale);
  ImGui::TextColored(ImVec4(0.4f, 0.65f, 0.7f, 1.0f), "Speed %d",
                     static_cast<int>(player->getMoveVelocity()));
  ImGui::SetWindowFontScale(1.0f);

  ImGui::Spacing();

  float heartsY = cursor.y + speedIconSize + 10.0f * scale;

  for (int i = 0; i < totalHearts; ++i) {
    ImVec2 topLeft(cursor.x + i * (heartSize + heartSpacing),
                   heartsY);

    ImU32 color = (i < filledHearts) ? IM_COL32(220, 40, 60, 255)
                                     : IM_COL32(90, 90, 90, 150);

    drawHeart(drawList, topLeft, heartSize, color);
  }

  ImGui::Dummy(
      ImVec2(totalHearts * (heartSize + heartSpacing), heartSize));

  ImGui::End();
}

void Game::renderMainMenu() {
  float scale = hudScale();
  float w = static_cast<float>(width);
  float h = static_cast<float>(height);

  ImVec2 panelSize(600.0f * scale, 400.0f * scale);

  ImGui::SetNextWindowPos(ImVec2(w * 0.5f, h * 0.5f),
                          ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(panelSize, ImGuiCond_Always);

  ImGuiWindowFlags flags =
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

  ImGui::Begin("MainMenu", nullptr, flags);
  ImGui::SetWindowFontScale(scale);

  ImGui::Dummy(ImVec2(0.0f, 10.0f * scale));
  {
    const char *titleText = title;
    float textWidth = ImGui::CalcTextSize(titleText).x;
    centerNextItem(textWidth, panelSize.x);
    ImGui::Text("%s", titleText);
  }

  ImGui::Dummy(ImVec2(0.0f, 30.0f * scale));

  ImVec2 buttonSize(220.0f * scale, 50.0f * scale);

  centerNextItem(buttonSize.x, panelSize.x);
  if (ImGui::Button("Play", buttonSize)) {
    startNewGame();
  }

  ImGui::Dummy(ImVec2(0.0f, 12.0f * scale));

  centerNextItem(buttonSize.x, panelSize.x);
  if (ImGui::Button("Quit", buttonSize)) {
    glfwSetWindowShouldClose(wnd, true);
  }

  ImGui::Dummy(ImVec2(0.0f, 20.0f * scale));

  {
    const char *helpLines[] = {
        "Left/Right arrows : Move",
        "Up/Down arrows : Accelerate/Decelerate",
        "Space : Change altitude",
        "Esc : Pause",
    };

    for (const char *line : helpLines) {
      float textWidth = ImGui::CalcTextSize(line).x;

      centerNextItem(textWidth, panelSize.x);

      ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", line);
    }
  }

  ImGui::SetWindowFontScale(1.0f);
  ImGui::End();
}

void Game::renderPauseMenu() {
  float scale = hudScale();
  float w = static_cast<float>(width);
  float h = static_cast<float>(height);

  ImDrawList *bgDrawList = ImGui::GetBackgroundDrawList();
  bgDrawList->AddRectFilled(ImVec2(0, 0), ImVec2(w, h),
                            IM_COL32(0, 0, 0, 140));

  ImVec2 panelSize(360.0f * scale, 360.0f * scale);

  ImGui::SetNextWindowPos(ImVec2(w * 0.5f, h * 0.5f),
                          ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(panelSize, ImGuiCond_Always);

  ImGuiWindowFlags flags =
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

  ImGui::Begin("PauseMenu", nullptr, flags);
  ImGui::SetWindowFontScale(scale);

  ImGui::Dummy(ImVec2(0.0f, 10.0f * scale));
  {
    const char *titleText = "PAUSED";
    float textWidth = ImGui::CalcTextSize(titleText).x;
    centerNextItem(textWidth, panelSize.x);
    ImGui::Text("%s", titleText);
  }

  ImGui::Dummy(ImVec2(0.0f, 20.0f * scale));

  ImVec2 buttonSize(200.0f * scale, 45.0f * scale);

  centerNextItem(buttonSize.x, panelSize.x);
  if (ImGui::Button("Resume", buttonSize)) {
    togglePause();
  }

  ImGui::Dummy(ImVec2(0.0f, 10.0f * scale));

  centerNextItem(buttonSize.x, panelSize.x);
  if (ImGui::Button("Main Menu", buttonSize)) {
    state = GameState::MainMenu;
    glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }

  ImGui::Dummy(ImVec2(0.0f, 10.0f * scale));

  centerNextItem(buttonSize.x, panelSize.x);
  if (ImGui::Button("Quit", buttonSize)) {
    glfwSetWindowShouldClose(wnd, true);
  }

  ImGui::SetWindowFontScale(1.0f);
  ImGui::End();
}

void Game::renderGameOverMenu() {
  float scale = hudScale();
  float w = static_cast<float>(width);
  float h = static_cast<float>(height);

  ImDrawList *bgDrawList = ImGui::GetBackgroundDrawList();
  bgDrawList->AddRectFilled(ImVec2(0, 0), ImVec2(w, h),
                            IM_COL32(0, 0, 0, 160));

  ImVec2 panelSize(420.0f * scale, 400.0f * scale);

  ImGui::SetNextWindowPos(ImVec2(w * 0.5f, h * 0.5f),
                          ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(panelSize, ImGuiCond_Always);

  ImGuiWindowFlags flags =
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

  ImGui::Begin("GameOverMenu", nullptr, flags);
  ImGui::SetWindowFontScale(scale);

  ImGui::Dummy(ImVec2(0.0f, 10.0f * scale));
  {
    const char *titleText = "GAME OVER";
    float textWidth = ImGui::CalcTextSize(titleText).x;
    centerNextItem(textWidth, panelSize.x);
    ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "%s",
                       titleText);
  }

  ImGui::Dummy(ImVec2(0.0f, 12.0f * scale));
  {
    char scoreText[64];
    snprintf(scoreText, sizeof(scoreText), "Score: %d", finalScore);
    float textWidth = ImGui::CalcTextSize(scoreText).x;
    centerNextItem(textWidth, panelSize.x);
    ImGui::Text("%s", scoreText);
  }

  ImGui::Dummy(ImVec2(0.0f, 24.0f * scale));

  ImVec2 buttonSize(220.0f * scale, 48.0f * scale);

  centerNextItem(buttonSize.x, panelSize.x);
  if (ImGui::Button("Play Again", buttonSize)) {
    startNewGame();
  }

  ImGui::Dummy(ImVec2(0.0f, 10.0f * scale));

  centerNextItem(buttonSize.x, panelSize.x);
  if (ImGui::Button("Main Menu", buttonSize)) {
    state = GameState::MainMenu;
  }

  ImGui::Dummy(ImVec2(0.0f, 10.0f * scale));

  centerNextItem(buttonSize.x, panelSize.x);
  if (ImGui::Button("Quit", buttonSize)) {
    glfwSetWindowShouldClose(wnd, true);
  }

  ImGui::SetWindowFontScale(1.0f);
  ImGui::End();
}

void Game::renderLoadingScreen() {
  float scale = hudScale();
  float w = static_cast<float>(width);
  float h = static_cast<float>(height);

  ImDrawList *bgDrawList = ImGui::GetBackgroundDrawList();
  bgDrawList->AddRectFilled(ImVec2(0, 0), ImVec2(w, h),
                            IM_COL32(10, 10, 14, 255));

  ImVec2 panelSize(420.0f * scale, 140.0f * scale);

  ImGui::SetNextWindowPos(ImVec2(w * 0.5f, h * 0.5f),
                          ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(panelSize, ImGuiCond_Always);

  ImGuiWindowFlags flags =
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground;

  ImGui::Begin("Loading", nullptr, flags);
  ImGui::SetWindowFontScale(scale);

  ImGui::Dummy(ImVec2(0.0f, 12.0f * scale));

  ImVec2 barSize(360.0f * scale, 22.0f * scale);
  centerNextItem(barSize.x, panelSize.x);
  ImGui::ProgressBar(loadProgress, barSize);

  ImGui::Dummy(ImVec2(0.0f, 8.0f * scale));
  {
    float textWidth = ImGui::CalcTextSize(loadStatusText.c_str()).x;
    centerNextItem(textWidth, panelSize.x);
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s",
                       loadStatusText.c_str());
  }

  ImGui::SetWindowFontScale(1.0f);
  ImGui::End();
}

void Game::renderHud() {
  float scale = hudScale();
  float margin = 16.0f * scale;

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  if (hudFont) {
    ImGui::PushFont(hudFont);
  }

  ImGuiWindowFlags hudFlags =
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoBackground |
      ImGuiWindowFlags_NoSavedSettings |
      ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

  float scrheight = static_cast<float>(height);

  switch (state) {
  case GameState::Loading:
    renderLoadingScreen();
    break;

  case GameState::MainMenu:
    renderMainMenu();
    break;

  case GameState::Playing:
    renderHudScore(hudFlags, ImVec2(margin, margin),
                   ImVec2(0.0f, 0.0f));
    renderHudPlayerStats(hudFlags,
                         ImVec2(margin, scrheight - (margin * 3.0f)),
                         ImVec2(0.0f, 1.0f));
    break;

  case GameState::Paused:
    renderHudScore(hudFlags, ImVec2(margin, margin),
                   ImVec2(0.0f, 0.0f));
    renderHudPlayerStats(hudFlags,
                         ImVec2(margin, scrheight - (margin * 3.0f)),
                         ImVec2(0.0f, 1.0f));
    renderPauseMenu();
    break;

  case GameState::GameOver:
    renderGameOverMenu();
    break;
  }

  if (hudFont) {
    ImGui::PopFont();
  }

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Game::render() {
  glViewport(0, 0, static_cast<int>(width), static_cast<int>(height));
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (state != GameState::Loading) {
    auto playerPos = player->getObject().pos;

    glm::vec3 cameraPos = playerPos + cameraOffset;
    glm::vec3 cameraTarget =
        playerPos + glm::vec3(0.0f, -1.0f, -6.0f);
    glm::mat4 view =
        glm::lookAt(cameraPos, cameraTarget, glm::vec3(0, 1, 0));

    float aspect = height > 0 ? static_cast<float>(width) /
                                    static_cast<float>(height)
                              : 1.0f;
    glm::mat4 projection =
        glm::perspective(glm::radians(60.0f), aspect, 0.1f, 300.0f);

    renderSkybox(view, projection);
    player->draw(view, projection);
    obstacleSpawner->render(view, projection);
  }

  renderHud();
  glfwSwapBuffers(wnd);
}

void Game::processInput() {
  bool escDown = glfwGetKey(wnd, GLFW_KEY_ESCAPE) == GLFW_PRESS;
  if (escDown && !pauseKeyWasDown) {
    if (state == GameState::Playing || state == GameState::Paused) {
      togglePause();
    } else if (state == GameState::MainMenu) {
      glfwSetWindowShouldClose(wnd, true);
    }
  }
  pauseKeyWasDown = escDown;

  if (state != GameState::Playing) {
    return;
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

void Game::run() {
  if (wnd == nullptr) {
    std::cerr << "[Game] game is not initialized\n";
    return;
  }

  glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  lastFrameTime = static_cast<float>(glfwGetTime());

  while (!glfwWindowShouldClose(wnd)) {
    processInput();
    update();
    render();
    glfwPollEvents();
  }
}
