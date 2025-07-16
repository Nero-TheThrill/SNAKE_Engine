#pragma once
#include <string>
#include <unordered_map>
#include <variant>
#include "../ThirdParty/glm/glm.hpp"

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
/**
 * @brief Represents a GPU-bound material used to render objects.
 *
 * @details
 * A Material encapsulates a Shader, a set of bound textures, and uniform values.
 * It also optionally supports GPU instancing using a per-instance matrix buffer.
 *
 * - Textures are set using SetTexture().
 * - Uniforms (int, float, vectors, matrices) are set using SetUniform().
 * - Call EnableInstancing() to enable per-instance rendering.
 *
 * Used internally by ObjectManager and RenderManager to prepare draw submissions.
 */
class Material {
    friend ObjectManager;

public:
    /**
     * @brief Constructs a material using the given shader.
     *
     * @param _shader Pointer to the shader program this material uses.
     */
    Material(Shader* _shader) : shader(_shader), instanceVBO{ 0 }, isInstancingEnabled(false) {}

    /**
     * @brief Binds a texture to a named uniform slot in the shader.
     *
     * @details
     * This method registers a texture to be used for a given uniform name.
     * The texture is not immediately bound to the GPU; instead, it is recorded
     * internally and later sent to the shader via SendUniforms().
     *
     * The uniform name must match a sampler2D (or similar) declared in the shader.
     *
     * @param uniformName The name of the sampler uniform in the shader.
     * @param texture Pointer to the Texture to be associated with this uniform.
     *
     * @code
     * material->SetTexture("u_DiffuseMap", texture);
     * @endcode
     */
    void SetTexture(const std::string& uniformName, Texture* texture)
    {
        textures[uniformName] = texture;
    }

    /**
     * @brief Stores a uniform value to be sent to the shader during rendering.
     *
     * @details
     * This method does not immediately send the value to the GPU.
     * Instead, it stores the value in an internal map, and the uniform is sent
     * later via SendUniforms(), typically right before drawing.
     *
     * Supported types include:
     * - int
     * - float
     * - glm::vec2
     * - glm::vec3
     * - glm::vec4
     * - glm::mat4
     *
     * @param name Name of the uniform variable in the shader.
     * @param value The value to be sent (as a variant).
     *
     * @code
     * material->SetUniform("u_Color", glm::vec4(1.0, 0.0, 0.0, 1.0));
     * @endcode
     */
    void SetUniform(const std::string& name, UniformValue value)
    {
        uniforms[name] = value;
    }

    /**
     * @brief Checks whether the material supports GPU instancing.
     *
     * @return True if the bound shader supports instancing via `i_Model` attribute.
     */
    [[nodiscard]] bool IsInstancingSupported() const;

    /**
     * @brief Enables instanced rendering if supported by the shader.
     *
     * @details
     * If the shader does not contain an `i_Model` attribute, this call is ignored with a warning.
     * Otherwise, generates an instance VBO and binds it to the mesh to support per-instance transform data.
     *
     * @param enable True to enable instancing.
     * @param mesh The mesh that will use this material for instanced drawing.
     */
    void EnableInstancing(bool enable, Mesh* mesh);

private:
    /**
     * @brief Binds this material's shader for use.
     *
     * @details
     * Activates the associated shader via glUseProgram().
     * Note: Does not bind textures or uniforms-that is handled separately by SendUniforms().
     */
    void Bind() const;

    /**
     * @brief Unbinds all textures and the shader associated with this material.
     *
     * @details
     * - Unbinds each texture from its assigned texture unit.
     * - Calls glUseProgram(0) to deactivate the shader.
     */
    void UnBind() const;

    /**
     * @brief Sends all textures and uniform values to the GPU.
     *
     * @details
     * - Each texture is bound to a unique texture unit and sent to the shader.
     * - All stored uniform values are passed via the shader's SendUniform() methods.
     *
     * Must be called after Bind().
     */
    void SendUniforms();

    /**
     * @brief Uploads instance model matrices to the GPU.
     *
     * @details
     * Binds the instance VBO and uploads the given transform matrices using glBufferData().
     * Assumes that the instance buffer layout was already set up via SetupInstanceAttributes().
     *
     * @param transforms Array of model matrices for each instance.
     */
    void UpdateInstanceBuffer(const std::vector<glm::mat4>& transforms) const;
    void UpdateInstanceBuffer(const std::vector<glm::vec4>& vec4s) const;

    /**
     * @brief Returns the shader used by this material.
     *
     * @return Pointer to the Shader.
     */
    [[nodiscard]] Shader* GetShader() const { return shader; }

    Shader* shader;
    std::unordered_map<std::string, Texture*> textures;
    std::unordered_map<std::string, UniformValue> uniforms;

    GLuint instanceVBO[2];
    bool isInstancingEnabled;
};