#include "glad/gl.h"
#include "Material.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "Debug.h"

void Material::Bind() const
{
    shader->Use();
}

void Material::UnBind() const
{
    int unit = 0;
    for (const auto& [uniformName, tex] : textures)
    {
        if (!tex) continue;
        tex->UnBind(unit);
        unit++;
    }
    shader->Unuse();
}

void Material::UpdateInstanceBuffer(const std::vector<glm::mat4>& transforms) const
{
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(glm::mat4), transforms.data(), GL_DYNAMIC_DRAW);    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Material::UpdateInstanceBuffer(const std::vector<glm::vec4>& vec4s) const
{
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO[1]);
    glBufferData(GL_ARRAY_BUFFER, vec4s.size() * sizeof(glm::vec4), vec4s.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

bool Material::IsInstancingSupported() const
{
    return isInstancingEnabled && shader && shader->SupportsInstancing();
}
void Material::EnableInstancing(bool enable, Mesh* mesh)
{
    if (shader && !shader->SupportsInstancing())
    {
        SNAKE_WRN("Enable Instancing skipped: Tried enable instancing, but shader does not support 'i_Model'.");
        return;
    }
    if (!isInstancingEnabled)
    {
        isInstancingEnabled = enable;
        if (mesh)
        {
            if (!instanceVBO[0])
                glGenBuffers(2, instanceVBO);
            mesh->SetupInstanceAttributes(instanceVBO);
        }
    }
}

void Material::SendUniforms()
{
    int unit = 0;
    for (const auto& [uniformName, tex] : textures)
    {
        if (!tex) continue;
        tex->BindToUnit(unit);
        shader->SendUniform(uniformName, unit);
        unit++;
    }

    for (const auto& [name, value] : uniforms)
    {
        std::visit([&](auto&& val)
            {
                shader->SendUniform(name, val);
            }, value);
    }
}
