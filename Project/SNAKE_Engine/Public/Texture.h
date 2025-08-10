#pragma once
#include <string>

using FilePath = std::string;

/**
 * @brief Minification filter options for textures.
 * @details
 * Mapped to GL constants via internal helpers (ConvertMinFilter).
 */
enum class TextureMinFilter
{
    Nearest,
    Linear,
    NearestMipmapNearest,
    LinearMipmapNearest,
    NearestMipmapLinear,
    LinearMipmapLinear
};

/**
 * @brief Magnification filter options for textures.
 * @details
 * Mapped to GL constants via internal helpers (ConvertMagFilter).
 */
enum class TextureMagFilter
{
    Nearest,
    Linear,
};

/**
 * @brief Texture wrap modes for S and T axes.
 * @details
 * Mapped to GL constants via internal helpers (ConvertWrap).
 */
enum class TextureWrap
{
    ClampToEdge,
    Repeat,
    MirroredRepeat,
    ClampToBorder
};

/**
 * @brief Runtime-configurable texture parameters with sensible defaults.
 *
 * @details
 * Defaults:
 * - minFilter = Linear
 * - magFilter = Linear
 * - wrapS = ClampToEdge
 * - wrapT = ClampToEdge
 * - generateMipmap = true (triggers glGenerateTextureMipmap in GenerateTexture)
 */
struct TextureSettings
{
    TextureMinFilter minFilter = TextureMinFilter::Linear;
    TextureMagFilter magFilter = TextureMagFilter::Linear;
    TextureWrap wrapS = TextureWrap::ClampToEdge;
    TextureWrap wrapT = TextureWrap::ClampToEdge;
    bool generateMipmap = true;
};

class Texture
{
    friend class Material;
    friend class RenderManager;
public:
    /**
     * @brief Loads an image from disk and creates a GPU texture.
     *
     * @details
     * - Uses stb_image with vertical flip enabled.
     * - On success, calls GenerateTexture(data, settings) and frees the image data.
     * - On failure, logs an error and leaves id == 0, width/height/channels == 0.
     *
     * @param path File path to the image.
     * @param settings Texture parameters applied during creation (filters, wraps, mipmap).
     * @code
     * TextureSettings settings;
     * settings.minFilter = TextureMinFilter::LinearMipmapLinear;
     * settings.magFilter = TextureMagFilter::Linear;
     * settings.wrapS = TextureWrap::Repeat;
     * settings.wrapT = TextureWrap::Repeat;
     * settings.generateMipmap = true;
     *
     * Texture tex("assets/brick.png", settings);
     * @endcode
     */
    Texture(const FilePath& path, const TextureSettings& settings = {});

    /**
     * @brief Creates a GPU texture from raw pixel data.
     *
     * @details
     * Width/height/channels are taken as-is; data is uploaded in GenerateTexture().
     *
     * @param data Pointer to pixel bytes.
     * @param width_ Image width in pixels.
     * @param height_ Image height in pixels.
     * @param channels_ Number of channels (1, 3, or 4 handled explicitly).
     * @param settings Texture parameters applied during creation.
     */
    Texture(const unsigned char* data, int width_, int height_, int channels_, const TextureSettings& settings = {});

    /**
     * @brief Destroys the GPU texture if created.
     * @details Calls glDeleteTextures(1, &id) when id != 0.
     */
    ~Texture();

    /**
     * @brief Returns the image width in pixels.
     */
    [[nodiscard]] int GetWidth() const { return width; }

    /**
     * @brief Returns the image height in pixels.
     */
    [[nodiscard]] int GetHeight() const { return height; }

    /**
     * @brief Returns the OpenGL texture object id (0 if not created).
     */
    [[nodiscard]] unsigned int GetID() const { return id; }

private:
    /**
     * @brief Binds this texture to a texture unit.
     * @details Internal. Used by Material when sending uniforms.
     * @param unit Texture unit index.
     */
    void BindToUnit(unsigned int unit) const;

    /**
     * @brief Unbinds any texture from a texture unit (binds 0).
     * @details Internal. Used by Material when cleaning up.
     * @param unit Texture unit index.
     */
    void UnBind(unsigned int unit) const;

    /**
     * @brief Allocates and uploads texture storage/data, then sets parameters.
     *
     * @details
     * - Derives internal/pixel formats from channel count (1->R8/RED, 3->RGB8/RGB, 4->RGBA8/RGBA).
     * - Uses DSA: glCreateTextures / glTextureStorage2D / glTextureSubImage2D.
     * - Sets min/mag filter and wrap S/T from TextureSettings.
     * - If generateMipmap is true, calls glGenerateTextureMipmap.
     *
     * @param data Pixel source (unsigned byte).
     * @param settings Texture parameters to apply.
     */
    void GenerateTexture(const unsigned char* data, const TextureSettings& settings);

    unsigned int id;
    int width, height, channels;
};
