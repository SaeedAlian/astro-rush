#pragma once

#include "mesh.hpp"
#include "shader.hpp"

class Object {
public:
  glm::vec3 pos{0.0f};
  glm::vec3 scale{1.0f};
  float rotationY = 0.0f;
  glm::vec3 color{1.0f};

  Object(Mesh *mesh, Shader *shader);

  glm::mat4 getModelMat() const;
  void draw(const glm::mat4 &view, const glm::mat4 &projection) const;

private:
  Mesh *mesh;
  Shader *shader;
};
