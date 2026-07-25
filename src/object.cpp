#include "object.hpp"

#include <glm/gtc/matrix_transform.hpp>

Object::Object(Mesh *mesh, glm::vec3 defaultColor, Texture *texture,
               Shader *shader)
    : parts{{mesh, defaultColor, texture}}, shader(shader) {}

Object::Object(std::vector<ObjectPart> parts, Shader *shader)
    : parts(std::move(parts)), shader(shader) {}

/*
 * Calculates the object model matrix based on its position,
 * scale and rotation (in radians) through the Y axis.
 */
glm::mat4 Object::getModelMat() const {
  glm::mat4 model = glm::mat4(1.0f);

  model = glm::translate(model, pos);
  model = glm::rotate(model, rotationY, glm::vec3(0, 1, 0));
  model = glm::scale(model, scale);

  return model;
}

/*
 * Sets the view, projection and model matrices in shader,
 * and draws the object.
 *
 * @param view | view matrix (transforms it to camera coords)
 * @param projection | projection matrix (transforms it to screen
 * coords)
 */
void Object::draw(const glm::mat4 &view,
                  const glm::mat4 &projection) const {
  shader->use();

  shader->setMat4("model", getModelMat());
  shader->setMat4("projection", projection);
  shader->setMat4("view", view);

  for (const auto &part : parts) {
    shader->setVec3("objColor", part.color);

    bool hasTexture = part.texture != nullptr;
    shader->setInt("hasTexture", hasTexture ? 1 : 0);

    if (hasTexture) {
      part.texture->bind(0);
      shader->setInt("diffuseTex", 0);
    }

    part.mesh->draw();
  }
}
