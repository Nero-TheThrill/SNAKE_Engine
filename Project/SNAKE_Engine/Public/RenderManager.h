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

using TextureTag = std::string;   ///< Logical name used to look up textures.
using UniformName = std::string;  ///< GLSL uniform name.
using FilePath = std::string;     ///< Filesystem path string.
using RenderCommand = std::function<void()>; ///< (Reserved) generic draw call wrapper.

/// Maps Shader* -> (InstanceBatchKey -> [ (Object*, Camera2D*) ... ]) for one layer.
using ShaderMap = std::map<Shader*, std::map<InstanceBatchKey, std::vector<std::pair<Object*, Camera2D*>>>>;
/// One ShaderMap per render layer (0..MAX_LAYERS-1).
using RenderMap = std::array<ShaderMap, RenderLayerManager::MAX_LAYERS>;

/// Debug line instance (2D endpoints, RGBA color, width in pixels).
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
    /**
     * @brief Registers and compiles a shader program from file sources.
     * @details
     * - Ignores if @p tag already exists (logs a message).
     * - Each pair specifies (stage, file path).
     * - Aborts registration on any attach/link failure.
     */
    void RegisterShader(const std::string& tag, const std::vector<std::pair<ShaderStage, FilePath>>& sources);

    /**
     * @brief Registers a pre-built Shader under a tag.
     * @details Ignored if @p tag already exists (logs).
     */
    void RegisterShader(const std::string& tag, std::unique_ptr<Shader> shader);

    /**
     * @brief Loads a texture from disk and registers it.
     * @details Ignored if @p tag already exists (logs).
     */
    void RegisterTexture(const std::string& tag, const FilePath& path, const TextureSettings& settings = {});

    /**
     * @brief Registers a pre-built Texture under a tag.
     * @details Ignored if @p tag already exists (logs).
     */
    void RegisterTexture(const std::string& tag, std::unique_ptr<Texture> texture);

    /**
     * @brief Builds and registers a Mesh from vertices/indices.
     * @details Ignored if @p tag already exists (logs).
     */
    void RegisterMesh(const std::string& tag, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices = {}, PrimitiveType primitiveType = PrimitiveType::Triangles);

    /**
     * @brief Registers a pre-built Mesh under a tag.
     * @details Ignored if @p tag already exists (logs).
     */
    void RegisterMesh(const std::string& tag, std::unique_ptr<Mesh> mesh);

    /**
     * @brief Creates a Material from a shader tag and binds textures by uniform name.
     * @details
     * - Ignored if @p tag already exists (logs).
     * - Missing shader or texture tags are logged; material is still created if possible.
     */
    void RegisterMaterial(const std::string& tag, const std::string& shaderTag, const std::unordered_map<UniformName, TextureTag>& textureBindings);

    /**
     * @brief Registers a pre-built Material under a tag.
     * @details Ignored if @p tag already exists (logs).
     */
    void RegisterMaterial(const std::string& tag, std::unique_ptr<Material> material);

    /**
     * @brief Creates and registers a Font from TTF with a pixel size.
     * @details
     * - Valid pixelSize range: [4, 64]. Out-of-range is rejected (logs).
     * - Ignored if @p tag already exists (logs).
     */
    void RegisterFont(const std::string& tag, const std::string& ttfPath, uint32_t pixelSize);

    /**
     * @brief Registers a pre-built Font under a tag.
     * @details Ignored if @p tag already exists (logs).
     */
    void RegisterFont(const std::string& tag, std::unique_ptr<Font> font);

    /**
     * @brief Registers a named render layer id.
     * @details For use by objects via SetRenderLayer(tag).
     */
    void RegisterRenderLayer(const std::string& tag, uint8_t layer);

    /**
     * @brief Registers a SpriteSheet from a texture tag and frame size.
     * @details
     * - Fails if texture tag is missing (logs).
     * - Ignored if @p tag already exists (logs).
     */
    void RegisterSpriteSheet(const std::string& tag, const std::string& textureTag, int frameW, int frameH);

    /**
     * @brief Unregisters a shader if no object references it.
     * @details Scans all objects' materials; rejects deletion if any material uses this shader.
     */
    void UnregisterShader(const std::string& tag, const EngineContext& engineContext);

    /**
     * @brief Unregisters a texture if no object references it.
     * @details Rejects deletion if any object's material still binds this texture.
     */
    void UnregisterTexture(const std::string& tag, const EngineContext& engineContext);

    /**
     * @brief Unregisters a mesh if no object references it.
     */
    void UnregisterMesh(const std::string& tag, const EngineContext& engineContext);

    /**
     * @brief Unregisters a material if no object references it.
     */
    void UnregisterMaterial(const std::string& tag, const EngineContext& engineContext);

    /**
     * @brief Unregisters a font if no TextObject references it.
     */
    void UnregisterFont(const std::string& tag, const EngineContext& engineContext);

    /**
     * @brief Unregisters a named render layer.
     */
    void UnregisterRenderLayer(const std::string& tag);

    /**
     * @brief Unregisters a sprite sheet if no animator references it.
     */
    void UnregisterSpriteSheet(const std::string& tag, const EngineContext& engineContext);


    /**
     * @brief Looks up a Shader by tag.
     * @return Shader*, or engine default shader if not found (logs).
     */
    [[nodiscard]] Shader* GetShaderByTag(const std::string& tag);

    /**
     * @brief Looks up a Texture by tag.
     * @return Texture*, or engine error texture if not found (logs).
     */
    [[nodiscard]] Texture* GetTextureByTag(const std::string& tag);

    /**
     * @brief Looks up a Mesh by tag.
     * @return Mesh*, or engine default mesh if not found (logs).
     */
    [[nodiscard]] Mesh* GetMeshByTag(const std::string& tag);

    /**
     * @brief Looks up a Material by tag.
     * @return Material*, or engine default material if not found (logs).
     */
    [[nodiscard]] Material* GetMaterialByTag(const std::string& tag);

    /**
     * @brief Looks up a Font by tag.
     * @return Font* if found; nullptr otherwise (logs).
     */
    [[nodiscard]] Font* GetFontByTag(const std::string& tag);

    /**
     * @brief Looks up a SpriteSheet by tag.
     * @return SpriteSheet*, or engine default sprite sheet if not found (logs).
     */
    SpriteSheet* GetSpriteSheetByTag(const std::string& tag);

    /**
     * @brief Flushes all queued draw batches to the GPU.
     * @details
     * - Iterates layers -> shaders -> (mesh, material, spritesheet) batches.
     * - If the first object in a batch is instancing-capable, submits per-instance data;
     *   otherwise draws objects one-by-one.
     * - Sets view/projection from the batch camera (or identity view); falls back to
     *   engine window size when no camera is present.
     * - Ensures a bound texture exists; if not, binds the engine error texture.
     * - Clears the internal render map afterwards.
     */
    void FlushDrawCommands(const EngineContext& engineContext);

    /**
     * @brief Sets the OpenGL viewport.
     */
    void SetViewport(int x, int y, int width, int height);

    /**
     * @brief Clears a rectangular area with a solid color.
     * @details Uses scissor test; only affects the specified region.
     */
    void ClearBackground(int x, int y, int width, int height, glm::vec4 color);

    /**
     * @brief Queues a debug line to be drawn.
     * @details Lines are grouped by (Camera2D*, lineWidth) and drawn on FlushDebugLineDrawCommands().
     * @param camera If null, renders in screen space (identity view).
     */
    void DrawDebugLine(const glm::vec2& from, const glm::vec2& to, Camera2D* camera = nullptr, const glm::vec4& color = { 1,1,1,1 }, float lineWidth = 1.0f);

    /**
     * @brief Returns the render-layer registry.
     */
    [[nodiscard]] RenderLayerManager& GetRenderLayerManager();
private:
    /**
     * @brief Initializes default resources and GL state.
     * @details
     * - Builds internal default shaders/material/mesh/sprite sheet and a checkerboard error texture.
     * - Creates buffers/VAO for debug lines.
     * - Enables alpha blending (SRC_ALPHA, ONE_MINUS_SRC_ALPHA).
     * @note Internal. Called by SNAKE_Engine.
     */
    void Init(const EngineContext& engineContext);

    /**
     * @brief Builds per-layer shader/batch maps from visible objects.
     * @details Skips null/invisible objects and invalid layer indices.
     * @note Internal. Called by Submit().
     */
    void BuildRenderMap(const std::vector<Object*>& source, Camera2D* camera);

    /**
     * @brief Collects visible objects (by active camera) and queues them for rendering.
     * @note Internal. Called by ObjectManager.
     */
    void Submit(const std::vector<Object*>& objects, const EngineContext& engineContext);

    /**
     * @brief Draws queued debug lines and clears the line map.
     * @details Computes view/projection per (camera, lineWidth) group and renders GL_LINES.
     * @note Internal. Called by StateManager when debug draw is enabled.
     */
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
    std::unordered_map<CameraAndWidth, std::vector<LineInstance>, CameraAndWidthHash> debugLineMap; ///< Queued debug lines per camera/width.
    GLuint debugLineVAO = 0, debugLineVBO = 0;

    Shader* defaultShader, * debugLineShader;
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
    /**
     * @brief Filters objects into @p outVisibleList using circle bounds against the camera view.
     * @details
     * - Skips objects that are !IsAlive() or !IsVisible().
     * - If obj->ShouldIgnoreCamera() is true, the object is always included.
     * - Otherwise, uses camera.IsInView(obj->GetWorldPosition(), obj->GetBoundingRadius(), viewportSize).
     * - @p outVisibleList is cleared before filling.
     */
    static void CullVisible(const Camera2D& camera, const std::vector<Object*>& allObjects,
        std::vector<Object*>& outVisibleList, glm::vec2 viewportSize);
};
