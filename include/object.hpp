#pragma once

#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"

/*
 * A single renderable part of an object.
 *
 * An object can consist of multiple parts, where each part has its
 * own mesh and material properties.
 */
struct ObjectPart {
  Mesh *mesh;
  glm::vec3 color;
  Texture *texture = nullptr;
};

/*
 * A renderable object in the scene.
 *
 * An Object combines one or more mesh parts with a shader and a
 * world transformation. The transformation is defined by its
 * position, scale, and rotation values.
 *
 * Objects do not own meshes, textures, or shaders. They only store
 * references to these resources.
 */
class Object {
public:
  /*
   * Position of the object in world coordinates.
   */
  glm::vec3 pos{0.0f};

  /*
   * Scale vector of the object.
   */
  glm::vec3 scale{1.0f};

  float rotationX = 0.0f;
  float rotationY = 0.0f;
  float rotationZ = 0.0f;

  /*
   * Initialize an object with single mesh (single part object).
   *
   * @param mesh
   * @param defaultColor
   * @param texture
   * @param shader
   */
  Object(Mesh *mesh, glm::vec3 defaultColor, Texture *texture,
         Shader *shader);

  /*
   * Initialize an object with multiple object parts.
   *
   * @param parts
   * @param shader
   */
  Object(std::vector<ObjectPart> parts, Shader *shader);

  /*
   * Calculates the object model matrix based on its position,
   * scale and rotation (in radians) through the X,Y,Z axis.
   */
  glm::mat4 getModelMat() const;

  /*
   * Sets the view, projection and model matrices in shader,
   * and draws the object.
   *
   * @param view | view matrix (transforms it to camera coords)
   * @param projection | projection matrix (transforms it to screen
   * coords)
   */
  void draw(const glm::mat4 &view, const glm::mat4 &projection) const;

private:
  std::vector<ObjectPart> parts;
  Shader *shader;
};
