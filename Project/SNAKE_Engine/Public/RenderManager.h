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

/**
 * @brief Manages all rendering operations in the engine.
 *
 * @details
 * RenderManager is responsible for submitting and batching draw calls,
 * managing render layers, handling instanced rendering, and drawing debug visuals.
 * It also acts as a central registry for shaders, textures, materials, meshes, fonts,
 * and sprite sheets using string-based tags.
 *
 * Rendering is deferred: draw commands are queued using Submit() and flushed at the end of each frame.
 * The system supports instanced and non-instanced rendering, 2D camera-based culling, and custom render layers.
 *
 * Used internally by StateManager, ObjectManager, and GameObjects to organize rendering.
 *
 * @note This class is automatically initialized by the engine. End users typically interact with it through EngineContext.
 */
class RenderManager
{
    friend ObjectManager;
    friend StateManager;
    friend SNAKE_Engine;

public:
    /**
	 * @brief Registers a shader from multiple source files using a tag.
	 *
	 * @details
	 * Loads and compiles a shader using the provided list of stage-filepath pairs.
	 * The resulting shader is linked and stored in the internal shader registry with the given tag.
	 * If a shader with the same tag already exists, a warning is logged and registration is skipped.
	 *
	 * @param tag A unique string identifier for the shader.
	 * @param sources A list of pairs consisting of ShaderStage and corresponding file path.
	 *
	 * @note This function is intended for external registration of custom shaders.
	 *
	 * @code
	 * renderManager.RegisterShader("basic", {
	 *     { ShaderStage::Vertex, "shaders/basic.vert" },
	 *     { ShaderStage::Fragment, "shaders/basic.frag" }
	 * });
	 * @endcode
	 */
    void RegisterShader(const std::string& tag, const std::vector<std::pair<ShaderStage, FilePath>>& sources);

    /**
	 * @brief Registers a pre-constructed Shader instance with a tag.
	 *
	 * @details
	 * Stores the provided Shader object in the internal shader map under the given tag.
	 * If the tag already exists, the registration is skipped and a warning is logged.
	 * Ownership of the Shader is transferred to the RenderManager.
	 *
	 * @param tag A unique string identifier for the shader.
	 * @param shader A unique pointer to a pre-linked Shader object.
	 *
	 * @note Use this when creating shaders programmatically without loading from files.
	 *
	 * @code
	 * auto shader = std::make_unique<Shader>();
	 * shader->AttachFromSource(...);
	 * shader->Link();
	 * renderManager.RegisterShader("custom", std::move(shader));
	 * @endcode
	 */
    void RegisterShader(const std::string& tag, std::unique_ptr<Shader> shader);

    /**
	 * @brief Loads and registers a texture from file using a tag.
	 *
	 * @details
	 * Creates a new Texture instance from the given file path with optional texture settings
	 * such as filtering and wrapping. The texture is stored internally using the provided tag.
	 * If a texture with the same tag already exists, registration is skipped and a warning is logged.
	 *
	 * @param tag A unique identifier for the texture.
	 * @param path File path to the texture image.
	 * @param settings Optional texture parameters (e.g., filter, wrap, mipmap).
	 *
	 * @code
	 * renderManager.RegisterTexture("player_tex", "textures/player.png");
	 * @endcode
	 */
    void RegisterTexture(const std::string& tag, const FilePath& path, const TextureSettings& settings = {});

    /**
	 * @brief Registers an externally created texture with a tag.
	 *
	 * @details
	 * Stores the provided Texture instance in the internal texture registry using the given tag.
	 * If a texture with the same tag already exists, registration is skipped and a warning is logged.
	 * Ownership of the Texture is transferred to the RenderManager.
	 *
	 * @param tag A unique string identifier for the texture.
	 * @param texture A unique pointer to a pre-created Texture object.
	 *
	 * @note Use this to register dynamic or procedurally generated textures.
	 *
	 * @code
	 * auto tex = std::make_unique<Texture>(data, w, h, 4);
	 * renderManager.RegisterTexture("dynamic_ui", std::move(tex));
	 * @endcode
	 */
    void RegisterTexture(const std::string& tag, std::unique_ptr<Texture> texture);

	/**
	 * @brief Creates and registers a mesh using vertex/index data.
	 *
	 * @details
	 * Builds a new Mesh instance from the given vertex and index buffers,
	 * using the specified primitive type (e.g., triangles, lines). The mesh is then
	 * stored internally under the provided tag for future use. If a mesh with the
	 * same tag already exists, registration is skipped and a warning is logged.
	 *
	 * @param tag A unique identifier for the mesh.
	 * @param vertices Vertex buffer to define geometry.
	 * @param indices Optional index buffer. If empty, non-indexed drawing is used.
	 * @param primitiveType Type of primitive to render (default: triangles).
	 *
	 * @code
	 * renderManager.RegisterMesh("quad", quadVertices, quadIndices);
	 * @endcode
	 */
    void RegisterMesh(const std::string& tag, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices = {}, PrimitiveType primitiveType = PrimitiveType::Triangles);

	/**
	 * @brief Registers a pre-constructed mesh with a tag.
	 *
	 * @details
	 * Stores the given Mesh object in the internal registry using the specified tag.
	 * If the tag already exists, the registration is skipped and a warning is logged.
	 * Ownership of the Mesh is transferred to the RenderManager.
	 *
	 * @param tag A unique identifier for the mesh.
	 * @param mesh A unique pointer to a Mesh object.
	 *
	 * @note Use this to register procedurally generated or external meshes.
	 *
	 * @code
	 * auto mesh = std::make_unique<Mesh>(vertices, indices);
	 * renderManager.RegisterMesh("dynamic", std::move(mesh));
	 * @endcode
	 */
    void RegisterMesh(const std::string& tag, std::unique_ptr<Mesh> mesh);

	/**
	 * @brief Creates and registers a material using a shader and texture bindings.
	 *
	 * @details
	 * Constructs a new Material by looking up the shader using shaderTag, then applies
	 * the provided texture bindings (uniform name -> texture tag). The material is registered
	 * internally with the given tag. If the shader is not found, the material is not created.
	 * If a material with the same tag already exists, registration is skipped and a warning is logged.
	 *
	 * @param tag A unique identifier for the material.
	 * @param shaderTag The tag of a previously registered shader.
	 * @param textureBindings A map of uniform names to texture tags for binding.
	 *
	 * @code
	 * renderManager.RegisterMaterial("player_mat", "basic", {
	 *     { "u_Texture", "player_tex" }
	 * });
	 * @endcode
	 */
    void RegisterMaterial(const std::string& tag, const std::string& shaderTag, const std::unordered_map<UniformName, TextureTag>& textureBindings);

	/**
	 * @brief Registers an externally constructed material using a tag.
	 *
	 * @details
	 * Stores the given Material object in the internal material registry under the specified tag.
	 * If the tag already exists, registration is skipped and a warning is logged.
	 * Ownership of the Material is transferred to the RenderManager.
	 *
	 * @param tag A unique string identifier for the material.
	 * @param material A unique pointer to a Material instance.
	 *
	 * @code
	 * auto mat = std::make_unique<Material>(shader);
	 * mat->SetTexture("u_Texture", texture);
	 * renderManager.RegisterMaterial("custom_mat", std::move(mat));
	 * @endcode
	 */
    void RegisterMaterial(const std::string& tag, std::unique_ptr<Material> material);

	/**
	 * @brief Loads and registers a font from a TTF file.
	 *
	 * @details
	 * Creates a new Font object using the given TTF file path and pixel size,
	 * then stores it in the internal font registry with the specified tag.
	 * The pixel size must be within the allowed bounds (typically 4 to 64).
	 * If a font with the same tag already exists, registration is skipped and a warning is logged.
	 *
	 * @param tag A unique identifier for the font.
	 * @param ttfPath Path to the TTF font file.
	 * @param pixelSize Desired pixel size for glyph rendering.
	 *
	 * @code
	 * renderManager.RegisterFont("main_ui", "fonts/NanumGothic.ttf", 32);
	 * @endcode
	 */
    void RegisterFont(const std::string& tag, const std::string& ttfPath, uint32_t pixelSize);

	/**
	 * @brief Registers an externally created Font object.
	 *
	 * @details
	 * Stores the given Font instance in the internal font registry under the specified tag.
	 * If a font with the same tag already exists, the registration is skipped and a warning is logged.
	 * Ownership of the Font is transferred to the RenderManager.
	 *
	 * @param tag A unique string identifier for the font.
	 * @param font A unique pointer to a Font object.
	 *
	 * @code
	 * auto font = std::make_unique<Font>(renderManager, "fonts/Roboto.ttf", 24);
	 * renderManager.RegisterFont("body_text", std::move(font));
	 * @endcode
	 */
    void RegisterFont(const std::string& tag, std::unique_ptr<Font> font);

	/**
	 * @brief Registers a custom render layer using a name tag.
	 *
	 * @details
	 * Adds a new render layer to the RenderLayerManager with the given name.
	 * This allows grouping of objects to be drawn in a specific rendering order.
	 * Only up to RenderLayerManager::MAX_LAYERS (typically 16) layers can be registered.
	 * If the name already exists or the maximum is exceeded, registration is skipped.
	 *
	 * @param tag The name of the new render layer.
	 *
	 * @code
	 * renderManager.RegisterRenderLayer("UI");
	 * renderManager.RegisterRenderLayer("Enemy");
	 * @endcode
	 */
    void RegisterRenderLayer(const std::string& tag);

	/**
	 * @brief Registers a sprite sheet using a texture and frame dimensions.
	 *
	 * @details
	 * Retrieves the texture using textureTag and creates a SpriteSheet object by dividing
	 * the texture into grid-based frames of size (frameW x frameH). The resulting sheet is stored
	 * under the given tag. If the texture is not found, registration is aborted.
	 * If a sprite sheet with the same tag already exists, registration is skipped.
	 *
	 * @param tag A unique identifier for the sprite sheet.
	 * @param textureTag The tag of the texture to use.
	 * @param frameW Width of each frame in pixels.
	 * @param frameH Height of each frame in pixels.
	 *
	 * @code
	 * renderManager.RegisterSpriteSheet("explosion_sheet", "explosion_tex", 64, 64);
	 * @endcode
	 */
    void RegisterSpriteSheet(const std::string& tag, const std::string& textureTag, int frameW, int frameH);


	/**
	 * @brief Retrieves a registered shader by tag.
	 *
	 * @details
	 * Looks up a shader from the internal shader map using the specified tag.
	 * If the tag is not found, logs an error and returns nullptr.
	 *
	 * @param tag Tag of the shader to retrieve.
	 * @return Pointer to the Shader object, or nullptr if not found.
	 */
    [[nodiscard]] Shader* GetShaderByTag(const std::string& tag);

	/**
	 * @brief Retrieves a registered texture by tag.
	 *
	 * @details
	 * Returns the texture associated with the given tag from the internal registry.
	 * If the tag is not found, logs an error and returns nullptr.
	 *
	 * @param tag Tag of the texture to retrieve.
	 * @return Pointer to the Texture object, or nullptr if not found.
	 */
    [[nodiscard]] Texture* GetTextureByTag(const std::string& tag);

	/**
	 * @brief Retrieves a registered mesh by tag.
	 *
	 * @details
	 * Fetches a Mesh object associated with the given tag from the internal mesh map.
	 * Returns nullptr if the tag is not found.
	 *
	 * @param tag Tag of the mesh to retrieve.
	 * @return Pointer to the Mesh object, or nullptr if not found.
	 */
    [[nodiscard]] Mesh* GetMeshByTag(const std::string& tag);

	/**
	 * @brief Retrieves a registered material by tag.
	 *
	 * @details
	 * Searches for a material using the specified tag. If no such material exists,
	 * logs an error and returns nullptr.
	 *
	 * @param tag Tag of the material to retrieve.
	 * @return Pointer to the Material object, or nullptr if not found.
	 */
    [[nodiscard]] Material* GetMaterialByTag(const std::string& tag);

	/**
	 * @brief Retrieves a registered font by tag.
	 *
	 * @details
	 * Finds and returns a Font object using the given tag. If not found,
	 * logs an error and returns nullptr.
	 *
	 * @param tag Tag of the font to retrieve.
	 * @return Pointer to the Font object, or nullptr if not found.
	 */
    [[nodiscard]] Font* GetFontByTag(const std::string& tag);

	/**
	 * @brief Retrieves a registered sprite sheet by tag.
	 *
	 * @details
	 * Looks up the sprite sheet associated with the given tag. If the sheet is not found,
	 * logs an error and returns nullptr.
	 *
	 * @param tag Tag of the sprite sheet to retrieve.
	 * @return Pointer to the SpriteSheet object, or nullptr if not found.
	 */
    SpriteSheet* GetSpriteSheetByTag(const std::string& tag);

	/**
	 * @brief Submits a raw draw command to the render queue.
	 *
	 * @details
	 * Stores the provided draw function into the internal render queue.
	 * All submitted draw commands are deferred and executed in order
	 * at the end of the current frame via FlushDrawCommands().
	 *
	 * This method is intended for advanced use cases such as manual OpenGL calls
	 * or custom rendering outside the object-based system.
	 *
	 * @param drawFunc A callable rendering function (usually a lambda).
	 *
	 * @code
	 * renderManager.Submit([]() {
	 *     glDrawArrays(GL_TRIANGLES, 0, 6);
	 * });
	 * @endcode
	 */
    void Submit(std::function<void()>&& drawFunc);

	/**
	 * @brief Executes all deferred draw commands for the current frame.
	 *
	 * @details
	 * Flushes the internal render queue, executing all submitted commands in order.
	 * Also triggers batch rendering based on render layers, shaders, and instance keys.
	 * Typically, should be called once per frame after object rendering is complete.
	 *
	 * @param engineContext The current engine context.
	 *
	 * @note This is automatically called by StateManager::Draw().
	 */
    void FlushDrawCommands(const EngineContext& engineContext);

	/**
	 * @brief Sets the OpenGL viewport for rendering.
	 *
	 * @details
	 * Queues a command that sets the OpenGL viewport to the specified region.
	 * The viewport defines the rectangular area of the window where rendering occurs.
	 * This function uses deferred submission, meaning the viewport change takes effect
	 * during the next FlushDrawCommands() call.
	 *
	 * @param x The x-coordinate of the lower-left corner of the viewport.
	 * @param y The y-coordinate of the lower-left corner of the viewport.
	 * @param width Width of the viewport in pixels.
	 * @param height Height of the viewport in pixels.
	 *
	 * @code
	 * renderManager.SetViewport(0, 0, 1280, 720);
	 * @endcode
	 */
    void SetViewport(int x, int y, int width, int height);

	/**
	 * @brief Clears a specified region of the screen with a given color.
	 *
	 * @details
	 * Submits a command to clear a rectangular region of the framebuffer
	 * using scissor test and the specified RGBA color. This operation only affects
	 * the specified region and can be used to selectively clear parts of the screen.
	 * The clear command is deferred until FlushDrawCommands().
	 *
	 * @param x The x-coordinate of the bottom-left corner.
	 * @param y The y-coordinate of the bottom-left corner.
	 * @param width Width of the region to clear.
	 * @param height Height of the region to clear.
	 * @param color RGBA color to clear with.
	 *
	 * @code
	 * renderManager.ClearBackground(0, 0, 640, 360, {0.0f, 0.0f, 0.0f, 1.0f});
	 * @endcode
	 */
    void ClearBackground(int x, int y, int width, int height, glm::vec4 color);

	/**
	 * @brief Submits a 2D debug line to be drawn on screen.
	 *
	 * @details
	 * Adds a line to the internal debug draw buffer, which will be rendered at the end of the frame.
	 * Useful for visualizing colliders, logic zones, or guidance lines.
	 * Lines are grouped by camera and line width for efficient rendering.
	 *
	 * @param from Starting position of the line in world space.
	 * @param to Ending position of the line in world space.
	 * @param camera Optional camera to use for projection. If null, screen-space projection is used.
	 * @param color Line color in RGBA format.
	 * @param lineWidth Width of the line (default: 1.0f).
	 *
	 * @code
	 * renderManager.DrawDebugLine({0,0}, {100,0}, camera, {1,0,0,1}, 2.0f);
	 * @endcode
	 */
    void DrawDebugLine(const glm::vec2& from, const glm::vec2& to, Camera2D* camera = nullptr, const glm::vec4& color = { 1,1,1,1 }, float lineWidth = 1.0f);

	/**
	 * @brief Provides access to the internal render layer manager.
	 *
	 * @details
	 * Returns a reference to the RenderLayerManager instance, which allows users to
	 * query or assign custom layer IDs for rendering order control.
	 *
	 * @return Reference to the internal RenderLayerManager.
	 *
	 * @code
	 * auto& layerMgr = renderManager.GetRenderLayerManager();
	 * uint8_t uiLayer = layerMgr.GetLayerID("UI").value_or(0);
	 * @endcode
	 */
    [[nodiscard]] RenderLayerManager& GetRenderLayerManager();
private:
	/**
	 * @brief Initializes internal shaders and debug line buffers.
	 *
	 * @details
	 * Loads default internal shaders such as "internal_text" and "internal_debug_line",
	 * and prepares GPU resources (VAO, VBO) for debug line rendering.
	 * Also enables OpenGL blending for alpha support.
	 *
	 * @param engineContext The current engine context.
	 *
	 * @note This function is called internally by the engine and should not be called manually.
	 */
    void Init(const EngineContext& engineContext);

	/**
	 * @brief Groups renderable objects into instancing batches for submission.
	 *
	 * @details
	 * Categorizes visible objects by render layer, shader, mesh-material pairs
	 * and stores them into the internal render map. This batching system enables
	 * optimized instanced or individual rendering later during submission.
	 *
	 * @param source List of objects to process.
	 * @param camera The camera used for rendering. May be null for screen-space rendering.
	 *
	 * @note This is an internal helper and not intended for external use.
	 */
    void BuildRenderMap(const std::vector<Object*>& source, Camera2D* camera);

	/**
	 * @brief Converts the render map into executable OpenGL draw calls.
	 *
	 * @details
	 * Iterates through all render layers and batches in the internal render map,
	 * generating OpenGL draw commands for either instanced or non-instanced rendering.
	 * Automatically binds materials, sets projection/model matrices, and updates instance buffers.
	 *
	 * @param engineContext The current engine context.
	 *
	 * @note This is an internal rendering step called by FlushDrawCommands().
	 */
    void SubmitRenderMap(const EngineContext& engineContext);

	/**
	 * @brief Submits a batch of objects for rendering, with optional frustum culling.
	 *
	 * @details
	 * Receives a list of objects and prepares them for rendering. If a camera is provided,
	 * performs frustum culling and only submits visible objects. Otherwise, submits all objects.
	 * Uses BuildRenderMap() internally for batching.
	 *
	 * @param engineContext The current engine context.
	 * @param objects The list of objects to render.
	 * @param camera Optional camera for visibility culling.
	 *
	 * @note This function is called internally by ObjectManager::DrawAll() and similar functions.
	 */
    void Submit(const EngineContext& engineContext, const std::vector<Object*>& objects, Camera2D* camera);

	/**
	 * @brief Renders all submitted debug lines for the current frame.
	 *
	 * @details
	 * Iterates through all buffered debug lines and draws them using an internal debug line shader.
	 * Called automatically by StateManager after all normal rendering is complete,
	 * but only if the engine's debug mode is enabled.
	 *
	 * @param engineContext The current engine context.
	 *
	 * @note Users do not need to call this manually. It is internally managed.
	 */
	void FlushDebugLineDrawCommands(const EngineContext& engineContext);

    std::unordered_map<std::string, std::unique_ptr<Shader>> shaderMap;
    std::unordered_map<std::string, std::unique_ptr<Texture>> textureMap;
    std::unordered_map<std::string, std::unique_ptr<Mesh>> meshMap;
    std::unordered_map<std::string, std::unique_ptr<Material>> materialMap;
    std::unordered_map<std::string, std::unique_ptr<Font>> fontMap;
    std::unordered_map<std::string, std::unique_ptr<SpriteSheet>> spritesheetMap;
    std::vector<RenderCommand> renderQueue;


    using CameraAndWidth = std::pair<Camera2D*, float>;

	/// @brief Internal hash function used to group debug lines by (Camera2D*, line width).
    struct CameraAndWidthHash
    {
        std::size_t operator()(const CameraAndWidth& key) const
        {
            return std::hash<Camera2D*>()(key.first) ^ std::hash<float>()(key.second);
        }
    };
    std::unordered_map<CameraAndWidth, std::vector<LineInstance>, CameraAndWidthHash> debugLineMap;
    GLuint debugLineVAO = 0, debugLineVBO = 0;
    Shader* debugLineShader;

    RenderMap renderMap;
    RenderLayerManager renderLayerManager;
};



class FrustumCuller
{
public:
	/**
	 * @brief Filters visible objects using 2D frustum culling.
	 *
	 * @details
	 * Iterates through a list of objects and determines whether each object
	 * is within the visible bounds of the given camera and viewport.
	 * Objects flagged as "ignore camera" are always included.
	 * The result is stored in the outVisibleList.
	 *
	 * @param camera The 2D camera used to define the view bounds.
	 * @param allObjects The list of all objects to be tested.
	 * @param outVisibleList Output list containing only visible objects.
	 * @param viewportSize Size of the rendering area in screen pixels.
	 *
	 * @note This is an internal helper used by RenderManager for visibility culling.
	 */
    static void CullVisible(const Camera2D& camera, const std::vector<Object*>& allObjects,
        std::vector<Object*>& outVisibleList, glm::vec2 viewportSize);
};
