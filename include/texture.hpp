#pragma once

#include <GL/glew.h>

#include <memory>
#include <string>
#include <unordered_map>

class Texture {
public:
  static Texture *load(const std::string &path);

  void bind(unsigned int unit) const;

  unsigned int getId() const { return id; }

private:
  unsigned int id = 0;

  explicit Texture(unsigned int id) : id(id) {}

  static std::unordered_map<std::string, std::unique_ptr<Texture>>
      cache;
};
