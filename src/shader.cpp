#include "shader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

/*
 * Reads a file based on the path input and then returns the
 * file contents as a std::string.
 *
 * @param path | the file path
 *
 * @return file contents as string
 */
std::string Shader::readFile(const char *path) {
  std::ifstream file(path);

  if (!file.is_open()) {
    std::cerr << "[Shader] failed to open file: " << path << "\n";
    return "";
  }

  std::stringstream buf;
  buf << file.rdbuf();

  return buf.str();
}

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
unsigned int Shader::compile(unsigned int type,
                             const std::string &source,
                             const char *debugName) {
  unsigned int shader = glCreateShader(type);
  const char *src = source.c_str();
  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);

  int success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

  if (!success) {
    unsigned int logSize = 1024;
    char infoLog[logSize];
    glGetShaderInfoLog(shader, logSize, nullptr, infoLog);
    std::cerr << "[Shader] compile error in " << debugName << ":\n"
              << infoLog << "\n";
  }

  return shader;
}

/*
 * Receives two vertexPath and fragmentPath shaders,
 * compiles them, links them to the program, checks for error,
 * then deletes the unused shader objects.
 *
 * @param vertexPath
 * @param fragmentPath
 */
Shader::Shader(const char *vertexPath, const char *fragmentPath) {
  std::string vertexCode = readFile(vertexPath);
  std::string fragmentCode = readFile(fragmentPath);

  unsigned int vertexShader =
      compile(GL_VERTEX_SHADER, vertexCode, vertexPath);
  unsigned int fragmentShader =
      compile(GL_FRAGMENT_SHADER, fragmentCode, fragmentPath);

  programId = glCreateProgram();
  glAttachShader(programId, vertexShader);
  glAttachShader(programId, fragmentShader);
  glLinkProgram(programId);

  checkLinkErrors();

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

/*
 * If programId exists, then it will delete the program.
 */
Shader::~Shader() {
  if (programId) {
    glDeleteProgram(programId);
  }
}

/*
 * Checks the link error, then logs it in stderr.
 */
void Shader::checkLinkErrors() const {
  int success;
  glGetProgramiv(programId, GL_LINK_STATUS, &success);

  if (!success) {
    unsigned int logSize = 1024;
    char infoLog[logSize];
    glGetProgramInfoLog(programId, logSize, nullptr, infoLog);
    std::cerr << "[Shader] link error: " << infoLog << "\n";
  }
}

/*
 * Set GL to use the shader's program.
 */
void Shader::use() const { glUseProgram(programId); }

/*
 * Uploads/Links a 4*4 matrix to the shader.
 *
 * @param name | name of the matrix
 * @param value | the matrix itself
 */
void Shader::setMat4(const std::string &name,
                     const glm::mat4 &value) const {
  glUniformMatrix4fv(glGetUniformLocation(programId, name.c_str()), 1,
                     GL_FALSE, &value[0][0]);
}

/*
 * Uploads/Links a 3*1 vector to the shader.
 *
 * @param name | name of the vector
 * @param value | the vector itself
 */
void Shader::setVec3(const std::string &name,
                     const glm::vec3 &value) const {
  glUniform3fv(glGetUniformLocation(programId, name.c_str()), 1,
               &value[0]);
}

/*
 * Uploads/Links a float value to the shader.
 *
 * @param name | name of the float
 * @param value | the float value itself
 */
void Shader::setFloat(const std::string &name, float value) const {
  glUniform1f(glGetUniformLocation(programId, name.c_str()), value);
}

/*
 * Uploads/Links a integer value to the shader.
 *
 * @param name | name of the integer
 * @param value | the integer value itself
 */
void Shader::setInt(const std::string &name, int value) const {
  glUniform1i(glGetUniformLocation(programId, name.c_str()), value);
}
