#include "Engine.h"
#include "gl.h"

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
                glGenBuffers(4, instanceVBO);
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

void Material::UpdateInstanceBuffer(const std::vector<glm::mat4>& transforms, const std::vector<glm::vec4>& colors, const std::vector<glm::vec2>& uvOffsets, const std::vector<glm::vec2>& uvScales) const
{
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(glm::mat4), transforms.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO[1]);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec4), colors.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO[2]);
    glBufferData(GL_ARRAY_BUFFER, uvOffsets.size() * sizeof(glm::vec2), uvOffsets.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO[3]);
    glBufferData(GL_ARRAY_BUFFER, uvScales.size() * sizeof(glm::vec2), uvScales.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
