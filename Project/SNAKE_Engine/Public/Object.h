#pragma once
#include <memory>
#include <string>

#include "Animation.h"
#include "Collider.h"
#include "Mesh.h"
#include "Transform.h"
class FrustumCuller;
struct EngineContext;

/**
 * @brief Runtime category of objects.
 */
enum class ObjectType
{
    GAME, ///< Regular game object.
    TEXT  ///< Text object (see TextObject).
};

/**
 * @brief Base interface for all renderable/updatable objects.
 *
 * @details
 * - Provides lifecycle hooks (Init/LateInit/Update/Draw/Free/LateFree).
 * - Holds Transform2D, Material*, Mesh*, color, tags, and optional components:
 *   SpriteAnimator and Collider.
 * - Supports render layer tagging and per-object visibility/aliveness flags.
 * - Supports camera-ignored behavior: when enabled, world position/scale are
 *   derived relative to a reference Camera2D (see GetWorldPosition/GetWorldScale).
 * - Friend with FrustumCuller to allow visibility tests using GetBoundingRadius().
 */
class Object
{
    friend FrustumCuller;
public:
    Object() = delete;

    /**
     * @brief Called once before the object participates in updates.
     * @param engineContext Aggregated engine systems.
     */
    virtual void Init([[maybe_unused]] const EngineContext& engineContext) = 0;

    /**
     * @brief Called after Init() for post-initialization that needs other objects ready.
     * @param engineContext Aggregated engine systems.
     */
    virtual void LateInit([[maybe_unused]] const EngineContext& engineContext) = 0;

    /**
     * @brief Per-frame update.
     * @param dt Delta time (seconds).
     * @param engineContext Aggregated engine systems.
     */
    virtual void Update([[maybe_unused]] float dt, [[maybe_unused]] const EngineContext& engineContext) = 0;

    /**
     * @brief Per-frame draw submission (if the derived object uses it).
     * @param engineContext Aggregated engine systems.
     * @note Actual batching and flushing are handled by RenderManager.
     */
    virtual void Draw([[maybe_unused]] const EngineContext& engineContext) = 0;

    /**
     * @brief Release runtime resources that depend on engine systems.
     * @param engineContext Aggregated engine systems.
     */
    virtual void Free([[maybe_unused]] const EngineContext& engineContext) = 0;

    /**
     * @brief Called after Free() for final teardown ordering.
     * @param engineContext Aggregated engine systems.
     */
    virtual void LateFree([[maybe_unused]] const EngineContext& engineContext) = 0;

    /**
     * @brief Virtual destructor.
     */
    virtual ~Object() = default;

    /**
     * @brief Collision event callback. Default: no-op.
     * @param other The other object involved in collision.
     */
    virtual void OnCollision(Object* other) {}

    /**
     * @brief Returns whether the object is alive.
     * @details Dead objects are removed by ObjectManager.
     */
    [[nodiscard]] const bool& IsAlive() const;

    /**
     * @brief Returns whether the object is currently visible to renderer submission.
     */
    [[nodiscard]] const bool& IsVisible() const;

    /**
     * @brief Sets visibility flag (participation in rendering).
     */
    void SetVisibility(bool _isVisible);

    /**
     * @brief Marks the object as dead. ObjectManager will erase it.
     */
    void Kill();

    /**
     * @brief Assigns a unique tag string to this object.
     * @param tag Identifier used by ObjectManager lookup APIs.
     */
    void SetTag(const std::string& tag);

    /**
     * @brief Returns the current object tag.
     */
    [[nodiscard]] const std::string& GetTag() const;

    /**
     * @brief Returns the render layer tag this object belongs to.
     */
    [[nodiscard]] const std::string& GetRenderLayerTag() const;

    /**
     * @brief Sets the render layer tag (affects RenderManager layer batching).
     * @param tag Layer name registered on RenderManager.
     */
    void SetRenderLayer(const std::string& tag);

    /**
     * @brief Sets Material* by tag resolved through RenderManager.
     * @param engineContext Engine systems (uses engineContext.renderManager).
     * @param tag Material tag registered on RenderManager.
     * @code
     * obj->SetMaterial(engineContext, "sprite-lit");
     * @endcode
     */
    void SetMaterial(const EngineContext& engineContext, const std::string& tag);

    /**
     * @brief Directly assigns a Material*.
     */
    void SetMaterial(Material* material_) { material = material_; }

    /**
     * @brief Returns the assigned Material* (may be nullptr).
     */
    [[nodiscard]] Material* GetMaterial() const;

    /**
     * @brief Sets Mesh* by tag resolved through RenderManager.
     * @param engineContext Engine systems (uses engineContext.renderManager).
     * @param tag Mesh tag registered on RenderManager.
     * @code
     * obj->SetMesh(engineContext, "quad");
     * @endcode
     */
    void SetMesh(const EngineContext& engineContext, const std::string& tag);

    /**
     * @brief Directly assigns a Mesh*.
     */
    void SetMesh(Mesh* mesh_) { mesh = mesh_; }

    /**
     * @brief Returns the assigned Mesh* (may be nullptr).
     */
    [[nodiscard]] Mesh* GetMesh() const;

    /**
     * @brief Returns true if this object can be instanced.
     * @details Requires mesh && material && material->IsInstancingSupported().
     */
    [[nodiscard]] bool CanBeInstanced() const;

    /**
     * @brief Returns the 4x4 transform matrix for rendering.
     * @details Forwards to Transform2D::GetMatrix() (cached, lazy rebuild).
     */
    [[nodiscard]] glm::mat4 GetTransform2DMatrix();

    /**
     * @brief Returns a reference to the 2D transform component.
     */
    [[nodiscard]] Transform2D& GetTransform2D();

    /**
     * @brief Sets the per-object color (used by materials that sample it).
     * @param color_ RGBA.
     */
    void SetColor(const  glm::vec4& color_);
    /**
     * @brief Returns the per-object color.
     */
    [[nodiscard]] const glm::vec4& GetColor();

    /**
     * @brief Animator presence query. Default: true if spriteAnimator is set.
     */
    [[nodiscard]] virtual bool HasAnimation() const { return spriteAnimator != nullptr; }

    /**
     * @brief Returns the animator pointer (modifiable).
     */
    [[nodiscard]] virtual SpriteAnimator* GetAnimator() { return spriteAnimator.get(); }

    /**
     * @brief Returns the animator pointer (const).
     */
    [[nodiscard]] virtual SpriteAnimator* GetSpriteAnimator() const { return spriteAnimator.get(); }

    /**
     * @brief Attaches/Detaches a sprite animator.
     */
    void AttachAnimator(std::unique_ptr<SpriteAnimator> anim) { spriteAnimator = std::move(anim); }
    void AttachAnimator(SpriteSheet* sheet, float frameTime, bool loop = true) { spriteAnimator = std::make_unique<SpriteAnimator>(sheet, frameTime, loop); }
    void DetachAnimator() { spriteAnimator = nullptr; }

    /**
     * @brief Sets a Collider component and connects it to this object.
     * @details Ownership is transferred.
     */
    void SetCollider(std::unique_ptr<Collider> c) { collider = std::move(c); }

    /**
     * @brief Returns the collider (may be nullptr).
     */
    [[nodiscard]] Collider* GetCollider() const { return collider.get(); }

    /**
     * @brief Configures collision category/mask bits from ObjectManager's registry.
     *
     * @details
     * - collisionCategory = bit of @p tag.
     * - collisionMask = OR over bits of @p checkCollisionList.
     *
     * @param objectManager Registry owner.
     * @param tag Category group tag for this object.
     * @param checkCollisionList Group tags to collide with.
     * @code
     * obj->SetCollision(objectManager, "player", {"enemy","wall"});
     * @endcode
     */
    void SetCollision(ObjectManager& objectManager, const std::string& tag, const std::vector<std::string>& checkCollisionList);

    /**
     * @brief Returns the collision mask (who this object collides with).
     */
    [[nodiscard]] uint32_t GetCollisionMask() const { return collisionMask; }

    /**
     * @brief Returns the collision category bit of this object.
     */
    [[nodiscard]] uint32_t GetCollisionCategory() const { return collisionCategory; }

    /**
     * @brief Returns whether this object ignores the camera transform.
     * @details When true, world position/scale are computed against referenceCamera.
     */
    [[nodiscard]] bool ShouldIgnoreCamera() const;

    /**
     * @brief Enables/disables camera-ignored behavior and sets the reference camera(Set as an UI).
     * @param shouldIgnoreCamera If true, subsequent world queries use @p cameraForTransformCalc.
     * @param cameraForTransformCalc Reference camera used for GetWorldPosition/Scale.
     */
    void SetIgnoreCamera(bool shouldIgnoreCamera, Camera2D* cameraForTransformCalc = nullptr);

    /**
     * @brief Returns the object type.
     */
    [[nodiscard]] ObjectType GetType() const { return type; }

    /**
     * @brief Returns the reference camera (may be nullptr).
     */
    [[nodiscard]] Camera2D* GetReferenceCamera() const { return referenceCamera; }

    /**
     * @brief Returns world position.
     * @details
     * If ShouldIgnoreCamera() && referenceCamera:
     *   corrected = (cameraPos + transform2D.position) * (1/zoom);
     * else:
     *   corrected = transform2D.position.
     */
    [[nodiscard]] virtual glm::vec2 GetWorldPosition() const;

    /**
     * @brief Returns world scale.
     * @details If ShouldIgnoreCamera() && referenceCamera, divides by camera zoom.
     */
    [[nodiscard]] virtual glm::vec2 GetWorldScale() const;

    /**
     * @brief Sets horizontal/vertical UV flip flags.
     */
    void SetFlipUV_X(bool shouldFlip) { flipUV_X = shouldFlip; }
    void SetFlipUV_Y(bool shouldFlip) { flipUV_Y = shouldFlip; }

    /**
     * @brief Returns a vector multiplier for UV flipping: (-1 or 1 per axis).
     */
    [[nodiscard]] glm::vec2 GetUVFlipVector() const;

protected:
    /**
     * @brief Construct with a specific object type.
     */
    Object(ObjectType objectType) : type(objectType) {}
    ObjectType type;

    /**
     * @brief Bounding radius used for coarse visibility/culling.
     * @details Uses mesh->GetLocalBoundsHalfSize() scaled by Transform2D::GetScale() length.
     * @note Primarily used by FrustumCuller via friend access.
     */
    [[nodiscard]] virtual float GetBoundingRadius() const;

    bool isAlive = true;
    bool isVisible = true;

    bool ignoreCamera = false;
    Camera2D* referenceCamera = nullptr;

    std::string objectTag;
    std::string renderLayerTag;

    Transform2D transform2D;
    Material* material = nullptr;
    Mesh* mesh = nullptr;

    glm::vec4 color = glm::vec4(1);

    std::unique_ptr<SpriteAnimator> spriteAnimator;
    std::unique_ptr<Collider> collider;

    uint32_t collisionCategory = 0;
    uint32_t collisionMask = 0;

    bool flipUV_X = false;
    bool flipUV_Y = false;
};
