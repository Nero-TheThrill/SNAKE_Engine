#pragma once
#include <string>
#include <unordered_map>
#include <variant>
#include "glm.hpp"

class RenderManager;
class ObjectManager;
class Shader;
class Texture;
class Mesh;

using GLuint = unsigned int;
using UniformValue = std::variant<
    int,
    float,
    glm::vec2,
    glm::vec3,
    glm::vec4,
    glm::mat4
>;

class Material {
    friend RenderManager;

public:
    Material(Shader* _shader) : shader(_shader), isInstancingEnabled(false){}

    void SetTexture(const std::string& uniformName, Texture* texture)
    {
        textures[uniformName] = texture;
    }

    void SetUniform(const std::string& name, UniformValue value)
    {
        uniforms[name] = value;
    }

    [[nodiscard]] bool IsInstancingSupported() const;

    void EnableInstancing(bool enable, Mesh* mesh);

private:
    void Bind() const;

    void UnBind() const;

    void SendUniforms();

    bool HasTexture() const { return !textures.empty(); }

    [[nodiscard]] Shader* GetShader() const { return shader; }

    Shader* shader;
    std::unordered_map<std::string, Texture*> textures;
    std::unordered_map<std::string, UniformValue> uniforms;


    bool isInstancingEnabled;
};
