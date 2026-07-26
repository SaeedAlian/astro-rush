#include "object.hpp"

#include <glm/gtc/matrix_transform.hpp>

Object::Object(Mesh *mesh, glm::vec3 defaultColor, Texture *texture,
               Shader *shader)
    : parts{{mesh, defaultColor, texture}}, shader(shader) {}

Object::Object(std::vector<ObjectPart> parts, Shader *shader)
    : parts(std::move(parts)), shader(shader) {}

glm::mat4 Object::getModelMat() const {
  glm::mat4 model = glm::mat4(1.0f);

  model = glm::translate(model, pos);
  model = glm::rotate(model, rotationX, glm::vec3(1, 0, 0));
  model = glm::rotate(model, rotationY, glm::vec3(0, 1, 0));
  model = glm::rotate_slow(model, rotationZ, glm::vec3(0, 0, 1));
  model = glm::scale(model, scale);

  return model;
}

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
