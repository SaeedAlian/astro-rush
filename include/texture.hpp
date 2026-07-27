#pragma once

#include <GL/glew.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

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
   * Loads a cubemap texture from disk and creates an GL texture
   * object.
   *
   * Gets the face paths, and loads them via stb_image library,
   * then binds them to GL.
   *
   * // clang-format off
   * Order of the faces matters:
   *     GL_TEXTURE_CUBE_MAP_POSITIVE_X..NEGATIVE_Z expects
   *         +X, -X, +Y, -Y, +Z, -Z.
   *
   *         +X = right,  -X = left
   *         +Y = up,     -Y = down
   *         +Z = back,   -Z = front (or vice versa)
   *
   * // clang-format on
   *
   * @param faces | paths of the textures of the 6 faces
   *
   * @return Texture object instance
   */
  static Texture *loadCubemap(const std::vector<std::string> &faces);

  /*
   * Selects the texture unit for rendering.
   *
   * @param unit | texture unit index (for example, 0 corresponds to
   * GL_TEXTURE0)
   */
  void bind(unsigned int unit) const;

  /*
   * Selects the cubemap texture unit for rendering.
   *
   * @param unit | texture unit index (for example, 0 corresponds to
   * GL_TEXTURE0)
   */
  void bindCubemap(unsigned int unit) const;

  /*
   * @return texture object id
   */
  unsigned int getId() const { return id; }

private:
  unsigned int id = 0;
  bool isCubemap = false;

  /**
   * Creates a texture wrapper from an existing GL texture id.
   *
   * @param id | GL texture object id
   * @param isCubemap | is the texture a cube map
   */
  explicit Texture(unsigned int id, bool isCubemap = false)
      : id(id), isCubemap(isCubemap) {}

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
