#pragma once

#include <GL/glew.h>
#include <vector>

class Mesh {
public:
  Mesh(const std::vector<float> &verts,
       const std::vector<unsigned int> &idxs);
  ~Mesh();

  void draw() const;

  static Mesh createPlane(float width, float depth);

private:
  unsigned int VAO = 0, VBO = 0, EBO = 0, idxcnt = 0;
};
