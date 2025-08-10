#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "glm.hpp"


class SpatialHashGrid;
class ObjectManager;
class Camera2D;
class RenderManager;
class Object;
class CircleCollider;
class AABBCollider;

/** @brief Collider concrete type tags. */
enum class ColliderType
{
    None,
    Circle,
    AABB
};

/**
 * @brief Abstract collider attached to an Object, with optional transform-scale coupling.
 *
 * @details
 * - Stores non-owning pointer to the owning Object.
 * - World position can be set/read independently from the owner's transform via SetWorldPosition()/GetWorldPosition().
 * - If useTransformScale is true, derived colliders may adapt their size to the owner's scale (see SyncWithTransformScale()).
 * - Collision uses double dispatch via DispatchAgainst() overrides.
 */
class Collider
{
    friend ObjectManager;
    friend CircleCollider;
    friend AABBCollider;
    friend SpatialHashGrid;
public:
    Collider() = delete;
    /** @brief Constructs with an owner and worldPosition = (0,0). */
    Collider(Object* owner_) : owner(owner_), worldPosition() {}
    virtual ~Collider() = default;

    /** @brief Enable/disable coupling to the owner's transform scale. */
    void SetUseTransformScale(bool use) { useTransformScale = use; }
    /** @brief Returns whether transform scale coupling is enabled. */
    [[nodiscard]] bool IsUsingTransformScale() const { return useTransformScale; }

    /** @brief Sets the collider's world-space position. */
    void SetWorldPosition(const glm::vec2& pos) { worldPosition = pos; }
    /** @brief Returns the collider's world-space position. */
    const glm::vec2& GetWorldPosition() const { return worldPosition; }

    /** @brief Point test in world space. */
    virtual bool CheckPointCollision(const glm::vec2& point) const = 0;

protected:
    /** @brief Owning object (non-owning). */
    [[nodiscard]] Object* GetOwner() const { return owner; }

    /** @brief Concrete collider type. */
    [[nodiscard]] virtual ColliderType GetType() const = 0;

    /**
     * @brief Bounding radius used for broad-phase tests.
     * @details Should bound the collider shape in world space.
     */
    [[nodiscard]] virtual float GetBoundingRadius() const = 0;

    /**
     * @brief Generic collision test against another collider.
     * @details Implemented via double dispatch onto concrete DispatchAgainst().
     */
    [[nodiscard]] virtual bool CheckCollision(const Collider* other) const = 0;

    /** @brief Double-dispatch against a circle collider. */
    [[nodiscard]] virtual bool DispatchAgainst(const CircleCollider& other) const = 0;
    /** @brief Double-dispatch against an AABB collider. */
    [[nodiscard]] virtual bool DispatchAgainst(const AABBCollider& other) const = 0;

    /**
     * @brief Sync internal size with owner's transform scale when enabled.
     * @note Called by the owner/manager when transform changes.
     */
    virtual void SyncWithTransformScale() = 0;

    /**
     * @brief Draws a debug representation of the collider.
     * @param rm Render manager to emit debug geometry.
     * @param cam Camera for debug rendering (may be nullptr).
     * @param color RGBA color for the debug shape.
     */
    virtual void DrawDebug(RenderManager* rm, Camera2D* cam, const glm::vec4& color = { 1,0,0,1 }) const = 0;

    Object* owner;
    bool useTransformScale = true;
    glm::vec2 worldPosition;
};


/**
 * @brief Circle collider with radius (size = 2xradius).
 *
 * @details
 * - Maintains base (unscaled) radius and a scaled radius cache.
 * - When transform-scale coupling is enabled, SyncWithTransformScale() updates the effective radius from the owner's scale.
 */
class CircleCollider : public Collider
{
    friend AABBCollider;
    friend SpatialHashGrid;
public:
    /**
     * @brief Constructs a circle collider.
     * @param owner Owning object.
     * @param size Diameter in world units (initial); radius = size/2.
     */
    CircleCollider(Object* owner, float size)
        : Collider(owner), baseRadius(size / 2.f), scaledRadius(size / 2.f) {
    }

    /** @brief Current effective radius in world units. */
    [[nodiscard]] float GetRadius() const;

    /** @brief Convenience: returns 2xGetRadius(). */
    [[nodiscard]] float GetSize() const;

    /** @brief Sets the (unscaled) radius. */
    void SetRadius(float r);

    /** @brief Point test against the circle in world space. */
    [[nodiscard]] bool CheckPointCollision(const glm::vec2& point) const override;

private:
    [[nodiscard]] ColliderType GetType() const override { return ColliderType::Circle; }

    /** @brief Bounding radius equals the circle radius. */
    [[nodiscard]] float GetBoundingRadius() const override;

    /** @brief Generic collision implemented via double dispatch. */
    [[nodiscard]] bool CheckCollision(const Collider* other) const override;

    [[nodiscard]] bool DispatchAgainst(const CircleCollider& other) const override;
    [[nodiscard]] bool DispatchAgainst(const AABBCollider& other) const override;

    /** @brief Updates scaledRadius from the owner's transform scale when enabled. */
    void SyncWithTransformScale() override;

    /** @brief Draws a circle outline as debug geometry. */
    void DrawDebug(RenderManager* rm, Camera2D* cam, const glm::vec4& color) const override;

    float baseRadius = 0.5f;
    float scaledRadius = 0.5f;
};


/**
 * @brief Axis-aligned bounding-box collider, stored as half-size.
 *
 * @details
 * - Keeps base (unscaled) half-size and a scaled half-size cache.
 * - When transform-scale coupling is enabled, SyncWithTransformScale() updates the effective half-size from the owner's scale.
 */
class AABBCollider : public Collider
{
    friend CircleCollider;
    friend SpatialHashGrid;
public:
    /**
     * @brief Constructs an AABB collider.
     * @param owner Owning object.
     * @param size Full width/height in world units; half-size = size/2.
     */
    AABBCollider(Object* owner, const glm::vec2& size)
        : Collider(owner), baseHalfSize(size / glm::vec2(2)), scaledHalfSize(size / glm::vec2(2)) {
    }

    /** @brief Returns current effective half-size (world units). */
    [[nodiscard]] glm::vec2 GetHalfSize() const;

    /** @brief Returns full size = 2xhalf-size. */
    [[nodiscard]] glm::vec2 GetSize() const;
    /** @brief Sets the (unscaled) full size; half-size is updated accordingly. */
    void SetSize(const glm::vec2& hs);

    /** @brief Point test against the AABB in world space. */
    [[nodiscard]] bool CheckPointCollision(const glm::vec2& point) const override;

private:
    [[nodiscard]] ColliderType GetType() const override { return ColliderType::AABB; }

    /** @brief Bounding radius large enough to contain the AABB. */
    [[nodiscard]] float GetBoundingRadius() const override;

    /** @brief Generic collision implemented via double dispatch. */
    [[nodiscard]] bool CheckCollision(const Collider* other) const override;

    [[nodiscard]] bool DispatchAgainst(const CircleCollider& other) const override;
    [[nodiscard]] bool DispatchAgainst(const AABBCollider& other) const override;

    /** @brief Updates scaledHalfSize from the owner's transform scale when enabled. */
    void SyncWithTransformScale() override;

    /** @brief Draws a box outline as debug geometry. */
    void DrawDebug(RenderManager* rm, Camera2D* cam, const glm::vec4& color) const override;

    glm::vec2 baseHalfSize = { 0.5f, 0.5f };
    glm::vec2 scaledHalfSize = { 0.5f, 0.5f };
};

/** @brief Hash for glm::ivec2 to use as unordered_map key. */
struct Vec2Hash
{
    size_t operator()(const glm::ivec2& v) const
    {
        return std::hash<int>()(v.x * 73856093 ^ v.y * 19349663);
    }
};

/**
 * @brief Spatial hash grid for broad-phase collision candidate generation.
 *
 * @details
 * - cellSize controls the grid resolution (in world units).
 * - Each inserted object is associated with one or more grid cells via GetCell().
 * - ComputeCollisions() visits cell buckets and invokes a callback for candidate pairs.
 */
class SpatialHashGrid
{
    friend ObjectManager;
private:
    /** @brief Removes all buckets and entries. */
    void Clear();
    /** @brief Inserts an object into appropriate cell buckets. */
    void Insert(Object* obj);
    /**
     * @brief Enumerates candidate pairs and calls @p onCollision for each pair.
     * @param onCollision Callback invoked with (a, b) for candidate overlaps.
     */
    void ComputeCollisions(std::function<void(Object*, Object*)> onCollision);
    /** @brief Converts world-space position to a grid cell coordinate. */
    [[nodiscard]] glm::ivec2 GetCell(const glm::vec2& pos) const;
    /** @brief Inserts an object into a specific cell bucket. */
    void InsertToCell(Object* obj, const glm::ivec2& cell);

    int cellSize = 50;
    std::unordered_map<glm::ivec2, std::vector<Object*>, Vec2Hash> grid;
};

/**
 * @brief Registry mapping collision group tags to unique bit masks and back.
 *
 * @details
 * - GetGroupBit(tag): returns an existing bit or allocates the next free bit for a new tag.
 * - GetGroupTag(bit): returns the tag for a bit if known; returns "unknown" otherwise.
 */
class CollisionGroupRegistry
{
    friend Collider;
    friend Object;
private:
    [[nodiscard]] uint32_t GetGroupBit(const std::string& tag);
    [[nodiscard]] std::string GetGroupTag(uint32_t bit) const;
    std::unordered_map<std::string, uint32_t> tagToBit;
    std::unordered_map<uint32_t, std::string> bitToTag;
    uint32_t currentBit = 0;
};
