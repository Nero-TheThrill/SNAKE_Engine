#pragma once
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <array>

#include "Animation.h"
#include "Material.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera2D.h"
#include "Font.h"
#include "GameObject.h"
#include "InstanceBatchKey.h"
#include "RenderLayerManager.h"

struct TextInstance;
class SNAKE_Engine;
class StateManager;

using TextureTag = std::string;
using UniformName = std::string;
using FilePath = std::string;
using RenderCommand = std::function<void()>;

using ShaderMap = std::map<Shader*, std::map<InstanceBatchKey, std::vector<std::pair<Object*, Camera2D*>>>>;
using RenderMap = std::array<ShaderMap, RenderLayerManager::MAX_LAYERS>;

struct LineInstance
{
    glm::vec2 from = { 0,0 };
    glm::vec2 to = { 0,0 };
    glm::vec4 color = { 1,1,1,1 };
    float lineWidth = 1;
};

class RenderManager
{
    friend ObjectManager;
    friend StateManager;
    friend SNAKE_Engine;

public:
    void RegisterShader(const std::string& tag, const std::vector<std::pair<ShaderStage, FilePath>>& sources);

    void RegisterShader(const std::string& tag, std::unique_ptr<Shader> shader);

    void RegisterTexture(const std::string& tag, const FilePath& path, const TextureSettings& settings = {});

    void RegisterTexture(const std::string& tag, std::unique_ptr<Texture> texture);

    void RegisterMesh(const std::string& tag, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices = {}, PrimitiveType primitiveType = PrimitiveType::Triangles);

    void RegisterMesh(const std::string& tag, std::unique_ptr<Mesh> mesh);

    void RegisterMaterial(const std::string& tag, const std::string& shaderTag, const std::unordered_map<UniformName, TextureTag>& textureBindings);

    void RegisterMaterial(const std::string& tag, std::unique_ptr<Material> material);

    void RegisterFont(const std::string& tag, const std::string& ttfPath, uint32_t pixelSize);

    void RegisterFont(const std::string& tag, std::unique_ptr<Font> font);

    void RegisterRenderLayer(const std::string& tag, uint8_t layer);

    void RegisterSpriteSheet(const std::string& tag, const std::string& textureTag, int frameW, int frameH);

    void UnregisterShader(const std::string& tag, const EngineContext& engineContext);

    void UnregisterTexture(const std::string& tag, const EngineContext& engineContext);

    void UnregisterMesh(const std::string& tag, const EngineContext& engineContext);

    void UnregisterMaterial(const std::string& tag, const EngineContext& engineContext);

    void UnregisterFont(const std::string& tag, const EngineContext& engineContext);

    void UnregisterRenderLayer(const std::string& tag);

    void UnregisterSpriteSheet(const std::string& tag, const EngineContext& engineContext);
    

    [[nodiscard]] Shader* GetShaderByTag(const std::string& tag);

    [[nodiscard]] Texture* GetTextureByTag(const std::string& tag);

    [[nodiscard]] Mesh* GetMeshByTag(const std::string& tag);

    [[nodiscard]] Material* GetMaterialByTag(const std::string& tag);

    [[nodiscard]] Font* GetFontByTag(const std::string& tag);

    SpriteSheet* GetSpriteSheetByTag(const std::string& tag);

    void FlushDrawCommands(const EngineContext& engineContext);

    void SetViewport(int x, int y, int width, int height);

    void ClearBackground(int x, int y, int width, int height, glm::vec4 color);

    void DrawDebugLine(const glm::vec2& from, const glm::vec2& to, Camera2D* camera = nullptr, const glm::vec4& color = { 1,1,1,1 }, float lineWidth = 1.0f);

    [[nodiscard]] RenderLayerManager& GetRenderLayerManager();
private:
    void Init(const EngineContext& engineContext);

    void BuildRenderMap(const std::vector<Object*>& source, Camera2D* camera);

    void Submit(const std::vector<Object*>& objects, const EngineContext& engineContext);

    void FlushDebugLineDrawCommands(const EngineContext& engineContext);

    std::unordered_map<std::string, std::unique_ptr<Shader>> shaderMap;
    std::unordered_map<std::string, std::unique_ptr<Texture>> textureMap;
    std::unordered_map<std::string, std::unique_ptr<Mesh>> meshMap;
    std::unordered_map<std::string, std::unique_ptr<Material>> materialMap;
    std::unordered_map<std::string, std::unique_ptr<Font>> fontMap;
    std::unordered_map<std::string, std::unique_ptr<SpriteSheet>> spritesheetMap;


    using CameraAndWidth = std::pair<Camera2D*, float>;
    struct CameraAndWidthHash
    {
        std::size_t operator()(const CameraAndWidth& key) const
        {
            return std::hash<Camera2D*>()(key.first) ^ std::hash<float>()(key.second);
        }
    };
    std::unordered_map<CameraAndWidth, std::vector<LineInstance>, CameraAndWidthHash> debugLineMap;
    GLuint debugLineVAO = 0, debugLineVBO = 0;

    Shader* defaultShader, *debugLineShader;
    Material* defaultMaterial;
    SpriteSheet* defaultSpriteSheet;
    Mesh* defaultMesh;

    RenderMap renderMap;
    RenderLayerManager renderLayerManager;

    Texture* errorTexture;
};



class FrustumCuller
{
public:
    static void CullVisible(const Camera2D& camera, const std::vector<Object*>& allObjects,
        std::vector<Object*>& outVisibleList, glm::vec2 viewportSize);
};
