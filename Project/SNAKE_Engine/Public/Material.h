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

/**
 * @brief Represents a GPU-bound material that links a shader, textures, and uniforms.
 *
 * @details
 * The Material class encapsulates how an object should appear when rendered.
 * It manages a reference to a Shader, associated textures (by uniform name),
 * and arbitrary uniform values (int, float, vecX, mat4, etc.).
 *
 * Materials can optionally enable hardware instancing, in which case instance
 * data such as transforms and per-instance uniforms are sent via VBOs.
 *
 * @note Materials are usually created and managed by the RenderManager,
 * and assigned to renderable objects such as Mesh or Object.
 */
class Material {
    friend RenderManager;

public:
    /**
     * @brief Constructs a material using the given shader.
     *
     * @details
     * Initializes the material with a reference to the specified Shader.
     * Textures and uniform values can be added later using SetTexture() and SetUniform().
     * Instancing is disabled by default.
     *
     * @param _shader Pointer to the shader program used by this material.
     */
    Material(Shader* _shader) : shader(_shader), isInstancingEnabled(false), instanceVBO(){}

    /**
     * @brief Associates a texture with a uniform sampler name.
     *
     * @details
     * Stores a pointer to the given Texture under the specified uniform name (e.g., "u_MainTex").
     * This mapping is maintained in the internal texture map and used when the material is bound.
     * Textures are automatically assigned to texture units in order during Bind().
     *
     * This function does not immediately bind the texture to the GPU;
     * it only records the mapping for later use.
     *
     * @param uniformName The name of the sampler2D uniform in the shader.
     * @param texture Pointer to the Texture object to associate.
     *
     * @code
     * material.SetTexture("u_MainTex", texture);
     * @endcode
     */
    void SetTexture(const std::string& uniformName, Texture* texture)
    {
        textures[uniformName] = texture;
    }

    /**
     * @brief Stores a uniform value for later use during rendering.
     *
     * @details
     * Adds or updates a uniform value in the internal `uniforms` map, indexed by the uniform's name.
     * The value is stored as a std::variant that supports int, float, vec2, vec3, vec4, and mat4.
     *
     * Uniforms are not sent immediately to the GPU. Instead, they are batched and
     * submitted all at once in SendUniforms() during the material Bind() call.
     *
     * @param name The name of the uniform variable in the shader.
     * @param value The value to associate with the uniform (int, float, vec2, etc.).
     *
     * @code
     * material.SetUniform("u_Tint", glm::vec4(1.0f, 0, 0, 1.0f));
     * @endcode
     */
    void SetUniform(const std::string& name, UniformValue value)
    {
        uniforms[name] = value;
    }

    /**
     * @brief Checks whether the associated shader supports hardware instancing.
     *
     * @details
     * Returns true if the shader was analyzed and confirmed to accept instanced input.
     * Used to determine whether instancing can be enabled for this material.
     *
     * @return true if instancing is supported, false otherwise.
     */
    [[nodiscard]] bool IsInstancingSupported() const;

    /**
     * @brief Enables or disables instancing for this material.
     *
     * @details
     * If instancing is enabled and the shader supports it, the mesh's VAO is updated
     * with additional instance attributes, and internal VBOs are initialized.
     *
     * @param enable Whether to enable (true) or disable (false) instancing.
     * @param mesh The mesh to configure for instancing (VAO will be modified).
     */
    void EnableInstancing(bool enable, Mesh* mesh);

private:
    /**
     * @brief Binds the shader, textures, and uniforms for rendering.
     *
     * @details
     * Activates the shader program, binds all registered textures to appropriate texture units,
     * and sends stored uniform values to the GPU.
     *
     * @note This function is called internally before drawing an object.
     */
    void Bind() const;

    /**
     * @brief Unbinds any textures or state used by this material.
     *
     * @details
     * Restores the OpenGL state to avoid leaking bindings between draw calls.
     */
    void UnBind() const;

    /**
     * @brief Sends all stored uniform values to the GPU via the shader.
     *
     * @details
     * Iterates through the internal `uniforms` map and dispatches each value to the shader
     * using Shader::SendUniform(), based on its type (int, float, vec2, etc.).
     *
     * This function is automatically called during `Bind()` and is not intended
     * to be invoked directly by users.
     *
     * @note If a uniform name is invalid or not used by the shader, the shader may silently ignore it.
     */
    void SendUniforms();

    /**
     * @brief Uploads per-instance data (transforms, colors, UVs) to the GPU.
     *
     * @details
     * Sends transform matrices, per-instance colors, and UV offsets/scales
     * to the appropriate instance buffer objects (VBOs).
     *
     * @param transforms World transform matrices for each instance.
     * @param colors Color tint per instance.
     * @param uvOffsets UV offset per instance.
     * @param uvScales UV scale per instance.
     */
    void UpdateInstanceBuffer(const std::vector<glm::mat4>& transforms, const std::vector<glm::vec4>& colors, const std::vector<glm::vec2>& uvOffsets, const std::vector<glm::vec2>& uvScales) const;

    /**
     * @brief Returns the shader associated with this material.
     *
     * @return Pointer to the assigned Shader object.
     */
    [[nodiscard]] Shader* GetShader() const { return shader; }

    Shader* shader;
    std::unordered_map<std::string, Texture*> textures;
    std::unordered_map<std::string, UniformValue> uniforms;

    GLuint instanceVBO[4];
    bool isInstancingEnabled;
};
