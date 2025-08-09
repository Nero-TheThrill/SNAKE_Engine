#include "Engine.h"
#include "gl.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//used anonymous namespace to hide these functions from other files

static GLint ConvertMinFilter(TextureMinFilter filter)
{
    switch (filter)
    {
    case TextureMinFilter::Nearest:              return GL_NEAREST;
    case TextureMinFilter::Linear:               return GL_LINEAR;
    case TextureMinFilter::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
    case TextureMinFilter::LinearMipmapNearest:  return GL_LINEAR_MIPMAP_NEAREST;
    case TextureMinFilter::NearestMipmapLinear:  return GL_NEAREST_MIPMAP_LINEAR;
    case TextureMinFilter::LinearMipmapLinear:   return GL_LINEAR_MIPMAP_LINEAR;
    }
    return GL_LINEAR;
}

static GLint ConvertMagFilter(TextureMagFilter filter)
{
    switch (filter)
    {
    case TextureMagFilter::Nearest:              return GL_NEAREST;
    case TextureMagFilter::Linear:               return GL_LINEAR;
    }
    return GL_LINEAR;
}

static GLint ConvertWrap(TextureWrap wrap)
{
    switch (wrap)
    {
    case TextureWrap::ClampToEdge:    return GL_CLAMP_TO_EDGE;
    case TextureWrap::Repeat:         return GL_REPEAT;
    case TextureWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
    case TextureWrap::ClampToBorder:  return GL_CLAMP_TO_BORDER;
    }
    return GL_CLAMP_TO_EDGE;
}


Texture::Texture(const std::string& path, const TextureSettings& settings) :id(0), width(0), height(0), channels(0)
{
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (!data)
    {
        SNAKE_ERR("Failed to load texture: " << path);
        return;
    }
    GenerateTexture(data, settings);
    stbi_image_free(data);
}

Texture::Texture(const unsigned char* data, int width_, int height_, int channels_, const TextureSettings& settings)
{
    width = width_;
    height = height_;
    channels = channels_;

    GenerateTexture(data, settings);
}

Texture::~Texture()
{
    if (id != 0)
    {
        glDeleteTextures(1, &id);
    }
}

void Texture::BindToUnit(unsigned int unit) const
{
    glBindTextureUnit(unit, id);
}

void Texture::UnBind(unsigned int unit) const
{
    glBindTextureUnit(unit, 0);
}

void Texture::GenerateTexture(const unsigned char* data, const TextureSettings& settings)
{
    GLenum internalFormat = GL_RGBA8;
    GLenum pixelFormat = GL_RGBA;

    if (channels == 1)
    {
        internalFormat = GL_R8;
        pixelFormat = GL_RED;
    }
    else if (channels == 3)
    {
        internalFormat = GL_RGB8;
        pixelFormat = GL_RGB;
    }
    else if (channels == 4)
    {
        internalFormat = GL_RGBA8;
        pixelFormat = GL_RGBA;
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &id);
    glTextureStorage2D(id, settings.generateMipmap? 1 + floor(log2(std::max(width, height))) : 1, internalFormat, width, height);
    glTextureSubImage2D(id, 0, 0, 0, width, height, pixelFormat, GL_UNSIGNED_BYTE, data);

    glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, ConvertMinFilter(settings.minFilter));
    glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, ConvertMagFilter(settings.magFilter));
    glTextureParameteri(id, GL_TEXTURE_WRAP_S, ConvertWrap(settings.wrapS));
    glTextureParameteri(id, GL_TEXTURE_WRAP_T, ConvertWrap(settings.wrapT));

    if (settings.generateMipmap)
    {
        glGenerateTextureMipmap(id);
    }
}
