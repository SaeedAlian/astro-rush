#include "shader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

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

Shader::~Shader() {
  if (programId) {
    glDeleteProgram(programId);
  }
}

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

void Shader::use() const { glUseProgram(programId); }

void Shader::setMat4(const std::string &name,
                     const glm::mat4 &value) const {
  glUniformMatrix4fv(glGetUniformLocation(programId, name.c_str()), 1,
                     GL_FALSE, &value[0][0]);
}

void Shader::setVec3(const std::string &name,
                     const glm::vec3 &value) const {
  glUniform3fv(glGetUniformLocation(programId, name.c_str()), 1,
               &value[0]);
}

void Shader::setFloat(const std::string &name, float value) const {
  glUniform1f(glGetUniformLocation(programId, name.c_str()), value);
}

void Shader::setInt(const std::string &name, int value) const {
  glUniform1i(glGetUniformLocation(programId, name.c_str()), value);
}
