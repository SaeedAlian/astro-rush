#pragma once

#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"

struct ObjectPart {
  Mesh *mesh;
  glm::vec3 color;
  Texture *texture = nullptr;
};

class Object {
public:
  glm::vec3 pos{0.0f};
  glm::vec3 scale{1.0f};

  float rotationX = 0.0f;
  float rotationY = 0.0f;
  float rotationZ = 0.0f;

  Object(Mesh *mesh, glm::vec3 defaultColor, Texture *texture,
         Shader *shader);
  Object(std::vector<ObjectPart> parts, Shader *shader);

  glm::mat4 getModelMat() const;
  void draw(const glm::mat4 &view, const glm::mat4 &projection) const;

private:
  std::vector<ObjectPart> parts;
  Shader *shader;
};
