#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <string>

/*
 * Represents a program that will run on GPU.
 *
 * It will be binded with a programId and it can be used by GL.
 */
class Shader {
public:
  /*
   * Receives two vertexPath and fragmentPath shaders,
   * compiles them, links them to the program, checks for error,
   * then deletes the unused shader objects.
   *
   * @param vertexPath
   * @param fragmentPath
   */
  Shader(const char *vertexPath, const char *fragmentPath);

  /*
   * If programId exists, then it will delete the program.
   */
  ~Shader();

  /*
   * Set GL to use the shader's program.
   */
  void use() const;

  /*
   * Uploads/Links a 4*4 matrix to the shader.
   *
   * @param name | name of the matrix
   * @param value | the matrix itself
   */
  void setMat4(const std::string &name, const glm::mat4 &value) const;

  /*
   * Uploads/Links a 3*1 vector to the shader.
   *
   * @param name | name of the vector
   * @param value | the vector itself
   */
  void setVec3(const std::string &name, const glm::vec3 &value) const;

  /*
   * Uploads/Links a float value to the shader.
   *
   * @param name | name of the float
   * @param value | the float value itself
   */
  void setFloat(const std::string &name, float value) const;

  /*
   * Uploads/Links a integer value to the shader.
   *
   * @param name | name of the integer
   * @param value | the integer value itself
   */
  void setInt(const std::string &name, int value) const;

  /*
   * @return program id binded with shader
   */
  unsigned int getId() const { return programId; }

private:
  unsigned int programId = 0;

  /*
   * Reads a file based on the path input and then returns the
   * file contents as a std::string.
   *
   * @param path | the file path
   *
   * @return file contents as string
   */
  static std::string readFile(const char *path);

  /*
   * Gets a type and a string (contents) of a shader,
   * then compiles it. It also receives a debugName param,
   * for identifying the shader if compile process gives an error.
   *
   * @param type | the shader type (vertex, fragment, ...)
   * @param source | the contents of the shader as a string
   * @param debugName | the identifier of the shader for debugging
   * purposes
   *
   * @return an unsigned integer to reference the compiled shader
   */
  static unsigned int compile(unsigned int type,
                              const std::string &source,
                              const char *debugName);

  /*
   * Checks the link error, then logs it in stderr.
   */
  void checkLinkErrors() const;
};
