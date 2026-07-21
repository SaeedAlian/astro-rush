#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

class Shader {
public:
  Shader(const char *vertexPath, const char *fragmentPath);
  ~Shader();

  void use() const;

  void setMat4(const std::string &name, const glm::mat4 &value) const;
  void setVec3(const std::string &name, const glm::vec3 &value) const;
  void setFloat(const std::string &name, float value) const;
  void setInt(const std::string &name, int value) const;

  unsigned int getId() const { return programId; }

private:
  unsigned int programId = 0;

  static std::string readFile(const char *path);
  static unsigned int compile(unsigned int type,
                              const std::string &source,
                              const char *debugName);
  void checkLinkErrors() const;
};
