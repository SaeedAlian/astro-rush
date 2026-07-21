#include <object.hpp>

#include <glm/gtc/matrix_transform.hpp>

Object::Object(Mesh *mesh, Shader *shader)
    : mesh(mesh), shader(shader) {}

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
  shader->setVec3("objcolor", color);

  mesh->draw();
}
