#include "game.hpp"

#include <GLFW/glfw3.h>
#include <iostream>

int main() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    return -1;
  }

  GLFWmonitor *monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode *mode = glfwGetVideoMode(monitor);

  GameOptions opts;
  opts.title = "Astro Rush";
  opts.width = mode->width;
  opts.height = mode->height;

  glfwTerminate();

  Game g(opts);
  g.run();

  return 0;
}
