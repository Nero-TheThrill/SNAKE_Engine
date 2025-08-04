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
using GLenum = unsigned int;
using FilePath = std::string;

/**
 * @brief Represents an OpenGL shader program composed of one or more stages.
 *
 * @details
 * The Shader class manages the lifecycle of an OpenGL program, including attaching
 * shader stages (e.g., vertex, fragment), compiling source code, and linking them
 * into a usable GPU program.
 *
 * It provides uniform upload functions and tracks whether the shader supports
 * hardware instancing based on its input layout.
 *
 * Shaders are typically registered through RenderManager and bound automatically
 * during material usage.
 *
 * @note Shader instances are owned and used by RenderManager and Material. Users typically
 * do not create or bind shaders directly in game code.
 */
class Shader {
    friend Material;
    friend RenderManager;

public:
    /**
     * @brief Constructs an empty shader program.
     *
     * @details
     * Initializes a new OpenGL shader program ID. Shader stages must be
     * attached and linked before it can be used for rendering.
     */
    Shader();

    /**
     * @brief Destroys the shader program and frees GPU resources.
     */
    ~Shader();

    /**
     * @brief Sends an integer uniform to the shader.
     *
     * @param name The name of the uniform variable in the shader.
     * @param value The integer value to set.
     */
    void SendUniform(const std::string& name, int value) const;

    /**
     * @brief Sends a float uniform to the shader.
     *
     * @param name The name of the uniform variable in the shader.
     * @param value The float value to set.
     */
    void SendUniform(const std::string& name, float value) const;

    /**
     * @brief Sends a vec2 uniform to the shader.
     *
     * @param name The name of the uniform variable in the shader.
     * @param value The 2D vector value to set.
     */
    void SendUniform(const std::string& name, const glm::vec2& value) const;

    /**
     * @brief Sends a vec3 uniform to the shader.
     *
     * @param name The name of the uniform variable in the shader.
     * @param value The 3D vector value to set.
     */
    void SendUniform(const std::string& name, const glm::vec3& value) const;

    /**
     * @brief Sends a vec4 uniform to the shader.
     *
     * @param name The name of the uniform variable in the shader.
     * @param value The 4D vector value to set.
     */
    void SendUniform(const std::string& name, const glm::vec4& value) const;

    /**
     * @brief Sends a 4x4 matrix uniform to the shader.
     *
     * @param name The name of the uniform variable in the shader.
     * @param value The mat4 matrix value to set.
     */
    void SendUniform(const std::string& name, const glm::mat4& value) const;

    /**
     * @brief Returns the OpenGL program ID associated with this shader.
     *
     * @return OpenGL shader program ID.
     */
    [[nodiscard]] GLuint GetProgramID() const { return programID; }

private:
    /**
     * @brief Binds this shader program for use in rendering.
     *
     * @details
     * Makes the shader active in the OpenGL pipeline. Called internally by Material or RenderManager.
     *
     * @note This is an internal function and should not be called manually by users.
     */
    void Use() const;

    /**
     * @brief Unbinds the current shader program.
     *
     * @details
     * Deactivates any active shader in the OpenGL pipeline by binding 0.
     * Used after rendering is complete.
     *
     * @note Internal use only.
     */
    void Unuse() const;

    /**
     * @brief Checks if the shader supports hardware instancing.
     *
     * @details
     * Determined during shader linking based on input layout and attribute semantics.
     * This flag is used by RenderManager to optimize draw calls.
     *
     * @return true if instancing is supported, false otherwise.
     */
    [[nodiscard]] bool SupportsInstancing() const;

    /**
     * @brief Links all attached shader stages into a complete OpenGL program.
     *
     * @details
     * After attaching all required shader stages, this function performs
     * the actual linking of the program. It also validates the result and
     * sets instancing support status.
     *
     * @note Must be called after attaching all stages.
     */
    void Link();

    /**
     * @brief Loads and attaches a shader stage from a source file.
     *
     * @details
     * Reads the shader source code from the specified file path, compiles it,
     * and attaches it to this shader program under the given stage type.
     *
     * @param stage The shader stage to attach (e.g., Vertex, Fragment).
     * @param filepath Path to the shader source file.
     *
     * @note This is used internally by RenderManager during shader registration.
     */
    void AttachFromFile(ShaderStage stage, const FilePath& filepath);

    /**
     * @brief Compiles and attaches shader source code provided as a string.
     *
     * @details
     * Used when shader source is embedded directly in code or generated at runtime.
     *
     * @param stage The shader stage to attach.
     * @param source GLSL source code string.
     */
    void AttachFromSource(ShaderStage stage, const std::string& source);

    /**
     * @brief Loads shader source code from a file into a string.
     *
     * @details
     * Opens the specified file and reads its entire contents into a std::string.
     * Used by AttachFromFile() during shader registration.
     *
     * @param filepath Path to the shader source file.
     * @return The shader source code as a string.
     */
    [[nodiscard]] std::string LoadShaderSource(const FilePath& filepath);

    /**
     * @brief Compiles GLSL source code into a shader object for a given stage.
     *
     * @details
     * Uses OpenGL to compile the provided GLSL code into a shader object.
     * Returns the OpenGL handle of the compiled shader.
     *
     * @param stage The shader stage (e.g., Vertex, Fragment).
     * @param source GLSL shader source code.
     * @return The compiled shader object handle.
     */
    [[nodiscard]] GLuint CompileShader(ShaderStage stage, const std::string& source);

    /**
     * @brief Analyzes the shader to determine if it supports hardware instancing.
     *
     * @details
     * This is called after linking to scan active attributes and check for
     * instancing-compatible input layout.
     *
     * @note Used internally to optimize rendering in RenderManager.
     */
    void CheckSupportsInstancing();

    GLuint programID;
    std::vector<GLuint> attachedShaders;
    std::vector<ShaderStage> attachedStages;

    bool isSupportInstancing;
};
