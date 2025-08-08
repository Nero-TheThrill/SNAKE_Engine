#include "Engine.h"

#include <iosfwd>
#include <sstream>
#include <fstream>
#include "gl.h"



namespace
{
    GLenum ConvertShaderStageToGLenum(ShaderStage stage)
    {
        switch (stage)
        {
        case ShaderStage::Vertex:      return GL_VERTEX_SHADER;
        case ShaderStage::Fragment:    return GL_FRAGMENT_SHADER;
        case ShaderStage::Geometry:    return GL_GEOMETRY_SHADER;
        case ShaderStage::TessControl: return GL_TESS_CONTROL_SHADER;
        case ShaderStage::TessEval:    return GL_TESS_EVALUATION_SHADER;
        case ShaderStage::Compute:     return GL_COMPUTE_SHADER;
        }
        return 0; // invalid
    }
    std::string ShaderStageToString(ShaderStage stage)
    {
        switch (stage)
        {
        case ShaderStage::Vertex:      return "Vertex";
        case ShaderStage::Fragment:    return "Fragment";
        case ShaderStage::Geometry:    return "Geometry";
        case ShaderStage::TessControl: return "Tessellation Control";
        case ShaderStage::TessEval:    return "Tessellation Eval";
        case ShaderStage::Compute:     return "Compute";
        }
        return "Unknown";
    }
}
Shader::Shader() : programID(0), isSupportInstancing(false)
{
    programID = glCreateProgram();
}

Shader::~Shader()
{
    for (GLuint shader : attachedShaders)
        glDeleteShader(shader);
    glDeleteProgram(programID);
}

void Shader::AttachFromFile(ShaderStage stage, const FilePath& path)
{
    std::string src = LoadShaderSource(path);
    GLuint shader = CompileShader(stage, src);
    glAttachShader(programID, shader);
    attachedShaders.push_back(shader);
    attachedStages.push_back(stage);
}

void Shader::AttachFromSource(ShaderStage stage, const std::string& source)
{
    GLuint shader = CompileShader(stage, source);
    glAttachShader(programID, shader);
    attachedShaders.push_back(shader);
    attachedStages.push_back(stage);
}


void Shader::Link()
{
    bool hasTCS = false;
    bool hasTES = false;

    for (ShaderStage stage : attachedStages)
    {
        if (stage == ShaderStage::TessControl)  hasTCS = true;
        if (stage == ShaderStage::TessEval)     hasTES = true;
    }

    if (hasTCS != hasTES)
    {
        SNAKE_ERR("[Shader] Tessellation shaders must come in pairs (TCS + TES).");
        return;
    }
    glLinkProgram(programID);

    GLint success;
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[1024];
        glGetProgramInfoLog(programID, 1024, nullptr, infoLog);
        SNAKE_ERR("Shader program link error:\n" << infoLog);
    }

    CheckSupportsInstancing();

    for (GLuint shader : attachedShaders)
    {
        glDetachShader(programID, shader);
    }
}

void Shader::Use() const
{
    glUseProgram(programID);
}

void Shader::Unuse() const
{
    glUseProgram(0);
}

void Shader::SendUniform(const std::string& name, int value) const
{
    GLint location = glGetUniformLocation(programID, name.c_str());
    if (location == -1)
    {
        SNAKE_WRN("[Shader] Uniform not found: " << name);
        return;
    }
    glUniform1i(location, value);
}

void Shader::SendUniform(const std::string& name, float value) const
{
    GLint location = glGetUniformLocation(programID, name.c_str());
    if (location == -1)
    {
        SNAKE_WRN("[Shader] Uniform not found: " << name);
        return;
    }
    glUniform1f(location, value);
}

void Shader::SendUniform(const std::string& name, const glm::vec2& value) const
{
    GLint location = glGetUniformLocation(programID, name.c_str());
    if (location == -1)
    {
        SNAKE_WRN("Uniform not found: " << name);
        return;
    }

    glUniform2fv(location, 1, &value[0]);
}

void Shader::SendUniform(const std::string& name, const glm::vec3& value) const
{
    GLint location = glGetUniformLocation(programID, name.c_str());
    if (location == -1)
    {
        SNAKE_WRN("Uniform not found: " << name);
        return;
    }

    glUniform3fv(location, 1, &value[0]);
}

void Shader::SendUniform(const std::string& name, const glm::vec4& value) const
{
    GLint location = glGetUniformLocation(programID, name.c_str());
    if (location == -1)
    {
        SNAKE_WRN("Uniform not found: " << name);
        return;
    }

    glUniform4fv(location, 1, &value[0]);
}

void Shader::SendUniform(const std::string& name, const glm::mat4& value) const
{
    GLint location = glGetUniformLocation(programID, name.c_str());
    if (location == -1)
    {
        SNAKE_WRN("Uniform not found: " << name);
        return;
    }

    glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
}

bool Shader::SupportsInstancing() const
{
    return isSupportInstancing;
}


void Shader::CheckSupportsInstancing()
{
    GLint loc = glGetAttribLocation(programID, "i_Model");
    isSupportInstancing = loc != -1;
}

std::string Shader::LoadShaderSource(const FilePath& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        SNAKE_ERR("[Shader] Failed to open shader file: " << filepath);
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint Shader::CompileShader(ShaderStage stage, const std::string& source)
{
    GLenum glStage = ConvertShaderStageToGLenum(stage);
    GLuint shader = glCreateShader(glStage);

    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        SNAKE_ERR("[Shader] Compilation failed (" << ShaderStageToString(stage) << "):\n" << infoLog);
    }

    return shader;
}
