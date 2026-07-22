#include <mesh.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

/*
 * Initializes a mesh by its vertices and indices.
 * Binds the vertices and indices to the VAO, VBO and EBO
 * that it owns.
 *
 * @param verts | mesh vertices
 * @param idxs | mesh indices
 */
Mesh::Mesh(const std::vector<float> &verts,
           const std::vector<unsigned int> &idxs) {
  idxcnt = static_cast<unsigned int>(idxs.size());

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float),
               verts.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               idxs.size() * sizeof(unsigned int), idxs.data(),
               GL_STATIC_DRAW);

  // position (location 0), normal (location 1)
  unsigned int stride = 6 * sizeof(float);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}

/*
 * Removes and deletes VAO, VBO and EBO.
 */
Mesh::~Mesh() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
}

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
Mesh::Mesh(Mesh &&other) noexcept
    : VAO(other.VAO), VBO(other.VBO), EBO(other.EBO),
      idxcnt(other.idxcnt) {
  other.VAO = other.VBO = other.EBO = 0;
  other.idxcnt = 0;
}

/*
 * The reverse operation of move constructor (move assignment).
 * This will moves the ownership of values from other Mesh
 * to this Mesh, and leaves the other in an empty state.
 *
 * @param other | the Mesh being moved from (left empty afterward)
 *
 * @return reference to this Mesh, for assignment chaining
 */
Mesh &Mesh::operator=(Mesh &&other) noexcept {
  if (this != &other) {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    VAO = other.VAO;
    VBO = other.VBO;
    EBO = other.EBO;
    idxcnt = other.idxcnt;

    other.VAO = other.VBO = other.EBO = 0;
    other.idxcnt = 0;
  }
  return *this;
}

/*
 * Draws the triangles (primitives) from the VAO.
 */
void Mesh::draw() const {
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, idxcnt, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

/*
 * Default static method to create a cube.
 *
 * @param size
 *
 * @return cube mesh
 */
Mesh Mesh::createCube(float size) {
  float h = size * 0.5f;

  // clang-format off
  std::vector<float> vertices = {
      // position       // normal
      // front 
      -h, -h,  h,       0, 0, 1,
       h, -h,  h,       0, 0, 1,
       h,  h,  h,       0, 0, 1,
      -h,  h,  h,       0, 0, 1,
      // back 
       h, -h, -h,       0, 0, -1,
      -h, -h, -h,       0, 0, -1,
      -h,  h, -h,       0, 0, -1,
       h,  h, -h,       0, 0, -1,
      // left
      -h, -h, -h,  -    1, 0, 0,
      -h, -h,  h,  -    1, 0, 0,
      -h,  h,  h,  -    1, 0, 0,
      -h,  h, -h,  -    1, 0, 0,
      // right
       h, -h,  h,       1, 0, 0,
       h, -h, -h,       1, 0, 0,
       h,  h, -h,       1, 0, 0,
       h,  h,  h,       1, 0, 0,
      // top 
      -h,  h,  h,       0, 1, 0,
       h,  h,  h,       0, 1, 0,
       h,  h, -h,       0, 1, 0,
      -h,  h, -h,       0, 1, 0,
      // bottom
      -h, -h, -h,       0, -1, 0,
       h, -h, -h,       0, -1, 0,
       h, -h,  h,       0, -1, 0,
      -h, -h,  h,       0, -1, 0,
  };
  std::vector<unsigned int> indices = {
      0, 1, 2,       2, 3, 0,        // front
      4, 5, 6,       6, 7, 4,        // back
      8, 9, 10,      10, 11, 8,      // left
      12, 13, 14,    14, 15, 12,     // right
      16, 17, 18,    18, 19, 16,     // top
      20, 21, 22,    22, 23, 20,     // bottom
  };
  // clang-format on

  return Mesh(vertices, indices);
}

/*
 * Default static method to create a box.
 *
 * @param width
 * @param height
 * @param depth
 *
 * @return box mesh
 */
Mesh Mesh::createBox(float width, float height, float depth) {
  float w = width * 0.5f;
  float h = height * 0.5f;
  float d = depth * 0.5f;

  // clang-format off
  std::vector<float> vertices = {
      // position       // normal
      // front 
      -w, -h,  d,       0, 0, 1,
       w, -h,  d,       0, 0, 1,
       w,  h,  d,       0, 0, 1,
      -w,  h,  d,       0, 0, 1,
      // back 
       w, -h, -d,       0, 0, -1,
      -w, -h, -d,       0, 0, -1,
      -w,  h, -d,       0, 0, -1,
       w,  h, -d,       0, 0, -1,
      // left
      -w, -h, -d,  -    1, 0, 0,
      -w, -h,  d,  -    1, 0, 0,
      -w,  h,  d,  -    1, 0, 0,
      -w,  h, -d,  -    1, 0, 0,
      // right
       w, -h,  d,       1, 0, 0,
       w, -h, -d,       1, 0, 0,
       w,  h, -d,       1, 0, 0,
       w,  h,  d,       1, 0, 0,
      // top 
      -w,  h,  d,       0, 1, 0,
       w,  h,  d,       0, 1, 0,
       w,  h, -d,       0, 1, 0,
      -w,  h, -d,       0, 1, 0,
      // bottom
      -w, -h, -d,       0, -1, 0,
       w, -h, -d,       0, -1, 0,
       w, -h,  d,       0, -1, 0,
      -w, -h,  d,       0, -1, 0,
  };
  std::vector<unsigned int> indices = {
      0, 1, 2,       2, 3, 0,        // front
      4, 5, 6,       6, 7, 4,        // back
      8, 9, 10,      10, 11, 8,      // left
      12, 13, 14,    14, 15, 12,     // right
      16, 17, 18,    18, 19, 16,     // top
      20, 21, 22,    22, 23, 20,     // bottom
  };
  // clang-format on

  return Mesh(vertices, indices);
}

/*
 * Default static method to create a plane.
 *
 * @param width
 * @param depth
 *
 * @return plane mesh
 */
Mesh Mesh::createPlane(float width, float depth) {
  float hw = width * 0.5f;
  float hd = depth * 0.5f;

  // clang-format off
  std::vector<float> vertices = {
      // pos           // normal
      -hw, 0.0f,  hd,   0, 1, 0,
       hw, 0.0f,  hd,   0, 1, 0,
       hw, 0.0f, -hd,   0, 1, 0,
      -hw, 0.0f, -hd,   0, 1, 0,
  };
  std::vector<unsigned int> indices = {
      0, 1, 2,
      2, 3, 0,
  };
  // clang-format on

  return Mesh(vertices, indices);
}
