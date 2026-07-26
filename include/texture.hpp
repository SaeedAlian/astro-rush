#pragma once

#include <GL/glew.h>

#include <memory>
#include <string>
#include <unordered_map>

/*
 * Wrapper around an OpenGL texture object.
 *
 * A Texture represents an image loaded into GPU memory and provides
 * functions to bind it for rendering.
 *
 * Textures are cached by their file path to avoid loading the same
 * image multiple times. Calling load() with an already loaded path
 * returns the existing texture instance.
 */
class Texture {
public:
  /*
   * Loads a texture from disk and creates an GL texture object.
   * Gets a texture file path, and loads it via stb_image library,
   * then binds it to GL.
   *
   * @param path | texture file path
   *
   * @return Texture object instance
   */
  static Texture *load(const std::string &path);

  /*
   * Select the texture unit for rendering
   *
   * @param unit | texture unit index (for example, 0 corresponds to
   * GL_TEXTURE0)
   */
  void bind(unsigned int unit) const;

  /*
   * @return texture object id
   */
  unsigned int getId() const { return id; }

private:
  unsigned int id = 0;

  /**
   * Creates a texture wrapper from an existing GL texture id.
   *
   * @param id | GL texture object id
   */
  explicit Texture(unsigned int id) : id(id) {}

  /*
   * Cache of loaded textures.
   *
   * Maps texture file paths to their corresponding Texture objects.
   * unique_ptr ensures that each cached texture has a single owner
   * and is automatically destroyed when removed from the cache.
   */
  static std::unordered_map<std::string, std::unique_ptr<Texture>>
      cache;
};
