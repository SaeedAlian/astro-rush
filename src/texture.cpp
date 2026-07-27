#include "texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>

std::unordered_map<std::string, std::unique_ptr<Texture>>
    Texture::cache;

Texture *Texture::load(const std::string &path) {
  auto it = cache.find(path);

  if (it != cache.end()) {
    return it->second.get();
  }

  int width, height, channels;
  stbi_set_flip_vertically_on_load(true);

  unsigned char *data =
      stbi_load(path.c_str(), &width, &height, &channels, 0);

  if (!data) {
    std::cerr << "[Texture] failed to load: " << path << "\n";
    return nullptr;
  }

  GLenum format = GL_RGB;
  if (channels == 1)
    format = GL_RED;
  else if (channels == 3)
    format = GL_RGB;
  else if (channels == 4)
    format = GL_RGBA;

  unsigned int id;
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
               GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  stbi_image_free(data);

  auto tex = std::unique_ptr<Texture>(new Texture(id));
  Texture *ptr = tex.get();
  cache.emplace(path, std::move(tex));
  return ptr;
}

Texture *Texture::loadCubemap(const std::vector<std::string> &faces) {
  std::string cacheKey;
  for (const auto &f : faces) {
    cacheKey += f + "|---|";
  }

  auto it = cache.find(cacheKey);
  if (it != cache.end()) {
    return it->second.get();
  }

  unsigned int id;

  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_CUBE_MAP, id);

  stbi_set_flip_vertically_on_load(false);

  for (unsigned int i = 0; i < faces.size(); ++i) {
    int width, height, channels;
    unsigned char *data =
        stbi_load(faces[i].c_str(), &width, &height, &channels, 0);

    if (!data) {
      std::cerr << "[Texture] failed to load cubemap face: "
                << faces[i] << "\n";
      continue;
    }

    GLenum format = GL_RGB;
    if (channels == 1)
      format = GL_RED;
    else if (channels == 3)
      format = GL_RGB;
    else if (channels == 4)
      format = GL_RGBA;

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width,
                 height, 0, format, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER,
                  GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,
                  GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,
                  GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,
                  GL_CLAMP_TO_EDGE);

  auto tex = std::unique_ptr<Texture>(new Texture(id, true));
  Texture *ptr = tex.get();
  cache.emplace(cacheKey, std::move(tex));
  return ptr;
}

void Texture::bind(unsigned int unit) const {
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::bindCubemap(unsigned int unit) const {
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}
