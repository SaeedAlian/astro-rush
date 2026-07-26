#pragma once

#include <GL/glew.h>

#include <glm/ext/vector_float3.hpp>

#include <string>
#include <vector>

struct MeshBounds {
  glm::vec3 min{0.0f};
  glm::vec3 max{0.0f};

  glm::vec3 size() const { return (max - min); }
  glm::vec3 center() const { return (max + min) * 0.5f; }
};

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
  static Mesh createBox(float width, float height, float depth);
  static Mesh createPlane(float width, float depth);
  static std::vector<struct LoadedMesh>
  loadObj(const std::string &path, MeshBounds *bounds = nullptr);

private:
  unsigned int VAO = 0, VBO = 0, EBO = 0, idxcnt = 0;
};

struct MeshMaterial {
  glm::vec3 diffuseColor{1.0f};
  std::string diffuseTexPath;
};

struct LoadedMesh {
  Mesh mesh;
  MeshMaterial material;
};
