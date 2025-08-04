#pragma once
#include "EngineContext.h"
#include "Mesh.h"
#include "Object.h"
#include "Transform.h"

class Camera2D;

/**
 * @brief Holds font and text content for rendering.
 *
 * @details
 * TextInstance stores the font pointer and the actual UTF-8 text string
 * to be displayed. It is used by TextObject to track what text to render
 * and how it should be processed for mesh generation and alignment.
 *
 * The GetCacheKey() function is used internally to generate a key
 * for caching text meshes.
 */
struct TextInstance
{
    Font* font = nullptr;
    std::string text = "";
    /**
     * @brief Generates a cache key based on the current text content.
     *
     * @details
     * Returns a string key derived from the current UTF-8 text content,
     * used to store or retrieve generated mesh data from the internal cache.
     *
     * @return Cache key string.
     */
    std::string GetCacheKey() const
    {
        return "|" + text;
    }
};

/**
 * @brief A 2D object that renders UTF-8 text on screen.
 *
 * @details
 * TextObject is a renderable entity designed to display dynamic or static text
 * using a specified Font. It generates and caches mesh geometry based on the text content
 * and alignment options, supporting per-frame updates and world transformation.
 *
 * This class does not support animation or material/mesh customization.
 * It uses TextInstance internally to manage font and text content, and supports
 * horizontal and vertical alignment.
 *
 * @note TextObject cannot be assigned a custom material or mesh.
 * It is automatically updated and batched by the rendering system.
 */
class TextObject : public Object
{
public:
    /**
     * @brief Constructs a TextObject with a font, text, and optional alignment.
     *
     * @details
     * Initializes the internal TextInstance and alignment mode. The font must be valid
     * and pre-registered in the RenderManager. The actual mesh is generated later during Init().
     *
     * @param font Pointer to the Font used for rendering.
     * @param text The UTF-8 text string to render.
     * @param alignH Horizontal alignment (default: Left).
     * @param alignV Vertical alignment (default: Top).
     *
     * @code
     * auto text = new TextObject(myFont, "Score: 100", TextAlignH::Center, TextAlignV::Bottom);
     * @endcode
     */
    TextObject(Font* font, const std::string& text, TextAlignH alignH = TextAlignH::Left, TextAlignV alignV = TextAlignV::Top);

    /**
     * @brief Destroys the TextObject and its internal mesh cache.
     */
    ~TextObject() override = default;

    /**
     * @brief Initializes the text mesh and registers draw components.
     *
     * @details
     * Generates the initial mesh for the current text content and registers
     * the object with the rendering system. This is called once after construction.
     *
     * @param engineContext The current engine context.
     */
    void Init([[maybe_unused]] const EngineContext& engineContext) override;

    /**
     * @brief Performs any post-initialization steps after all objects are loaded.
     *
     * @details
     * Called after Init() on all objects. This allows resolving dependencies
     * between TextObject and other components, such as camera or transform adjustments.
     *
     * @param engineContext The current engine context.
     */
    void LateInit([[maybe_unused]] const EngineContext& engineContext) override;

    /**
     * @brief Updates the text object's transform and mesh if needed.
     *
     * @details
     * In most cases, this is a no-op for static text.
     * Dynamic text objects may update their transform or regenerate mesh when content changes.
     *
     * @param dt Delta time in seconds.
     * @param engineContext The current engine context.
     */
    void Update([[maybe_unused]] float dt, [[maybe_unused]] const EngineContext& engineContext) override;

    /**
     * @brief Submits the text mesh for rendering.
     *
     * @details
     * Sends the current mesh to the RenderManager for draw batching.
     * TextObject is rendered according to layer order and alignment settings.
     *
     * @param engineContext The current engine context.
     */
    void Draw([[maybe_unused]] const EngineContext& engineContext) override;

    /**
     * @brief Releases any GPU or cached resources used by the text object.
     *
     * @param engineContext The current engine context.
     */
    void Free([[maybe_unused]] const EngineContext& engineContext) override;

    /**
     * @brief Performs final cleanup after object is removed from the scene.
     *
     * @param engineContext The current engine context.
     */
    void LateFree([[maybe_unused]] const EngineContext& engineContext) override;

    /**
     * @brief Returns the approximate bounding radius of the text object.
     *
     * @details
     * This is used for visibility checks, sorting, or physics culling.
     * The value is estimated based on the current text mesh size.
     *
     * @return Bounding radius in world units.
     */
    [[nodiscard]] float GetBoundingRadius() const override;

    /**
     * @brief Updates the text content to render.
     *
     * @details
     * Replaces the current UTF-8 text string and triggers mesh regeneration.
     * Also clears the cached mesh for the previous content.
     *
     * @param text New UTF-8 text string.
     *
     * @code
     * textObj->SetText("Game Over");
     * @endcode
     */
    void SetText(const std::string& text);

    /**
     * @brief Replaces the current text instance with a new one.
     *
     * @details
     * Updates both the font and text content together, and regenerates the mesh accordingly.
     *
     * @param textInstance_ New TextInstance containing font and text.
     */
    void SetTextInstance(const TextInstance& textInstance_);

    /**
     * @brief Sets the horizontal alignment mode of the text.
     *
     * @details
     * Affects how the text is positioned relative to its Transform.
     * Changing this will rebuild the mesh.
     *
     * @param alignH_ Horizontal alignment mode (Left, Center, Right).
     */
    void SetAlignH(TextAlignH alignH_);

    /**
     * @brief Sets the vertical alignment mode of the text.
     *
     * @details
     * Determines vertical positioning relative to Transform.
     * Triggers mesh regeneration.
     *
     * @param alignV_ Vertical alignment mode (Top, Middle, Bottom).
     */
    void SetAlignV(TextAlignV alignV_);

    /**
     * @brief Returns a pointer to the current text instance.
     *
     * @details
     * Gives access to the internal TextInstance which contains the font and text string.
     * Can be used to inspect or modify the current text state.
     *
     * @return Pointer to the internal TextInstance.
     *
     * @code
     * auto* instance = textObj->GetTextInstance();
     * std::cout << instance->text << std::endl;
     * @endcode
     */
    TextInstance* GetTextInstance();

    /**
     * @brief Indicates that TextObject does not support animation.
     *
     * @return Always returns false.
     */
    [[nodiscard]] bool HasAnimation() const override { return false; }

    /**
     * @brief Returns null since TextObject does not support animation.
     *
     * @return Always returns nullptr.
     */
    [[nodiscard]] SpriteAnimator* GetAnimator() override { return nullptr; }

    /**
     * @brief Returns the world-space position of the text object.
     *
     * @details
     * Used for sorting or rendering relative to camera view.
     *
     * @return World position in 2D space.
     */
    [[nodiscard]] glm::vec2 GetWorldPosition() const override;

    /**
     * @brief Returns the world-space scale of the text object.
     *
     * @details
     * This may affect how glyphs are scaled when rendered.
     *
     * @return World scale as a 2D vector.
     */
    [[nodiscard]] glm::vec2 GetWorldScale() const override;

    /**
     * @brief Disabled. TextObject does not support custom material assignment.
     *
     * @details
     * TextObject uses an internal material derived from its Font system.
     * Attempting to assign a custom material is not allowed and will result in a compile-time error.
     */
    void SetMaterial(const EngineContext& engineContext, const std::string& tag) = delete;

    /**
     * @brief Disabled. TextObject does not expose a material instance.
     *
     * @details
     * TextObject does not allow querying or modifying its internal material.
     * The material is managed automatically through font rendering.
     *
     * @return This function is deleted and not accessible.
     */
    [[nodiscard]] Material* GetMaterial() const = delete;

    /**
     * @brief Disabled. TextObject generates its own mesh dynamically.
     *
     * @details
     * Users cannot assign a custom mesh. The mesh is internally generated from the text string and font.
     */
    void SetMesh(const EngineContext& engineContext, const std::string& tag) = delete;

    /**
     * @brief Disabled. TextObject does not expose its mesh.
     *
     * @details
     * Mesh access is not allowed. Use SetText() to trigger mesh regeneration instead.
     */
    [[nodiscard]] Mesh* GetMesh() const = delete;

    /**
     * @brief Disabled. TextObject does not support SpriteAnimator attachment.
     *
     * @details
     * Since text is rendered using font glyphs, animation systems are not applicable.
     */
    void AttachAnimator(std::unique_ptr<SpriteAnimator> anim) = delete;

    /**
     * @brief Disabled. TextObject does not support SpriteAnimator attachment.
     *
     * @details
     * Since text is rendered using font glyphs, animation systems are not applicable.
     */
    void AttachAnimator(SpriteSheet* sheet, float frameTime, bool loop = true) = delete;


protected:
    /**
     * @brief Regenerates the text mesh based on current content and alignment.
     *
     * @details
     * Clears any previous mesh associated with the current text and generates
     * new geometry using the assigned font and alignment settings.
     * This function is called automatically when text or alignment changes.
     *
     * @note This is an internal utility and is not meant to be called manually.
     */
    void UpdateMesh();

    TextAlignH alignH;
    TextAlignV alignV;

    TextInstance textInstance;
    std::unordered_map<std::string, std::unique_ptr<Mesh>> textMeshCache;
};
