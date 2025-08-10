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
/**
 * @brief Variant of supported uniform value types.
 * @details Supported: int, float, vec2, vec3, vec4, mat4.
 */
using UniformValue = std::variant<
    int,
    float,
    glm::vec2,
    glm::vec3,
    glm::vec4,
    glm::mat4
>;

/**
 * @brief Material binds a Shader with named textures and cached uniform values.
 *
 * @details
 * - Textures are associated by uniform name (e.g., "u_Texture").
 * - Uniform values are staged via SetUniform() and sent to the GPU on SendUniforms().
 * - RenderManager calls Bind()/SendUniforms()/UnBind() during submission; users typically
 *   only call SetTexture()/SetUniform().
 * - Instancing: IsInstancingSupported() is true only if instancing is enabled on this
 *   material and the underlying shader reports support.
 */
class Material {
    friend RenderManager;

public:
    /**
     * @brief Construct with a shader.
     * @param _shader Non-owning pointer to a linked shader program.
     */
    Material(Shader* _shader) : shader(_shader), isInstancingEnabled(false) {}

    /**
     * @brief Associate a texture with a sampler uniform name.
     * @details
     * Textures are bound to units sequentially starting at 0 inside SendUniforms(), and
     * the sampler uniform @p uniformName is set to the matching unit index.
     * @param uniformName GLSL sampler uniform name (e.g., "u_Texture").
     * @param texture Texture pointer (non-owning).
     */
    void SetTexture(const std::string& uniformName, Texture* texture)
    {
        textures[uniformName] = texture;
    }

    /**
     * @brief Stage a uniform value to be uploaded on the next SendUniforms().
     * @param name Uniform name.
     * @param value One of: int, float, vec2, vec3, vec4, mat4.
     */
    void SetUniform(const std::string& name, UniformValue value)
    {
        uniforms[name] = value;
    }

    /**
     * @brief Returns true if this material can be drawn with instancing.
     * @details Equivalent to (isInstancingEnabled && shader && shader->SupportsInstancing()).
     */
    [[nodiscard]] bool IsInstancingSupported() const;

    /**
     * @brief Enable/disable instancing for this material and prepare the mesh if enabled.
     * @details
     * - If @p mesh or shader is null, logs a warning and does nothing.
     * - When enabled, calls mesh->SetupInstanceAttributes().
     * @param enable Whether to enable instanced rendering for this material.
     * @param mesh Target mesh to prepare instance attributes on.
     */
    void EnableInstancing(bool enable, Mesh* mesh);

private:
    /**
     * @brief Bind the shader program for subsequent uniform/texture updates.
     * @note Internal. Called by the renderer.
     */
    void Bind() const;

    /**
     * @brief Unbind textures previously bound by SendUniforms().
     * @note Internal. Called by the renderer to clean up texture units.
     */
    void UnBind() const;

    /**
     * @brief Upload all staged uniforms and bind textures to texture units.
     * @details
     * - Binds textures in sequence (units 0,1,...) and sets the corresponding sampler uniforms.
     * - Sends each staged uniform variant via Shader::SendUniform().
     * @note Internal. Called by the renderer after Object::Draw() has set per-object uniforms.
     */
    void SendUniforms();

    /** @brief Returns true if any texture has been set on this material. */
    bool HasTexture() const { return !textures.empty(); }

    /**
     * @brief Returns true if @p texture is one of the bound textures.
     * @details Checks pointer equality against internal bindings.
     */
    bool HasTexture(Texture* texture) const;

    /**
     * @brief Returns true if this material uses @p shader_ (pointer equality).
     */
    bool HasShader(Shader* shader_) const;

    /** @brief Access the underlying shader (non-owning). */
    [[nodiscard]] Shader* GetShader() const { return shader; }

    Shader* shader;
    std::unordered_map<std::string, Texture*> textures;             ///< Sampler name -> Texture*.
    std::unordered_map<std::string, UniformValue> uniforms;         ///< Staged uniforms by name.

    bool isInstancingEnabled;                                       ///< Local flag toggled via EnableInstancing().
};
