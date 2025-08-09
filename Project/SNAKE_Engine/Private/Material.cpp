#include "Engine.h"

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
            mesh->SetupInstanceAttributes();
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

bool Material::HasTexture(Texture* texture) const
{
    for (const auto& pair : textures)
    {
        if (pair.second == texture)
        {
            return true;
        }
    }
    return false;
}

bool Material::HasShader(Shader* shader_) const
{
    return shader == shader_;
}
