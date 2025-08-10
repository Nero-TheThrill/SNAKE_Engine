#pragma once

#include <string>
#include <vector>
#include "glm.hpp"

enum class ShaderStage
{
    Vertex,
    Fragment,
    Geometry,
    TessControl,
    TessEval,
    Compute
};

class RenderManager;
class Material;

using GLuint = unsigned int;
using GLint = int;
using GLenum = unsigned int;
using FilePath = std::string;

/**
 * @brief OpenGL shader program wrapper with uniform helpers and instancing support query.
 *
 * @details
 * - Constructor creates a program (glCreateProgram). Destructor deletes attached shaders and the program.
 * - AttachFromFile/AttachFromSource compile a stage and attach it to the program.
 * - Link() links the program; enforces TCS/TES pairing; then checks instancing support via attribute "i_Model".
 * - SendUniform*() resolves a uniform location each call; logs if not found; then uploads the value.
 * - Material calls Use()/Unuse() and SendUniform*() through friendship; RenderManager may also access as a friend.
 */
class Shader {
    friend Material;
    friend RenderManager;

public:
    /**
     * @brief Creates an empty program object.
     * @note Program ID is created via glCreateProgram. No shaders are attached yet.
     */
    Shader();

    /**
     * @brief Releases attached shaders and deletes the program.
     */
    ~Shader();

    /**
     * @brief Sends an integer uniform.
     * @param name Uniform name.
     * @param value Integer value (glUniform1i).
     * @note Logs if the uniform is not found.
     */
    void SendUniform(const std::string& name, int value) const;

    /**
     * @brief Sends a float uniform.
     * @param name Uniform name.
     * @param value Float value (glUniform1f).
     * @note Logs if the uniform is not found.
     */
    void SendUniform(const std::string& name, float value) const;

    /**
     * @brief Sends a vec2 uniform.
     * @param name Uniform name.
     * @param value glm::vec2 (glUniform2fv).
     * @note Logs if the uniform is not found.
     */
    void SendUniform(const std::string& name, const glm::vec2& value) const;

    /**
     * @brief Sends a vec3 uniform.
     * @param name Uniform name.
     * @param value glm::vec3 (glUniform3fv).
     * @note Logs if the uniform is not found.
     */
    void SendUniform(const std::string& name, const glm::vec3& value) const;

    /**
     * @brief Sends a vec4 uniform.
     * @param name Uniform name.
     * @param value glm::vec4 (glUniform4fv).
     * @note Logs if the uniform is not found.
     */
    void SendUniform(const std::string& name, const glm::vec4& value) const;

    /**
     * @brief Sends a mat4 uniform.
     * @param name Uniform name.
     * @param value glm::mat4 (glUniformMatrix4fv, no transpose).
     * @note Logs if the uniform is not found.
     */
    void SendUniform(const std::string& name, const glm::mat4& value) const;

    /**
     * @brief Returns the linked program object ID.
     */
    [[nodiscard]] GLuint GetProgramID() const { return programID; }

private:
    /**
     * @brief Makes this program current (glUseProgram).
     * @note Internal. Used by Material when binding.
     */
    void Use() const;

    /**
     * @brief Unbinds any program (glUseProgram(0)).
     * @note Internal. Used by Material when unbinding.
     */
    void Unuse() const;

    /**
     * @brief Reports whether the program supports instancing.
     *
     * @details
     * Determined after a successful Link() by querying the attribute
     * location of "i_Model" (see CheckSupportsInstancing()).
     *
     * @return true if attribute "i_Model" exists; false otherwise.
     */
    [[nodiscard]] bool SupportsInstancing() const;

    /**
     * @brief Links the attached shader stages into a program.
     *
     * @details
     * - Fails if only one of TessControl/TessEval is attached.
     * - On success: calls CheckSupportsInstancing() and detaches all attached shaders.
     *
     * @return true on success; false if link or validation fails.
     */
    bool Link();

    /**
     * @brief Compiles and attaches a shader stage from a file.
     * @param stage Shader stage to attach.
     * @param filepath Source file path.
     * @return true on success; false if load/compile fails.
     */
    bool AttachFromFile(ShaderStage stage, const FilePath& filepath);

    /**
     * @brief Compiles and attaches a shader stage from a string source.
     * @param stage Shader stage to attach.
     * @param source GLSL source code.
     * @return true on success; false if compile fails.
     */
    bool AttachFromSource(ShaderStage stage, const std::string& source);

    /**
     * @brief Loads a text file into a string.
     * @param filepath Path to the shader source file.
     * @param success Output flag set to GL_TRUE on success, GL_FALSE on failure.
     * @return File contents as a single string (empty on failure).
     * @note Internal. Used by AttachFromFile().
     */
    [[nodiscard]] std::string LoadShaderSource(const FilePath& filepath, GLint& success);

    /**
     * @brief Compiles a GLSL shader of the given stage.
     * @param stage Shader stage.
     * @param source GLSL source text.
     * @param success Output flag set via glGetShaderiv(..., GL_COMPILE_STATUS).
     * @return The created shader object (valid even on failure; check @p success).
     * @note Internal. Used by AttachFromFile/AttachFromSource().
     */
    [[nodiscard]] GLuint CompileShader(ShaderStage stage, const std::string& source, GLint& success);

    /**
     * @brief Checks for per-instance attribute "i_Model" and caches support.
     * @note Internal. Called by Link() after a successful link.
     */
    void CheckSupportsInstancing();

    GLuint programID;                          ///< GL program object.
    std::vector<GLuint> attachedShaders;       ///< Attached shader ids (detached after Link()).
    std::vector<ShaderStage> attachedStages;   ///< Stages attached to this program.

    bool isSupportInstancing;                  ///< Cached result of CheckSupportsInstancing().
};
