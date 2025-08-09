#include "Engine.h"

#include <algorithm>
#include "ext/matrix_clip_space.hpp"
#include "gl.h"

void RenderManager::Submit(const std::vector<Object*>& objects, const EngineContext& engineContext)
{
    std::vector<Object*> visibleObjects;
    Camera2D* camera = engineContext.stateManager->GetCurrentState()->GetActiveCamera();
    if (camera)
    {
        FrustumCuller::CullVisible(*camera, objects, visibleObjects, glm::vec2(camera->GetScreenWidth(), camera->GetScreenHeight()));
        BuildRenderMap(visibleObjects, camera);
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
    Material* lastMaterial = nullptr;


    for (uint8_t layer = 0; layer < renderMap.size(); ++layer)
    {
        const ShaderMap& _shaderMap = renderMap[layer];

        for (const auto& [shader, batchMap] : _shaderMap)
        {
            for (const auto& [key, batch] : batchMap)
            {
                if (batch.front().first->CanBeInstanced())
                {
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
                    if (!material)
                        material = defaultMaterial;
                    if (material != lastMaterial)
                    {
                        if (lastMaterial)
                            lastMaterial->UnBind();
                        material->Bind();
                        lastMaterial = material;
                    }

                    Camera2D* cam = batch.front().second;
                    bool ignoreCam = batch.front().first->ShouldIgnoreCamera();

                    if (!material->HasTexture())
                    {
                        material->SetTexture("u_Texture", errorTexture);
                    }

                    glm::mat4 view = ignoreCam ? glm::mat4(1.0f)
                        : (cam ? cam->GetViewMatrix() : glm::mat4(1.0f));

                    int w = cam ? cam->GetScreenWidth() : engineContext.windowManager->GetWidth();
                    int h = cam ? cam->GetScreenHeight() : engineContext.windowManager->GetHeight();
                    glm::mat4 projection = glm::ortho(-static_cast<float>(w) / 2.0f,
                        static_cast<float>(w) / 2.0f,
                        -static_cast<float>(h) / 2.0f,
                        static_cast<float>(h) / 2.0f);

                    material->SetUniform("u_View", view);
                    material->SetUniform("u_Projection", projection);

                    if (batch.front().first->HasAnimation())
                    {
                        material->SetTexture("u_Texture", batch.front().first->GetAnimator()->GetTexture());
                    }

                    batch.front().first->Draw(engineContext);
                    material->SendUniforms();
                    key.mesh->UpdateInstanceBuffer(transforms, colors, uvOffsets, uvScales);
                    key.mesh->DrawInstanced(static_cast<GLsizei>(transforms.size()));
                }

                else
                {
                    for (const auto& [obj, camera] : batch)
                    {
                        Material* material = key.material;
                        if (!material)
                            material = defaultMaterial;
                        if (material != lastMaterial)
                        {
                            if (lastMaterial)
                                lastMaterial->UnBind();
                            material->Bind();
                            lastMaterial = material;
                        }

                        bool ignoreCam = obj->ShouldIgnoreCamera();
                        Camera2D* cam = camera;

                        if (!material->HasTexture())
                        {
                            material->SetTexture("u_Texture", errorTexture);
                        }

                        glm::mat4 view = ignoreCam ? glm::mat4(1.0f)
                            : (cam ? cam->GetViewMatrix() : glm::mat4(1.0f));

                        int w = cam ? cam->GetScreenWidth() : engineContext.windowManager->GetWidth();
                        int h = cam ? cam->GetScreenHeight() : engineContext.windowManager->GetHeight();
                        glm::mat4 projection = glm::ortho(-static_cast<float>(w) / 2.0f,
                            static_cast<float>(w) / 2.0f,
                            -static_cast<float>(h) / 2.0f,
                            static_cast<float>(h) / 2.0f);

                        material->SetUniform("u_View", view);
                        material->SetUniform("u_Projection", projection);

                        glm::mat4 model = obj->GetTransform2DMatrix();
                        glm::vec2 flip = obj->GetUVFlipVector();
                        model = model * glm::scale(glm::mat4(1.0f), glm::vec3(flip, 1.0f));

                        material->SetUniform("u_Model", model);
                        material->SetUniform("u_Color", obj->GetColor());

                        if (obj->HasAnimation())
                        {
                            SpriteAnimator* anim = obj->GetAnimator();
                            material->SetUniform("u_UVOffset", anim->GetUVOffset());
                            material->SetUniform("u_UVScale", anim->GetUVScale());
                            material->SetTexture("u_Texture", anim->GetTexture());
                        }

                        obj->Draw(engineContext);
                        material->SendUniforms();
                        key.mesh->Draw();
                    }
                }
            }
        }
    }

    if (lastMaterial)
        lastMaterial->UnBind();

    for (auto& shdrMap : renderMap)
    {
        shdrMap.clear();
    }
}

void RenderManager::SetViewport(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
}

void RenderManager::ClearBackground(int x, int y, int width, int height, glm::vec4 color)
{
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y, width, height);
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
}

void RenderManager::DrawDebugLine(const glm::vec2& from, const glm::vec2& to, Camera2D* camera, const glm::vec4& color, float lineWidth)
{
    debugLineMap[{camera, lineWidth}].push_back({ from, to, color, lineWidth });
}
void RenderManager::FlushDebugLineDrawCommands(const EngineContext& engineContext)
{
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
		#version 460 core
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
	        #version 460 core
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
    shaderMap["[EngineShader]internal_text"] = std::move(shader);

    shader = std::make_unique<Shader>();
    shader->AttachFromSource(ShaderStage::Vertex, R"(
                #version 460 core
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
                #version 460 core
                in vec4 vColor;
                out vec4 FragColor;

                void main()
                {
                    FragColor = vColor;
                }
    )");
    shader->Link();

    shaderMap["[EngineShader]internal_debug_line"] = std::move(shader);
    debugLineShader = GetShaderByTag("[EngineShader]internal_debug_line");


    shader = std::make_unique<Shader>();
    shader->AttachFromSource(ShaderStage::Vertex, R"(
		#version 460 core

		layout (location = 0) in vec3 aPos;
		layout(location = 1) in vec2 a_UV;

		uniform mat4 u_Model;
		uniform mat4 u_View;
		uniform mat4 u_Projection;


		void main()
		{
		    gl_Position = u_Projection * u_View * u_Model * vec4(aPos, 1.0);
		}

    )");
    shader->AttachFromSource(ShaderStage::Fragment, R"(
                #version 460 core
	        uniform vec4 u_Color;
                out vec4 FragColor;

                void main()
                {
                    FragColor = u_Color;
                }
    )");
    shader->Link();

    shaderMap["[EngineShader]default"] = std::move(shader);


    std::vector<unsigned char> errorTexturePixels;
    errorTexturePixels.reserve(8 * 8 * 4);

    for (int y = 0; y < 8; ++y)
    {
        for (int x = 0; x < 8; ++x)
        {
            bool isYellow = (x + y) % 2 == 0;
            if (isYellow)
            {
                errorTexturePixels.insert(errorTexturePixels.end(), { 255, 255, 0, 255 });
            }
            else
            {
                errorTexturePixels.insert(errorTexturePixels.end(), { 0, 0, 0, 255 });
            }
        }
    }
    RegisterTexture("[EngineTexture]error", std::make_unique<Texture>(errorTexturePixels.data(), 8, 8, 4, TextureSettings{ TextureMinFilter::Nearest ,TextureMagFilter::Nearest ,TextureWrap::MirroredRepeat,TextureWrap::MirroredRepeat }));
    errorTexture = GetTextureByTag("[EngineTexture]error");




    shader = std::make_unique<Shader>();
    shader->AttachFromSource(ShaderStage::Vertex, R"(
                #version 460 core

                layout (location = 0) in vec3 aPos;
                layout(location = 1) in vec2 a_UV;
                out vec2 v_UV;

                uniform mat4 u_Model;
                uniform mat4 u_View;
                uniform mat4 u_Projection;


                void main()
                {
                    gl_Position = u_Projection * u_View * u_Model * vec4(aPos, 1.0);
                    v_UV = a_UV;
                }
    )");
    shader->AttachFromSource(ShaderStage::Fragment, R"(
                #version 460 core

                out vec4 FragColor;
                in vec2 v_UV;
                uniform vec4 u_Color;
                uniform sampler2D u_ErrorTexture;

                void main()
                {
                    FragColor = texture(u_ErrorTexture, v_UV) * u_Color;
                }
    )");
    shader->Link();

    shaderMap["[EngineShader]default_texture"] = std::move(shader);
    std::unique_ptr<Material> material = std::make_unique<Material>(GetShaderByTag("[EngineShader]default_texture"));
    material->SetTexture("u_ErrorTexture", errorTexture);
    RegisterMaterial("[EngineMaterial]error", std::move(material));
    defaultMaterial = GetMaterialByTag("[EngineMaterial]error");

    RegisterMesh("[EngineMesh]default", std::vector<Vertex>{
        {{-0.5f, -0.5f, 0.f}, { 0.f, 0.f }},
        { { 0.5f, -0.5f, 0.f }, { 1.f, 0.f } },
        { { 0.5f, 0.5f, 0.f }, { 1.f, 1.f } },
        { { -0.5f, 0.5f, 0.f }, { 0.f, 1.f } }
    }, std::vector<unsigned int>{0, 1, 2, 2, 3, 0});
    defaultMesh = GetMeshByTag("[EngineMesh]default");

    RegisterSpriteSheet("[EngineSpriteSheet]default", "[EngineTexture]error", 1, 1);
    defaultSpriteSheet = GetSpriteSheetByTag("[EngineSpriteSheet]default");

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
        SpriteAnimator* spriteAnimator = obj->GetSpriteAnimator();

        SpriteSheet* spritesheet = spriteAnimator ? spriteAnimator->GetSpriteSheet() : nullptr;
        Shader* shader = material ? material->GetShader() : nullptr;

        if (!material || !mesh || !shader)
            continue;

        uint8_t layer = renderLayerManager.GetLayerID(obj->GetRenderLayerTag()).value_or(0);
        if (layer >= RenderLayerManager::MAX_LAYERS)
        {
            SNAKE_WRN("render skipped - invalid layer\n");
            continue;
        }

        InstanceBatchKey key{ mesh, material, spritesheet };
        renderMap[layer][shader][key].emplace_back(obj, camera);
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
        SNAKE_LOG("Shader with tag \"" << tag << "\" already registered.");
        return;
    }
    auto shader = std::make_unique<Shader>();

    for (const auto& [stage, path] : sources)
    {
        if (!shader->AttachFromFile(stage, path))
        {
            SNAKE_ERR("Failed to register shader [" << tag << "].");
            return;
        }
    }

    if (!shader->Link())
    {
        SNAKE_ERR("Failed to register shader [" << tag << "].");
        return;
    }
    shaderMap[tag] = std::move(shader);
}

void RenderManager::RegisterShader(const std::string& tag, std::unique_ptr<Shader> shader)
{
    if (shaderMap.find(tag) != shaderMap.end())
    {
        SNAKE_LOG("Shader with tag \"" << tag << "\" already registered.");
        return;
    }
    shaderMap[tag] = std::move(shader);
}

void RenderManager::RegisterTexture(const std::string& tag, const FilePath& path, const TextureSettings& settings)
{
    if (textureMap.find(tag) != textureMap.end())
    {
        SNAKE_LOG("Texture with tag \"" << tag << "\" already registered.");
        return;
    }
    textureMap[tag] = std::make_unique<Texture>(path, settings);
}

void RenderManager::RegisterTexture(const std::string& tag, std::unique_ptr<Texture> texture)
{
    if (textureMap.find(tag) != textureMap.end())
    {
        SNAKE_LOG("Texture with tag \"" << tag << "\" already registered.");
        return;
    }
    textureMap[tag] = std::move(texture);
}

void RenderManager::RegisterMesh(const std::string& tag, const std::vector<Vertex>& vertices,
    const std::vector<unsigned int>& indices, PrimitiveType primitiveType)
{
    if (meshMap.find(tag) != meshMap.end())
    {
        SNAKE_LOG("Mesh with tag \"" << tag << "\" already registered.");
        return;
    }
    meshMap[tag] = std::make_unique<Mesh>(vertices, indices, primitiveType);
}

void RenderManager::RegisterMesh(const std::string& tag, std::unique_ptr<Mesh> mesh)
{
    if (meshMap.find(tag) != meshMap.end())
    {
        SNAKE_LOG("Mesh with tag \"" << tag << "\" already registered.");
        return;
    }
    meshMap[tag] = std::move(mesh);
}

void RenderManager::RegisterMaterial(const std::string& tag, const std::string& shaderTag,
    const std::unordered_map<UniformName, TextureTag>& textureBindings)
{
    if (materialMap.find(tag) != materialMap.end())
    {
        SNAKE_LOG("Material tag already registered: " << tag);
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
        SNAKE_LOG("Material tag already registered: " << tag);
        return;
    }
    materialMap[tag] = std::move(material);
}

void RenderManager::RegisterFont(const std::string& tag, const std::string& ttfPath, uint32_t pixelSize)
{
    if (fontMap.find(tag) != fontMap.end())
    {
        SNAKE_LOG("Font tag already registered: " << tag);
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
        SNAKE_LOG("Font tag already registered: " << tag);
        return;
    }
    fontMap[tag] = std::move(font);
}

void RenderManager::RegisterRenderLayer(const std::string& tag, uint8_t layer)
{
    renderLayerManager.RegisterLayer(tag, layer);
}

void RenderManager::RegisterSpriteSheet(const std::string& tag, const std::string& textureTag, int frameW, int frameH)
{
    if (spritesheetMap.find(tag) != spritesheetMap.end())
    {
        SNAKE_LOG("SpriteSheet already registered: " << tag);
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

void RenderManager::UnregisterShader(const std::string& tag, const EngineContext& engineContext)
{
    auto it = shaderMap.find(tag);
    if (it == shaderMap.end())
    {
        SNAKE_LOG("Cannot delete the shader [" << tag << "] because it was not found.");
        return;
    }
    Shader* target = it->second.get();
    GameState* gameState = engineContext.stateManager->GetCurrentState();
    if (gameState)
    {
        std::vector<Object*> objects = gameState->GetObjectManager().GetAllRawPtrObjects();
        for (auto obj : objects)
        {
            Material* material = obj->GetMaterial();
            if (material && material->HasShader(target))
            {
                SNAKE_WRN("Cannot delete the shader [" << tag << "] while there are objects referencing it.");
                return;
            }
        }
        shaderMap.erase(tag);
    }
}

void RenderManager::UnregisterTexture(const std::string& tag, const EngineContext& engineContext)
{
    auto it = textureMap.find(tag);
    if (it == textureMap.end())
    {
        SNAKE_LOG("Cannot delete the texture [" << tag << "] because it was not found.");
        return;
    }
    Texture* target = it->second.get();
    GameState* gameState = engineContext.stateManager->GetCurrentState();
    if (gameState)
    {
        std::vector<Object*> objects = gameState->GetObjectManager().GetAllRawPtrObjects();
        for (auto obj : objects)
        {
            Material* material = obj->GetMaterial();
            if (material && material->HasTexture(target))
            {
                SNAKE_WRN("Cannot delete the texture [" << tag << "] while there are objects referencing it.");
                return;
            }
        }
        textureMap.erase(tag);
    }
}

void RenderManager::UnregisterMesh(const std::string& tag, const EngineContext& engineContext)
{
    auto it = meshMap.find(tag);
    if (it == meshMap.end())
    {
        SNAKE_LOG("Cannot delete the mesh [" << tag << "] because it was not found.");
        return;
    }
    Mesh* target = it->second.get();
    GameState* gameState = engineContext.stateManager->GetCurrentState();
    if (gameState)
    {
        std::vector<Object*> objects = gameState->GetObjectManager().GetAllRawPtrObjects();
        for (auto obj : objects)
        {
            if (obj->GetMesh() == target)
            {
                SNAKE_WRN("Cannot delete the mesh [" << tag << "] while there are objects referencing it.");
                return;
            }
        }
        meshMap.erase(tag);
    }
}

void RenderManager::UnregisterMaterial(const std::string& tag, const EngineContext& engineContext)
{
    auto it = materialMap.find(tag);
    if (it == materialMap.end())
    {
        SNAKE_LOG("Cannot delete the material [" << tag << "] because it was not found.");
        return;
    }
    Material* target = it->second.get();
    GameState* gameState = engineContext.stateManager->GetCurrentState();
    if (gameState)
    {
        std::vector<Object*> objects = gameState->GetObjectManager().GetAllRawPtrObjects();
        for (auto obj : objects)
        {
            if (obj->GetMaterial() == target)
            {
                SNAKE_WRN("Cannot delete the material [" << tag << "] while there are objects referencing it.");
                return;
            }
        }
        materialMap.erase(tag);
    }
}

void RenderManager::UnregisterFont(const std::string& tag, const EngineContext& engineContext)
{
    auto it = fontMap.find(tag);
    if (it == fontMap.end())
    {
        SNAKE_LOG("Cannot delete the font [" << tag << "] because it was not found.");
        return;
    }
    Font* target = it->second.get();
    GameState* gameState = engineContext.stateManager->GetCurrentState();
    if (gameState)
    {
        std::vector<Object*> objects = gameState->GetObjectManager().GetAllRawPtrObjects();
        for (auto obj : objects)
        {
            if (obj->GetType() == ObjectType::TEXT && dynamic_cast<TextObject*>(obj)->GetTextInstance()->font == target)
            {
                SNAKE_WRN("Cannot delete the font [" << tag << "] while there are objects referencing it.");
                return;
            }
        }
        fontMap.erase(tag);
    }
}

void RenderManager::UnregisterRenderLayer(const std::string& tag)
{
    renderLayerManager.UnregisterLayer(tag);
}

void RenderManager::UnregisterSpriteSheet(const std::string& tag, const EngineContext& engineContext)
{
    auto it = spritesheetMap.find(tag);
    if (it == spritesheetMap.end())
    {
        SNAKE_LOG("Cannot delete the sprite sheet [" << tag << "] because it was not found.");
        return;
    }
    SpriteSheet* target = it->second.get();
    GameState* gameState = engineContext.stateManager->GetCurrentState();
    if (gameState)
    {
        std::vector<Object*> objects = gameState->GetObjectManager().GetAllRawPtrObjects();
        for (auto obj : objects)
        {
            SpriteAnimator* spriteAnim = obj->GetSpriteAnimator();
            if (spriteAnim && spriteAnim->GetSpriteSheet() == target)
            {
                SNAKE_WRN("Cannot delete the sprite sheet [" << tag << "] while there are objects referencing it.");
                return;
            }
        }
        spritesheetMap.erase(tag);
    }
}

SpriteSheet* RenderManager::GetSpriteSheetByTag(const std::string& tag)
{
    auto it = spritesheetMap.find(tag);
    if (it != spritesheetMap.end())
        return it->second.get();
    else
    {
        SNAKE_ERR("There is no SpriteSheet named '" << tag << "'");
        return defaultSpriteSheet;
    }
}

Shader* RenderManager::GetShaderByTag(const std::string& tag)
{
    auto it = shaderMap.find(tag);
    if (it != shaderMap.end())
        return it->second.get();
    else
    {
        SNAKE_ERR("There is no Shader named '" << tag << "'");
        return defaultShader;
    }
}

Texture* RenderManager::GetTextureByTag(const std::string& tag)
{
    auto it = textureMap.find(tag);
    if (it != textureMap.end())
        return it->second.get();
    else
    {
        SNAKE_ERR("There is no Texture named '" << tag << "'");
        return errorTexture;
    }
}

Mesh* RenderManager::GetMeshByTag(const std::string& tag)
{
    auto it = meshMap.find(tag);
    if (it != meshMap.end())
        return it->second.get();
    else
    {
        SNAKE_ERR("There is no Mesh named '" << tag << "'");
        return defaultMesh;
    }
}

Material* RenderManager::GetMaterialByTag(const std::string& tag)
{
    auto it = materialMap.find(tag);
    if (it != materialMap.end())
        return it->second.get();
    else
    {
        SNAKE_ERR("There is no Material named '" << tag << "'");
        return defaultMaterial;
    }
}

Font* RenderManager::GetFontByTag(const std::string& tag)
{
    auto it = fontMap.find(tag);
    if (it != fontMap.end())
        return it->second.get();
    else
    {
        SNAKE_ERR("There is no Font named '" << tag << "'");
        return nullptr;
    }
}
