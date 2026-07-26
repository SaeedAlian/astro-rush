#include "mesh.hpp"

#include <tiny_obj_loader.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <cstdlib>
#include <iostream>
#include <limits>

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

  // position (location 0), normal (location 1), uv (location 2)
  unsigned int stride = 8 * sizeof(float);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
                        (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

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
      // position       // normal       // uv
      // front 
      -h, -h,  h,       0, 0, 1,        0, 0,
       h, -h,  h,       0, 0, 1,        0, 0,
       h,  h,  h,       0, 0, 1,        0, 0,
      -h,  h,  h,       0, 0, 1,        0, 0,
      // back 
       h, -h, -h,       0, 0, -1,       0, 0,
      -h, -h, -h,       0, 0, -1,       0, 0,
      -h,  h, -h,       0, 0, -1,       0, 0,
       h,  h, -h,       0, 0, -1,       0, 0,
      // left
      -h, -h, -h,  -    1, 0, 0,        0, 0,
      -h, -h,  h,  -    1, 0, 0,        0, 0,
      -h,  h,  h,  -    1, 0, 0,        0, 0,
      -h,  h, -h,  -    1, 0, 0,        0, 0,
      // right
       h, -h,  h,       1, 0, 0,        0, 0,
       h, -h, -h,       1, 0, 0,        0, 0,
       h,  h, -h,       1, 0, 0,        0, 0,
       h,  h,  h,       1, 0, 0,        0, 0,
      // top 
      -h,  h,  h,       0, 1, 0,        0, 0,
       h,  h,  h,       0, 1, 0,        0, 0,
       h,  h, -h,       0, 1, 0,        0, 0,
      -h,  h, -h,       0, 1, 0,        0, 0,
      // bottom
      -h, -h, -h,       0, -1, 0,       0, 0,
       h, -h, -h,       0, -1, 0,       0, 0,
       h, -h,  h,       0, -1, 0,       0, 0,
      -h, -h,  h,       0, -1, 0,       0, 0,
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
      // position       // normal       // uv
      // front 
      -w, -h,  d,       0, 0, 1,        0, 0,
       w, -h,  d,       0, 0, 1,        0, 0,
       w,  h,  d,       0, 0, 1,        0, 0,
      -w,  h,  d,       0, 0, 1,        0, 0,
      // back 
       w, -h, -d,       0, 0, -1,       0, 0,
      -w, -h, -d,       0, 0, -1,       0, 0,
      -w,  h, -d,       0, 0, -1,       0, 0,
       w,  h, -d,       0, 0, -1,       0, 0,
      // left
      -w, -h, -d,  -    1, 0, 0,        0, 0,
      -w, -h,  d,  -    1, 0, 0,        0, 0,
      -w,  h,  d,  -    1, 0, 0,        0, 0,
      -w,  h, -d,  -    1, 0, 0,        0, 0,
      // right
       w, -h,  d,       1, 0, 0,        0, 0,
       w, -h, -d,       1, 0, 0,        0, 0,
       w,  h, -d,       1, 0, 0,        0, 0,
       w,  h,  d,       1, 0, 0,        0, 0,
      // top 
      -w,  h,  d,       0, 1, 0,        0, 0,
       w,  h,  d,       0, 1, 0,        0, 0,
       w,  h, -d,       0, 1, 0,        0, 0,
      -w,  h, -d,       0, 1, 0,        0, 0,
      // bottom
      -w, -h, -d,       0, -1, 0,       0, 0,
       w, -h, -d,       0, -1, 0,       0, 0,
       w, -h,  d,       0, -1, 0,       0, 0,
      -w, -h,  d,       0, -1, 0,       0, 0,
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
      // pos           // normal      // uv
      -hw, 0.0f,  hd,   0, 1, 0,       0, 0,
       hw, 0.0f,  hd,   0, 1, 0,       0, 0,
       hw, 0.0f, -hd,   0, 1, 0,       0, 0,
      -hw, 0.0f, -hd,   0, 1, 0,       0, 0,
  };
  std::vector<unsigned int> indices = {
      0, 1, 2,
      2, 3, 0,
  };
  // clang-format on

  return Mesh(vertices, indices);
}

/*
 * Loads .obj file and creates a LoadedMesh vector from the object
 * face materials.
 *
 * First, it loads the .obj file from path, checks for errors, then
 * loads the .mtl file related to the .obj.
 * From each face material (bucket), extract the vertices, indices and
 * uvs, then build the result (LoadedMesh) from the buckets.
 * LoadedMesh contains the mesh itself and the mesh material.
 * The mesh material can have textures map to it, so they will
 * be automatically loaded from the base path.
 *
 * @param path | .obj file path
 * @param bounds | a pointer to a MeshBounds struct to pass the outer
 * min,max bounds
 *
 * @return the (mesh, material) pair per each bucket
 */
std::vector<LoadedMesh> Mesh::loadObj(const std::string &path,
                                      MeshBounds *bounds) {
  tinyobj::ObjReaderConfig config;
  config.triangulate = true;
  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(path, config)) {
    std::cerr << "[Mesh] failed to load obj file: " << path << "\n";
    exit(1);
  }

  if (!reader.Error().empty()) {
    std::cerr << "[Mesh] reader error: " << reader.Error() << "\n";
    exit(1);
  }
  if (!reader.Warning().empty()) {
    std::cerr << "[Mesh] reader warning: " << reader.Warning()
              << "\n";
  }

  const auto &attrib = reader.GetAttrib();
  const auto &shapes = reader.GetShapes();
  const auto &materials = reader.GetMaterials();

  // vertices, indices and dedup tables per material id
  std::unordered_map<int, std::vector<float>> vertsByMaterial;
  std::unordered_map<int, std::vector<unsigned int>> idxsByMaterial;
  std::unordered_map<int,
                     std::unordered_map<std::string, unsigned int>>
      dedupByMaterial;

  // gets the outer bounds
  glm::vec3 globalMin(std::numeric_limits<float>::max());
  glm::vec3 globalMax(-std::numeric_limits<float>::max());

  for (const auto &shape : shapes) {
    size_t indexOffset = 0; // pos from mesh.indices

    for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
      int matId = f < shape.mesh.material_ids.size()
                      ? shape.mesh.material_ids[f]
                      : -1;

      unsigned char faceVerts = shape.mesh.num_face_vertices[f];

      auto &verts = vertsByMaterial[matId];
      auto &idxs = idxsByMaterial[matId];
      auto &dedup = dedupByMaterial[matId];

      for (unsigned char v = 0; v < faceVerts; ++v) {
        const auto &index =
            shape.mesh.indices[indexOffset + v]; // single corner of
                                                 // the current face

        // unique per distinct vertex
        std::string key = std::to_string(index.vertex_index) + "/" +
                          std::to_string(index.normal_index) + "/" +
                          std::to_string(index.texcoord_index);

        auto it = dedup.find(key);

        // if we saw the duplicate vertex, just reuse the index
        if (it != dedup.end()) {
          idxs.push_back(it->second);
          continue;
        }

        float px = attrib.vertices[3 * index.vertex_index + 0];
        float py = attrib.vertices[3 * index.vertex_index + 1];
        float pz = attrib.vertices[3 * index.vertex_index + 2];

        float nx = 0.0f, ny = 0.0f, nz = 0.0f;
        if (index.normal_index >= 0) {
          nx = attrib.normals[3 * index.normal_index + 0];
          ny = attrib.normals[3 * index.normal_index + 1];
          nz = attrib.normals[3 * index.normal_index + 2];
        }

        float u = 0.0f, uvY = 0.0f;
        if (index.texcoord_index >= 0) {
          u = attrib.texcoords[2 * index.texcoord_index + 0];
          uvY = attrib.texcoords[2 * index.texcoord_index + 1];
        }

        unsigned int newIndex =
            static_cast<unsigned int>(verts.size() / 8);
        verts.insert(verts.end(), {px, py, pz, nx, ny, nz, u, uvY});
        dedup.emplace(std::move(key), newIndex);
        idxs.push_back(newIndex);

        globalMin = glm::min(globalMin, glm::vec3(px, py, pz));
        globalMax = glm::max(globalMax, glm::vec3(px, py, pz));
      }

      indexOffset += faceVerts; // move past the face
    }
  }

  std::vector<LoadedMesh> result;

  // put back the result (buckets) together
  for (auto &[matId, verts] : vertsByMaterial) {
    MeshMaterial material;

    if (matId >= 0 && matId < static_cast<int>(materials.size())) {
      const auto &m = materials[matId];
      material.diffuseColor =
          glm::vec3(m.diffuse[0], m.diffuse[1], m.diffuse[2]);

      // setup the texture file path
      if (!m.diffuse_texname.empty()) {
        std::string baseDir =
            path.substr(0, path.find_last_of("/\\") + 1);

        std::string texname = m.diffuse_texname;
        for (char &c : texname) {
          if (c == '\\')
            c = '/';
        }

        material.diffuseTexPath = baseDir + texname;
      }
    }

    result.push_back(
        LoadedMesh{Mesh(verts, idxsByMaterial[matId]), material});
  }

  if (bounds) {
    bounds->min = globalMin;
    bounds->max = globalMax;
  }

  return result;
}
