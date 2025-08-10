#pragma once
#include "EngineContext.h"
#include "Mesh.h"
#include "Object.h"
#include "Transform.h"

class Camera2D;

/**
 * @brief Holds the font pointer and raw UTF-8 text to render.
 */
struct TextInstance
{
    Font* font = nullptr;
    std::string text = "";
};

/**
 * @brief Object subclass that renders text using a Font-generated mesh.
 *
 * @details
 * - Constructor sets material = font->GetMaterial() and builds an initial mesh via UpdateMesh().
 * - Mesh is (re)built from Font::GenerateTextMesh(text, alignH, alignV).
 * - Draw() batching/flush is handled by RenderManager.
 * - Alignment affects world-space anchoring; GetWorldPosition/GetWorldScale override Object's behavior.
 * - Material/Mesh setters and animator APIs are deleted for this class.
 */
class TextObject : public Object
{
public:
    /**
     * @brief Constructs a text object with a font, string, and alignment.
     * @param font Font used to generate geometry and provide material.
     * @param text UTF-8 text to render.
     * @param alignH Horizontal alignment anchor.
     * @param alignV Vertical alignment anchor.
     */
    TextObject(Font* font, const std::string& text, TextAlignH alignH = TextAlignH::Left, TextAlignV alignV = TextAlignV::Top);

    ~TextObject() override = default;

    /**
     * @brief No-op in current implementation.
     */
    void Init([[maybe_unused]] const EngineContext& engineContext) override;

    /**
     * @brief No-op in current implementation.
     */
    void LateInit([[maybe_unused]] const EngineContext& engineContext) override;

    /**
     * @brief No-op in current implementation.
     */
    void Update([[maybe_unused]] float dt, [[maybe_unused]] const EngineContext& engineContext) override;

    /**
     * @brief Applies per-object color uniform to the material.
     * @details Sets "u_Color" via material->SetUniform("u_Color", color).
     */
    void Draw([[maybe_unused]] const EngineContext& engineContext) override;

    /**
     * @brief No-op in current implementation.
     */
    void Free([[maybe_unused]] const EngineContext& engineContext) override;

    /**
     * @brief No-op in current implementation.
     */
    void LateFree([[maybe_unused]] const EngineContext& engineContext) override;

    /**
     * @brief Bounding radius for culling.
     * @details Uses length( Font::GetTextSize(text) * Transform2D::GetScale() ).
     */
    [[nodiscard]] float GetBoundingRadius() const override;

    /**
     * @brief Sets new text (UTF-8) and rebuilds the mesh.
     * @param text New text.
     * @note Calls UpdateMesh() when the value changes.
     */
    void SetText(const std::string& text);

    /**
     * @brief Replaces the stored TextInstance (font and text).
     * @param textInstance_ New TextInstance.
     * @note Calls UpdateMesh() when the value changes.
     */
    void SetTextInstance(const TextInstance& textInstance_);

    /**
     * @brief Sets horizontal alignment and rebuilds the mesh if changed.
     * @param alignH_ New horizontal alignment.
     */
    void SetAlignH(TextAlignH alignH_);

    /**
     * @brief Sets vertical alignment and rebuilds the mesh if changed.
     * @param alignV_ New vertical alignment.
     */
    void SetAlignV(TextAlignV alignV_);

    /**
     * @brief Returns a pointer to the internal TextInstance.
     */
    TextInstance* GetTextInstance();

    /**
     * @brief Text objects do not use sprite animation.
     * @return Always false.
     */
    [[nodiscard]] bool HasAnimation() const override { return false; }
    /**
     * @brief Text objects do not expose an animator.
     * @return Always nullptr.
     */
    [[nodiscard]] SpriteAnimator* GetAnimator() override { return nullptr; }
    /**
     * @brief Text objects do not expose an animator.
     * @return Always nullptr.
     */
    [[nodiscard]] SpriteAnimator* GetSpriteAnimator() const override { return nullptr; }

    /**
     * @brief World position with alignment pivot applied.
     * @details
     * - Computes alignedScreenPos = transform2D.position + offset,
     *   where offset depends on (alignH, alignV) and half of Font::GetTextSize(text).
     * - If ShouldIgnoreCamera() && referenceCamera, returns
     *   (referenceCamera->GetPosition() + alignedScreenPos) / referenceCamera->GetZoom();
     *   otherwise returns alignedScreenPos.
     */
    [[nodiscard]] glm::vec2 GetWorldPosition() const override;

    /**
     * @brief World scale with font size applied.
     * @details Returns transform2D.scale * Font::GetTextSize(text);
     * divides by camera zoom when ShouldIgnoreCamera() && referenceCamera.
     */
    [[nodiscard]] glm::vec2 GetWorldScale() const override;

    /**
     * @brief Rebuilds the mesh if the Font's atlas version has changed.
     * @details Compares an internal version tracker with Font::GetTextAtlasVersion().
     */
    void CheckFontAtlasAndMeshUpdate();

    /**
     * @brief Deleted: TextObject material is provided by its Font.
     * @note Not intended for user code.
     */
    void SetMaterial(const EngineContext& engineContext, const std::string& tag) = delete;

    /**
     * @brief Deleted: TextObject material is provided by its Font.
     * @note Not intended for user code.
     */
    void SetMaterial(Material* material_) = delete;

    /**
     * @brief Deleted: material access is not exposed on TextObject.
     * @note Not intended for user code.
     */
    [[nodiscard]] Material* GetMaterial() const = delete;

    /**
     * @brief Deleted: TextObject mesh is generated by its Font.
     * @note Not intended for user code.
     */
    void SetMesh(const EngineContext& engineContext, const std::string& tag) = delete;

    /**
     * @brief Deleted: TextObject mesh is generated by its Font.
     * @note Not intended for user code.
     */
    void SetMesh(Mesh* mesh_) = delete;

    /**
     * @brief Deleted: mesh access is not exposed on TextObject.
     * @note Not intended for user code.
     */
    [[nodiscard]] Mesh* GetMesh() const = delete;

    /**
     * @brief Deleted: sprite animation is not used on TextObject.
     * @note Not intended for user code.
     */
    void AttachAnimator(std::unique_ptr<SpriteAnimator> anim) = delete;

    /**
     * @brief Deleted: sprite animation is not used on TextObject.
     * @note Not intended for user code.
     */
    void AttachAnimator(SpriteSheet* sheet, float frameTime, bool loop = true) = delete;

    /**
     * @brief Deleted: sprite animation is not used on TextObject.
     * @note Not intended for user code.
     */
    void DetachAnimator() = delete;
protected:
    /**
     * @brief Rebuilds textMesh from the current TextInstance and alignment.
     * @details Creates a new Mesh via Font::GenerateTextMesh(); updates base mesh pointer.
     * @note Internal.
     */
    void UpdateMesh();

    TextAlignH alignH;
    TextAlignV alignV;

    TextInstance textInstance;
    std::unique_ptr<Mesh> textMesh;

    int textAtlasVersionTracker = 0;
};
