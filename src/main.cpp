#include "mesh.hpp"
#include "object.hpp"
#include "shader.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

int main() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(
      1920, 1080, "Endless Runner 3D", nullptr, nullptr);

  if (!window) {
    std::cerr << "Failed to create window\n";
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW\n";
    return -1;
  }

  Mesh plane = Mesh::createPlane(10.0f, 50.0f);

  Shader shader =
      Shader("shaders/vertex.glsl", "shaders/fragment.glsl");

  Object road = Object(&plane, &shader);

  glm::vec3 cameraPosition(0.0f, 10.0f, 10.0f);

  glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);

  glm::mat4 view =
      glm::lookAt(cameraPosition, cameraTarget, glm::vec3(0, 1, 0));

  glm::mat4 projection = glm::perspective(
      glm::radians(90.0f), 800.0f / 600.0f, 0.1f, 200.0f);

  road.color = glm::vec3(0.45, 0.15, 0.15);

  while (!glfwWindowShouldClose(window)) {
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, true);
    }

    glClearColor(0.2f, 0.3f, 0.4f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    road.draw(view, projection);

    glfwSwapBuffers(window);

    glfwPollEvents();
  }

  glfwTerminate();

  return 0;
}
