#include "RenderManager.h"
#include <algorithm>
#include "ext/matrix_clip_space.hpp"
#include "ext/matrix_transform.hpp"

#include "Camera2D.h"
#include "gl.h"

#include "Debug.h"
#include"Object.h"
#include "Material.h"
#include "InstanceBatchKey.h"
#include "EngineContext.h"
#include "GameState.h"
#include "TextObject.h"
#include "WindowManager.h"


void RenderManager::Submit(std::function<void()>&& drawFunc)
{
    renderQueue.push_back({ std::move(drawFunc) });
}

void RenderManager::Submit(const EngineContext& engineContext, const std::vector<Object*>& objects, Camera2D* camera)
{
    std::vector<Object*> visibleObjects;
    if (camera)
    {
        FrustumCuller::CullVisible(*camera, objects, visibleObjects, glm::vec2(camera->GetScreenWidth(), camera->GetScreenHeight()));
        BuildRenderMap(visibleObjects, camera);
    }
    else
    {
        BuildRenderMap(objects, camera);
    }
}

void FrustumCuller::CullVisible(const Camera2D& camera, const std::vector<Object*>& allObjects,
    std::vector<Object*>& outVisibleList, glm::vec2 viewportSize)
{
    outVisibleList.clear();
    for (Object* obj : allObjects)
    {
        if (!obj->IsAlive() || !obj->IsVisible())
            continue;
        if (obj->ShouldIgnoreCamera())
        {
            outVisibleList.push_back(obj);
            continue;
        }
        const glm::vec2& pos = obj->GetWorldPosition();
        float radius = obj->GetBoundingRadius();

        if (camera.IsInView(pos, radius, viewportSize))
            outVisibleList.push_back(obj);
    }
}

void RenderManager::FlushDrawCommands(const EngineContext& engineContext)
{
    SubmitRenderMap(engineContext);
    for (const auto& cmd : renderQueue)
    {
        cmd();
    }
    for (auto& shdrMap : renderMap)
    {
        shdrMap.clear();
    }
    renderQueue.clear();
}

void RenderManager::SetViewport(int x, int y, int width, int height)
{
    Submit([=]() {
        glViewport(x, y, width, height);
        });
}

void RenderManager::ClearBackground(int x, int y, int width, int height, glm::vec4 color)
{
    Submit([=]() {
        glEnable(GL_SCISSOR_TEST);
        glScissor(x, y, width, height);
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_SCISSOR_TEST);
        });
}

void RenderManager::DrawDebugLine(const glm::vec2& from, const glm::vec2& to, Camera2D* camera, const glm::vec4& color, float lineWidth)
{
    debugLineMap[{camera, lineWidth}].push_back({ from, to, color, lineWidth });
}
void RenderManager::FlushDebugLineDrawCommands(const EngineContext& engineContext)
{
    if (debugLineMap.empty())
        return;

    if (!debugLineShader)
    {
        debugLineShader = GetShaderByTag("internal_debug_line");
        if (!debugLineShader)
        {
            SNAKE_ERR("Missing internal_debug_line shader");
            return;
        }
    }

    debugLineShader->Use();

    for (const auto& [camWidth, lines] : debugLineMap)
    {
        Camera2D* camera = camWidth.first;
        float lineWidth = camWidth.second;

        glLineWidth(lineWidth);
        glm::mat4 view = camera
            ? camera->GetViewMatrix()
            : glm::mat4(1);
        glm::mat4 proj = glm::ortho(
            -static_cast<float>(engineContext.windowManager->GetWidth()) / 2,
            static_cast<float>(engineContext.windowManager->GetWidth()) / 2,
            -static_cast<float>(engineContext.windowManager->GetHeight()) / 2,
            static_cast<float>(engineContext.windowManager->GetHeight()) / 2
        );

        debugLineShader->SendUniform("u_View", view);
        debugLineShader->SendUniform("u_Projection", proj);

        std::vector<float> vertexData;
        vertexData.reserve(lines.size() * 12);

        for (const auto& line : lines)
        {
            vertexData.insert(vertexData.end(), {
                line.from.x, line.from.y, line.color.r, line.color.g, line.color.b, line.color.a,
                line.to.x,   line.to.y,   line.color.r, line.color.g, line.color.b, line.color.a
                });
        }

        glBindBuffer(GL_ARRAY_BUFFER, debugLineVBO);
        glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_DYNAMIC_DRAW);

        glBindVertexArray(debugLineVAO);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lines.size() * 2));
        glBindVertexArray(0);
    }

    glLineWidth(1.0f);
    debugLineShader->Unuse();
    debugLineMap.clear();
}


RenderLayerManager& RenderManager::GetRenderLayerManager()
{
    return renderLayerManager;
}

void RenderManager::Init(const EngineContext& engineContext)
{
    auto shader = std::make_unique<Shader>();

    shader->AttachFromSource(ShaderStage::Vertex, R"(
		#version 330 core
		layout (location = 0) in vec2 aPos;
		layout (location = 1) in vec2 aUV;

		uniform mat4 u_Model;
		uniform mat4 u_View;
		uniform mat4 u_Projection;

		out vec2 v_TexCoord;

		void main()
		{
		    v_TexCoord = aUV;
		    gl_Position = u_Projection * u_View * u_Model * vec4(aPos, 0.0, 1.0);
		}
    )");
    shader->AttachFromSource(ShaderStage::Fragment, R"(
	        #version 330 core
	        in vec2 v_TexCoord;
	        out vec4 FragColor;

	        uniform sampler2D u_FontTexture;
	        uniform vec4 u_Color;

	        void main()
	        {
	            float alpha = texture(u_FontTexture, v_TexCoord).r;
	            FragColor = vec4(u_Color.rgb, alpha * u_Color.a);
	        }
    )");

    shader->Link();
    shaderMap["internal_text"] = std::move(shader);
    RegisterMaterial("internal_text", "internal_text", {});


    shader = std::make_unique<Shader>();
    shader->AttachFromSource(ShaderStage::Vertex, R"(
                #version 330 core
                layout (location = 0) in vec2 aPos;
                layout (location = 1) in vec4 aColor;

				uniform mat4 u_View;
                uniform mat4 u_Projection;
                out vec4 vColor;

                void main()
                {
                    vColor = aColor;
                    gl_Position = u_Projection * u_View * vec4(aPos, 0.0, 1.0);
                }
    )");
    shader->AttachFromSource(ShaderStage::Fragment, R"(
                #version 330 core
                in vec4 vColor;
                out vec4 FragColor;

                void main()
                {
                    FragColor = vColor;
                }
    )");
    shader->Link();

    shaderMap["internal_debug_line"] = std::move(shader);



    glGenVertexArrays(1, &debugLineVAO);
    glGenBuffers(1, &debugLineVBO);

    glBindVertexArray(debugLineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, debugLineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 10000, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0); // vec2 position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);

    glEnableVertexAttribArray(1); // vec4 color
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 2));

    glBindVertexArray(0);



    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void RenderManager::BuildRenderMap(const std::vector<Object*>& source, Camera2D* camera)
{
    for (auto* obj : source)
    {
        if (!obj || !obj->IsVisible())
            continue;

        Material* material = obj->GetMaterial();
        Mesh* mesh = obj->GetMesh();
        Shader* shader = material ? material->GetShader() : nullptr;

        if (!material || !mesh || !shader)
            continue;

        uint8_t layer = obj->GetRenderLayer();
        if (layer >= RenderLayerManager::MAX_LAYERS)
        {
            SNAKE_WRN("render skipped - invalid layer\n");
            continue;
        }

        InstanceBatchKey key{ mesh, material };
        renderMap[layer][shader][key].emplace_back(obj, camera);
    }
}

void RenderManager::SubmitRenderMap(const EngineContext& engineContext)
{
    Material* lastMaterial = nullptr;
    Shader* lastShader = nullptr;

    for (uint8_t layer = 0; layer < renderMap.size(); ++layer)
    {
        const ShaderMap& _shaderMap = renderMap[layer];

        for (const auto& [shader, batchMap] : _shaderMap)
        {
            for (const auto& [key, batch] : batchMap)
            {
                if (batch.front().first->CanBeInstanced())
                {
                    Submit([=]() mutable {
                        std::vector<glm::mat4> transforms;
                        std::vector<glm::vec4> colors;
                        std::vector<glm::vec2> uvOffsets;
                        std::vector<glm::vec2> uvScales;
                        transforms.reserve(batch.size());
                        colors.reserve(batch.size());
                        uvOffsets.reserve(batch.size());
                        uvScales.reserve(batch.size());
                        for (const auto& [obj, camera] : batch)
                        {
                            glm::mat4 model = obj->GetTransform2DMatrix();
                            glm::vec2 flip = obj->GetUVFlipVector();
                            model = model * glm::scale(glm::mat4(1.0f), glm::vec3(flip, 1.0f));
                            transforms.push_back(model);

                            colors.push_back(obj->GetColor());
                            if (obj->HasAnimation())
                            {
                                uvOffsets.push_back(obj->GetAnimator()->GetUVOffset());
                                uvScales.push_back(obj->GetAnimator()->GetUVScale());
                            }
                            else
                            {
                                uvOffsets.emplace_back(0.0f, 0.0f);
                                uvScales.emplace_back(1.0f, 1.0f);
                            }
                        }

                        Material* material = key.material;
                        Shader* currentShader = material->GetShader();

                        if (material != lastMaterial)
                        {
                            material->Bind();
                            lastMaterial = material;
                        }

                        if (currentShader != lastShader)
                        {
                            glm::mat4 projection;
                            glm::mat4 view;

                            if (batch.front().first->ShouldIgnoreCamera())
                                view = glm::mat4(1);
                            else
                                view = batch.front().second->GetViewMatrix();


                            if (batch.front().second)
                            {
                                projection = glm::ortho(
                                    -static_cast<float>(batch.front().second->GetScreenWidth()) / 2,
                                    static_cast<float>(batch.front().second->GetScreenWidth()) / 2,
                                    -static_cast<float>(batch.front().second->GetScreenHeight()) / 2,
                                    static_cast<float>(batch.front().second->GetScreenHeight()) / 2
                                );
                            }
                            else
                            {
                                projection = glm::ortho(
                                    -static_cast<float>(engineContext.windowManager->GetWidth()) / 2,
                                    static_cast<float>(engineContext.windowManager->GetWidth()) / 2,
                                    -static_cast<float>(engineContext.windowManager->GetHeight()) / 2,
                                    static_cast<float>(engineContext.windowManager->GetHeight()) / 2
                                );
                            }
                            material->SetUniform("u_View", view);
                            material->SetUniform("u_Projection", projection);

                            glm::mat4 model = batch.front().first->GetTransform2DMatrix();
                            glm::vec2 flip = batch.front().first->GetUVFlipVector();
                            model = model * glm::scale(glm::mat4(1.0f), glm::vec3(flip, 1.0f));

                            material->SetUniform("u_Model", model);
                            material->SetUniform("u_Color", batch.front().first->GetColor());
                            if (batch.front().first->HasAnimation())
                            {
                                material->SetTexture("u_Texture", batch.front().first->GetAnimator()->GetTexture());
                            }
                            lastShader = currentShader;
                        }

                        batch.front().first->Draw(engineContext);
                        material->SendUniforms();

                        key.mesh->BindVAO();
                        material->UpdateInstanceBuffer(transforms, colors, uvOffsets, uvScales);
                        key.mesh->DrawInstanced(static_cast<GLsizei>(transforms.size()));
                        material->UnBind();
                        });
                }
                else
                {
                    for (const auto& [obj, camera] : batch)
                    {

                        Submit([=]() mutable {
                            Material* mat = key.material;
                            Shader* currentShader = mat->GetShader();

                            if (mat != lastMaterial)
                            {
                                mat->Bind();
                                lastMaterial = mat;
                            }

                            if (currentShader != lastShader)
                            {
                                glm::mat4 projection;
                                glm::mat4 view;

                                if (obj->ShouldIgnoreCamera())
                                    view = glm::mat4(1);
                                else
                                    view = camera->GetViewMatrix();


                                if (camera)
                                {
                                    projection = glm::ortho(
                                        -static_cast<float>(camera->GetScreenWidth()) / 2,
                                        static_cast<float>(camera->GetScreenWidth()) / 2,
                                        -static_cast<float>(camera->GetScreenHeight()) / 2,
                                        static_cast<float>(camera->GetScreenHeight()) / 2
                                    );
                                }
                                else
                                {
                                    projection = glm::ortho(
                                        -static_cast<float>(engineContext.windowManager->GetWidth()) / 2,
                                        static_cast<float>(engineContext.windowManager->GetWidth()) / 2,
                                        -static_cast<float>(engineContext.windowManager->GetHeight()) / 2,
                                        static_cast<float>(engineContext.windowManager->GetHeight()) / 2
                                    );
                                }
                                mat->SetUniform("u_View", view);
                                mat->SetUniform("u_Projection", projection);

                                glm::mat4 model = obj->GetTransform2DMatrix();
                                glm::vec2 flip = obj->GetUVFlipVector();
                                model = model * glm::scale(glm::mat4(1.0f), glm::vec3(flip, 1.0f));

                                mat->SetUniform("u_Model", model);
                                mat->SetUniform("u_Color", obj->GetColor());

                                lastShader = currentShader;
                            }

                            if (obj->HasAnimation())
                            {
                                SpriteAnimator* anim = obj->GetAnimator();
                                mat->SetUniform("u_UVOffset", anim->GetUVOffset());
                                mat->SetUniform("u_UVScale", anim->GetUVScale());
                                mat->SetTexture("u_Texture", anim->GetTexture());
                            }

                            obj->Draw(engineContext);
                            mat->SendUniforms();
                            key.mesh->Draw();
                            mat->UnBind();
                            });
                    }
                }
            }
        }
    }
}

/*
 * Usage:
 * renderManager.RegisterShader("basic", {
 * { ShaderStage::Vertex, "shaders/basic.vert" },
 * { ShaderStage::Fragment, "shaders/basic.frag" }
 * });
 */
void RenderManager::RegisterShader(const std::string& tag, const std::vector<std::pair<ShaderStage, FilePath>>& sources)
{
    if (shaderMap.find(tag) != shaderMap.end())
    {
        SNAKE_WRN("Shader with tag \"" << tag << "\" already registered.");
        return;
    }
    auto shader = std::make_unique<Shader>();

    for (const auto& [stage, path] : sources)
        shader->AttachFromFile(stage, path);

    shader->Link();
    shaderMap[tag] = std::move(shader);
}

void RenderManager::RegisterShader(const std::string& tag, std::unique_ptr<Shader> shader)
{
    if (shaderMap.find(tag) != shaderMap.end())
    {
        SNAKE_WRN("Shader with tag \"" << tag << "\" already registered.");
        return;
    }
    shaderMap[tag] = std::move(shader);
}

void RenderManager::RegisterTexture(const std::string& tag, const FilePath& path, const TextureSettings& settings)
{
    if (textureMap.find(tag) != textureMap.end())
    {
        SNAKE_WRN("Texture with tag \"" << tag << "\" already registered.");
        return;
    }
    textureMap[tag] = std::make_unique<Texture>(path, settings);
}

void RenderManager::RegisterTexture(const std::string& tag, std::unique_ptr<Texture> texture)
{
    if (textureMap.find(tag) != textureMap.end())
    {
        SNAKE_WRN("Texture with tag \"" << tag << "\" already registered.");
        return;
    }
    textureMap[tag] = std::move(texture);
}

void RenderManager::RegisterMesh(const std::string& tag, const std::vector<Vertex>& vertices,
    const std::vector<unsigned int>& indices, PrimitiveType primitiveType)
{
    if (meshMap.find(tag) != meshMap.end())
    {
        SNAKE_WRN("Mesh with tag \"" << tag << "\" already registered.");
        return;
    }
    meshMap[tag] = std::make_unique<Mesh>(vertices, indices, primitiveType);
}

void RenderManager::RegisterMesh(const std::string& tag, std::unique_ptr<Mesh> mesh)
{
    if (meshMap.find(tag) != meshMap.end())
    {
        SNAKE_WRN("Mesh with tag \"" << tag << "\" already registered.");
        return;
    }
    meshMap[tag] = std::move(mesh);
}

void RenderManager::RegisterMaterial(const std::string& tag, const std::string& shaderTag,
    const std::unordered_map<UniformName, TextureTag>& textureBindings)
{
    if (materialMap.find(tag) != materialMap.end())
    {
        SNAKE_WRN("Material tag already registered: " << tag);
        return;
    }

    Shader* shader = shaderMap[shaderTag].get();
    if (!shader)
    {
        SNAKE_WRN("Shader not found: " << shaderTag);
        return;
    }

    auto material = std::make_unique<Material>(shader);

    for (const auto& [uniformName, textureTag] : textureBindings)
    {
        auto it = textureMap.find(textureTag);
        if (it != textureMap.end())
            material->SetTexture(uniformName, it->second.get());
        else
            SNAKE_WRN("Texture not found: " << textureTag);
    }

    materialMap[tag] = std::move(material);
}

void RenderManager::RegisterMaterial(const std::string& tag, std::unique_ptr<Material> material)
{
    if (materialMap.find(tag) != materialMap.end())
    {
        SNAKE_WRN("Material tag already registered: " << tag);
        return;
    }
    materialMap[tag] = std::move(material);
}

void RenderManager::RegisterFont(const std::string& tag, const std::string& ttfPath, uint32_t pixelSize)
{
    if (fontMap.find(tag) != fontMap.end())
    {
        SNAKE_WRN("Font tag already registered: " << tag);
        return;
    }
    const uint32_t minSize = 4;
    const uint32_t maxSize = 64;

    if (pixelSize < minSize || pixelSize > maxSize)
    {
        SNAKE_ERR("Font pixelSize out of bounds: " << pixelSize << " (allowed: " << minSize << " - " << maxSize << ")");
        return;
    }

    auto font = std::make_unique<Font>(*this, ttfPath, pixelSize);

    fontMap[tag] = std::move(font);
}

void RenderManager::RegisterFont(const std::string& tag, std::unique_ptr<Font> font)
{
    if (fontMap.find(tag) != fontMap.end())
    {
        SNAKE_WRN("Font tag already registered: " << tag);
        return;
    }
    fontMap[tag] = std::move(font);
}

void RenderManager::RegisterRenderLayer(const std::string& tag)
{
    renderLayerManager.RegisterLayer(tag);
}

void RenderManager::RegisterSpriteSheet(const std::string& tag, const std::string& textureTag, int frameW, int frameH)
{
    if (spritesheetMap.find(tag) != spritesheetMap.end())
    {
        SNAKE_WRN("SpriteSheet already registered: " << tag);
        return;
    }

    Texture* texture = GetTextureByTag(textureTag);
    if (!texture)
    {
        SNAKE_ERR("Texture not found for SpriteSheet: " << textureTag);
        return;
    }

    spritesheetMap[tag] = std::make_unique<SpriteSheet>(texture, frameW, frameH);
}

SpriteSheet* RenderManager::GetSpriteSheetByTag(const std::string& tag)
{
    if (spritesheetMap.find(tag) != spritesheetMap.end())
        return spritesheetMap[tag].get();
    else
    {
        SNAKE_ERR("There is no SpriteSheet named '" << tag << "'");
        return nullptr;
    }
}

Shader* RenderManager::GetShaderByTag(const std::string& tag)
{
    if (shaderMap.find(tag) != shaderMap.end())
        return shaderMap[tag].get();
    else
    {
        SNAKE_ERR("There is no Shader named '" << tag << "'");
        return nullptr;
    }
}

Texture* RenderManager::GetTextureByTag(const std::string& tag)
{
    if (textureMap.find(tag) != textureMap.end())
        return textureMap[tag].get();
    else
    {
        SNAKE_ERR("There is no Texture named '" << tag << "'");
        return nullptr;
    }
}

Mesh* RenderManager::GetMeshByTag(const std::string& tag)
{
    if (meshMap.find(tag) != meshMap.end())
        return meshMap[tag].get();
    else
    {
        SNAKE_ERR("There is no Mesh named '" << tag << "'");
        return nullptr;
    }
}

Material* RenderManager::GetMaterialByTag(const std::string& tag)
{
    if (materialMap.find(tag) != materialMap.end())
        return materialMap[tag].get();
    else
    {
        SNAKE_ERR("There is no Material named '" << tag << "'");
        return nullptr;
    }
}

Font* RenderManager::GetFontByTag(const std::string& tag)
{
    if (fontMap.find(tag) != fontMap.end())
        return fontMap[tag].get();
    else
    {
        SNAKE_ERR("There is no Font named '" << tag << "'");
        return nullptr;
    }
}
