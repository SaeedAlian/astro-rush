#pragma once

#include <GL/glew.h>
#include <vector>

class Mesh {
public:
  Mesh(const std::vector<float> &verts,
       const std::vector<unsigned int> &idxs);
  ~Mesh();

  Mesh(const Mesh &) = delete;
  Mesh &operator=(const Mesh &) = delete;
  Mesh(Mesh &&other) noexcept;
  Mesh &operator=(Mesh &&other) noexcept;

  void draw() const;

  static Mesh createCube(float size);
  static Mesh createPlane(float width, float depth);

private:
  unsigned int VAO = 0, VBO = 0, EBO = 0, idxcnt = 0;
};
