#pragma once
#include <string>

using FilePath = std::string;

enum class TextureFilter
{
    Nearest,
    Linear,
    NearestMipmapNearest,
    LinearMipmapNearest,
    NearestMipmapLinear,
    LinearMipmapLinear
};

enum class TextureWrap
{
    ClampToEdge,
    Repeat,
    MirroredRepeat,
    ClampToBorder
};

struct TextureSettings
{
    TextureFilter minFilter = TextureFilter::Linear;
    TextureFilter magFilter = TextureFilter::Linear;
    TextureWrap wrapS = TextureWrap::ClampToEdge;
    TextureWrap wrapT = TextureWrap::ClampToEdge;
    bool generateMipmap = true;
};

/**
 * @brief Represents a 2D texture loaded from file or raw image data.
 *
 * @details
 * The Texture class encapsulates an OpenGL texture object, storing its dimensions,
 * format, and OpenGL-assigned ID. It supports configurable filtering, wrapping,
 * and mipmap generation via the TextureSettings struct.
 *
 * Textures can be loaded either from image files or directly from memory.
 * Once created, textures can be bound to specific texture units for use in shaders.
 *
 * @note Texture instances are managed internally by RenderManager and used in Material bindings.
 */
class Texture
{
    friend class Material;
    friend class RenderManager;
public:
    /**
     * @brief Loads a texture from an image file.
     *
     * @details
     * Creates an OpenGL texture by loading pixel data from the specified image file path.
     * Applies filtering and wrapping options as defined in the settings.
     * If enabled, generates mipmaps automatically.
     *
     * @param path File path to the image (e.g., PNG, JPG).
     * @param settings Texture creation options such as filter, wrap, mipmap (default: linear + clamp + mipmap).
     *
     * @note Supported formats depend on the underlying image loader (e.g., stb_image).
     *
     * @code
     * Texture tex("assets/player.png", { TextureFilter::Linear, TextureFilter::Linear, TextureWrap::Repeat });
     * @endcode
     */
    Texture(const FilePath& path, const TextureSettings& settings = {});

    /**
     * @brief Creates a texture from raw image data in memory.
     *
     * @details
     * Initializes an OpenGL texture using raw pixel data provided by the user.
     * Useful for runtime-generated images or fonts. The data must match the specified dimensions and channels.
     *
     * @param data Pointer to raw image pixel data.
     * @param width_ Image width in pixels.
     * @param height_ Image height in pixels.
     * @param channels_ Number of color channels (e.g., 3 = RGB, 4 = RGBA).
     * @param settings Texture filtering and wrapping options.
     *
     * @note The pixel data must remain valid during texture creation.
     */
    Texture(const unsigned char* data, int width_, int height_, int channels_, const TextureSettings& settings = {});

    /**
     * @brief Destroys the OpenGL texture and frees GPU memory.
     */
    ~Texture();

    /**
     * @brief Returns the width of the texture in pixels.
     *
     * @return Texture width in pixels.
     */
    [[nodiscard]] int GetWidth() const { return width; }

    /**
     * @brief Returns the height of the texture in pixels.
     *
     * @return Texture height in pixels.
     */
    [[nodiscard]] int GetHeight() const { return height; }

    /**
     * @brief Returns the OpenGL texture ID.
     *
     * @return GLuint texture object ID.
     */
    [[nodiscard]] unsigned int GetID() const { return id; }

private:
    /**
     * @brief Binds this texture to the specified OpenGL texture unit.
     *
     * @details
     * Used internally by the rendering system to activate this texture for shader use.
     * This binds the texture to GL_TEXTURE0 + unit.
     *
     * @param unit Texture unit index (e.g., 0 = GL_TEXTURE0).
     *
     * @note Internal use only. Not intended for external users.
     */
    void BindToUnit(unsigned int unit) const;

    /**
     * @brief Unbinds the texture from the specified OpenGL texture unit.
     *
     * @details
     * Restores the texture unit to a default (zero) binding.
     * Typically used for cleanup after draw calls.
     *
     * @param unit Texture unit index to unbind.
     *
     * @note Internal use only.
     */
    void UnBind(unsigned int unit) const;

    /**
     * @brief Generates and uploads the OpenGL texture from pixel data.
     *
     * @details
     * Called internally by the constructor to configure texture parameters,
     * upload pixel data to the GPU, and optionally generate mipmaps.
     *
     * @param data Pointer to raw image data.
     * @param settings Filtering, wrapping, and mipmap settings.
     *
     * @note Internal helper used only during texture initialization.
     */
    void GenerateTexture(const unsigned char* data, const TextureSettings& settings);
    unsigned int id;
    int width, height, channels;
};
