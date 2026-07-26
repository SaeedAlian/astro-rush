#pragma once

#include <GL/glew.h>

#include <glm/ext/vector_float3.hpp>

#include <string>
#include <vector>

/*
 * Outer bounds of a mesh.
 */
struct MeshBounds {
  glm::vec3 min{0.0f};
  glm::vec3 max{0.0f};

  glm::vec3 size() const { return (max - min); }
  glm::vec3 center() const { return (max + min) * 0.5f; }
};

/*
 * A collection of vertices, edges and faces.
 *
 * In this class, the mesh has these properties in shader:
 * - position (location 0) (vec3)
 * - normal (location 1) (vec3)
 * - uv (location 2) (vec2)
 *
 * The class owns its VAO, VBO, and EBO and manages
 * their lifetime automatically.
 */
class Mesh {
public:
  /*
   * Initializes a mesh by its vertices and indices.
   * Binds the vertices and indices to the VAO, VBO and EBO
   * that it owns.
   *
   * @param verts | mesh vertices
   * @param idxs | mesh indices
   */
  Mesh(const std::vector<float> &verts,
       const std::vector<unsigned int> &idxs);

  /*
   * Removes and deletes VAO, VBO and EBO.
   */
  ~Mesh();

  /*
   * Disables copy construction.
   *
   * A mesh owns GL resources (VAO, VBO, EBO), which cannot be safely
   * duplicated by copying the object. Copying is disabled to prevent
   * multiple Mesh instances from owning the same GL handles.
   */
  Mesh(const Mesh &) = delete;

  /*
   * Disables copy assignment.
   */
  Mesh &operator=(const Mesh &) = delete;

  /*
   * Move-Constructs a Mesh, so that when Mesh is created
   * by static functions like createPlane, createCube ...,
   * it won't delete the Mesh itself by calling the destructor
   * of the temporary result returned by these functions.
   *
   * Static function call
   * -> returns a temp Mesh class (rvalue)
   * -> moves the rvalue to lvalue
   * -> move constructor will be called
   * -> the temporary rvalue properties will be reset
   * -> destructor of rvalue will be called
   * -> because the properties have been resetted, it won't destruct
   * the lvalue
   *
   * @param other | the Mesh being moved from (left empty afterward)
   */
  Mesh(Mesh &&other) noexcept;

  /*
   * The reverse operation of move constructor (move assignment).
   * This will moves the ownership of values from other Mesh
   * to this Mesh, and leaves the other in an empty state.
   *
   * @param other | the Mesh being moved from (left empty afterward)
   *
   * @return reference to this Mesh, for assignment chaining
   */
  Mesh &operator=(Mesh &&other) noexcept;

  /*
   * Draws the triangles (primitives) from the VAO.
   */
  void draw() const;

  /*
   * Default static method to create a cube.
   *
   * @param size
   *
   * @return cube mesh
   */
  static Mesh createCube(float size);

  /*
   * Default static method to create a box.
   *
   * @param width
   * @param height
   * @param depth
   *
   * @return box mesh
   */
  static Mesh createBox(float width, float height, float depth);

  /*
   * Default static method to create a plane.
   *
   * @param width
   * @param depth
   *
   * @return plane mesh
   */
  static Mesh createPlane(float width, float depth);

  /*
   * Loads .obj file and creates a LoadedMesh vector from the object
   * face materials.
   *
   * First, it loads the .obj file from path, checks for errors, then
   * loads the .mtl file related to the .obj.
   * From each face material (bucket), extract the vertices, indices
   * and uvs, then build the result (LoadedMesh) from the buckets.
   * LoadedMesh contains the mesh itself and the mesh material.
   * The mesh material can have textures map to it, so they will
   * be automatically loaded from the base path.
   *
   * @param path | .obj file path
   * @param bounds | a pointer to a MeshBounds struct to pass the
   * outer min,max bounds
   *
   * @return the (mesh, material) pair per each bucket
   */
  static std::vector<struct LoadedMesh>
  loadObj(const std::string &path, MeshBounds *bounds = nullptr);

private:
  unsigned int VAO = 0, VBO = 0, EBO = 0, idxcnt = 0;
};

/*
 * Material of a mesh part.
 *
 * Includes diffuse color and texture path.
 */
struct MeshMaterial {
  glm::vec3 diffuseColor{1.0f};
  std::string diffuseTexPath;
};

/*
 * Represents a loaded mesh part (bucket) which is
 * loaded via an .obj file.
 *
 * Includes a mesh and its material
 */
struct LoadedMesh {
  Mesh mesh;
  MeshMaterial material;
};
