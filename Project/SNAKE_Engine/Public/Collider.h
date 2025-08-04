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

enum class ColliderType
{
    None,
    Circle,
    AABB
};

/**
 * @brief Base class for all 2D collider shapes (circle, AABB) used in collision detection.
 *
 * @details
 * Collider is an abstract base class representing a 2D collision component.
 * Each collider is owned by an Object and stores its world position independently from the transform.
 *
 * Subclasses (CircleCollider, AABBCollider) implement specific shape logic and double dispatch collision resolution.
 * This class supports spatial hashing, debug drawing, and transform scaling.
 *
 * Collider instances are managed internally by ObjectManager and should not be created directly by users.
 *
 * @note Collision checks use a double dispatch system via CheckCollision and DispatchAgainst().
 */
class Collider
{
    friend ObjectManager;
    friend CircleCollider;
    friend AABBCollider;
    friend SpatialHashGrid;
public:
    Collider() = delete;

    /**
     * @brief Constructs a collider and binds it to the specified owner object.
     *
     * @details
     * This constructor initializes the collider and stores a pointer to its owning Object.
     * The world position is initialized to zero. Transform scaling is enabled by default.
     *
     * Each collider must be associated with exactly one Object at creation.
     *
     * @param owner_ Pointer to the Object that owns this collider.
     */
    Collider(Object* owner_) : owner(owner_), worldPosition(){}

    /**
     * @brief Virtual destructor for the Collider base class.
     *
     * @details
     * Ensures proper cleanup of derived collider types via base pointer deletion.
     */
    virtual ~Collider() = default;

    /**
     * @brief Enables or disables transform scaling for this collider.
     *
     * @details
     * When enabled (default), the collider will scale its shape (e.g., radius or size)
     * based on the owner's transform scale.
     * When disabled, the collider maintains a fixed size regardless of transform.
     *
     * @param use Whether to apply transform scaling to the collider.
     */
    void SetUseTransformScale(bool use) { useTransformScale = use; }

    /**
     * @brief Checks whether transform scaling is enabled for this collider.
     *
     * @return true if transform scale affects the collider size, false otherwise.
     */
    [[nodiscard]] bool IsUsingTransformScale() const { return useTransformScale; }

    /**
     * @brief Sets the world-space center position of the collider.
     *
     * @details
     * This position determines where the collider is located in the game world and
     * is typically synchronized with the object's transform position each frame.
     *
     * For scale-dependent colliders, this position should be updated after scaling.
     *
     * @param pos World-space position (in pixels or units).
     */
    void SetWorldPosition(const glm::vec2& pos) { worldPosition = pos; }

    /**
     * @brief Returns the current world-space center position of the collider.
     *
     * @return A reference to the collider's world position vector.
     */
    const glm::vec2& GetWorldPosition() const { return worldPosition; }

    /**
     * @brief Checks whether a given point lies inside this collider's shape.
     *
     * @details
     * This is a pure virtual function implemented by concrete collider types such as
     * CircleCollider and AABBCollider.
     *
     * Used for point-based hit detection, such as mouse clicks or ray hits.
     *
     * @param point The point to test, in world space.
     * @return true if the point is inside the collider, false otherwise.
     */
    virtual bool CheckPointCollision(const glm::vec2& point) const = 0;

protected:
    /**
     * @brief Returns the object that owns this collider.
     *
     * @details
     * Each collider is attached to a single Object instance.
     * This is used internally for resolving collisions, retrieving transform data, or invoking callbacks.
     *
     * @return Pointer to the Object that owns this collider.
     */
    [[nodiscard]] Object* GetOwner() const { return owner; }

    /**
     * @brief Returns the type of this collider (e.g., Circle or AABB).
     *
     * @details
     * This pure virtual function is implemented by derived classes to identify their collider shape type.
     * Used in the double-dispatch collision resolution system.
     *
     * @return The ColliderType enum value indicating the shape.
     */
    [[nodiscard]] virtual ColliderType GetType() const = 0;

    /**
     * @brief Returns the approximate bounding radius of the collider.
     *
     * @details
     * This radius is used as a coarse heuristic for early rejection in collision testing.
     * For a circle collider, it returns the actual radius. For an AABB, it returns the diagonal extent.
     *
     * @return Bounding radius in world units.
     */
    [[nodiscard]] virtual float GetBoundingRadius() const = 0;

    /**
     * @brief Performs a collision check against another collider.
     *
     * @details
     * This is the primary virtual entry point for collision testing.
     * It uses a double dispatch pattern to resolve the correct collision logic
     * based on the type of the other collider.
     *
     * The caller should ensure that both colliders are valid and positioned in world space.
     *
     * @param other The other collider to test against.
     * @return true if the colliders intersect; false otherwise.
     */
    [[nodiscard]] virtual bool CheckCollision(const Collider* other) const = 0;

    /**
     * @brief Performs collision resolution between this collider and a CircleCollider.
     *
     * @details
     * This function is part of a double dispatch system and is implemented by each collider shape.
     * It determines whether this collider overlaps with a circle-shaped collider using precise logic.
     *
     * Called internally by CheckCollision() after type routing.
     *
     * @param other Reference to the other CircleCollider to test against.
     * @return true if a collision is detected; false otherwise.
     */
    [[nodiscard]] virtual bool DispatchAgainst(const CircleCollider& other) const = 0;

    /**
     * @brief Performs collision resolution between this collider and an AABBCollider.
     *
     * @details
     * Called as part of the double dispatch system to perform shape-specific collision logic
     * between this collider and a rectangular AABB collider.
     *
     * Invoked internally by CheckCollision() after type resolution.
     *
     * @param other Reference to the AABBCollider to test against.
     * @return true if the shapes intersect; false otherwise.
     */
    [[nodiscard]] virtual bool DispatchAgainst(const AABBCollider& other) const = 0;

    /**
     * @brief Updates internal size values based on the owner's transform scale.
     *
     * @details
     * If transform scaling is enabled, this function recalculates the collider's
     * effective radius (for CircleCollider) or size (for AABBCollider) using the
     * owner's current scale.
     *
     * Called automatically during collider update cycles.
     */
    virtual void SyncWithTransformScale() = 0;

    /**
     * @brief Renders a debug visualization of the collider.
     *
     * @details
     * This function draws the collider's shape using a simple outline or fill color,
     * typically for debugging or development purposes.
     *
     * Called by ObjectManager when debug draw mode is enabled.
     *
     * @param rm RenderManager pointer to handle drawing.
     * @param cam Active camera for projection.
     * @param color Optional color for debug lines (default: red).
     */
    virtual void DrawDebug(RenderManager* rm, Camera2D* cam, const glm::vec4& color = { 1,0,0,1 }) const = 0;

    Object* owner;
    bool useTransformScale = true;
    glm::vec2 worldPosition;
};

/**
 * @brief A circular 2D collider component.
 *
 * @details
 * CircleCollider is a concrete subclass of Collider representing a circle shape.
 * It stores both base radius and scaled radius, and supports scaling via transform.
 *
 * It provides collision resolution against both Circle and AABB shapes.
 * The center of the collider is world-aligned based on the owning object's transform.
 *
 * @note Used internally by ObjectManager. Do not instantiate directly.
 */
class CircleCollider : public Collider
{
    friend AABBCollider;
    friend SpatialHashGrid;
public:
    /**
     * @brief Constructs a circle collider with the specified diameter.
     *
     * @details
     * Initializes the collider using the given size value (diameter),
     * and computes the base radius as size / 2. The scaled radius is also
     * initialized to the same value and updated later if transform scaling is enabled.
     *
     * @param owner Pointer to the Object that owns this collider.
     * @param size Diameter of the circle in world units (not radius).
     */
    CircleCollider(Object* owner, float size)
        : Collider(owner), baseRadius(size/2.f), scaledRadius(size/2.f) {
    }

    /**
     * @brief Returns the currently scaled radius of the circle.
     *
     * @details
     * This value reflects any transform scaling if enabled. Otherwise, it equals the base radius.
     *
     * @return Radius in world units.
     */
    [[nodiscard]] float GetRadius() const;

    /**
     * @brief Returns the full diameter of the circle.
     *
     * @details
     * Equivalent to GetRadius() * 2. This represents the full width of the circle.
     *
     * @return Diameter in world units.
     */
    [[nodiscard]] float GetSize() const;

    /**
     * @brief Sets the base radius of the circle collider.
     *
     * @details
     * Updates the baseRadius to the given value and resets the scaledRadius to match.
     * If transform scaling is enabled, the scaledRadius will be recalculated later.
     *
     * @param r New radius to assign.
     */
    void SetRadius(float r);

    /**
     * @brief Checks if a given point is inside the circle collider.
     *
     * @details
     * Calculates the distance between the point and the circle's center.
     * If the distance is less than or equal to the scaled radius, a collision is detected.
     *
     * Used for mouse picking, hit tests, etc.
     *
     * @param point World-space position to test.
     * @return true if point is inside the circle; false otherwise.
     */
    [[nodiscard]] bool CheckPointCollision(const glm::vec2& point) const override;

private:
    /**
     * @brief Identifies this collider as a circle shape.
     *
     * @details
     * This function overrides Collider::GetType() and always returns ColliderType::Circle.
     * Used internally for dispatching and filtering by collider shape.
     *
     * @return ColliderType::Circle
     */
    [[nodiscard]] ColliderType GetType() const override { return ColliderType::Circle; }

    /**
     * @brief Returns the outer radius used for bounding sphere checks.
     *
     * @details
     * For a circle collider, the bounding radius is simply the current scaled radius.
     * This value is used for fast early-out collision rejection.
     *
     * @return Bounding radius in world units.
     */
    [[nodiscard]] float GetBoundingRadius() const override;

    /**
     * @brief Checks collision with another collider using double dispatch.
     *
     * @details
     * This function overrides Collider::CheckCollision() and dispatches
     * to the appropriate DispatchAgainst() overload based on the other collider's type.
     *
     * Supports Circle-vs-Circle and Circle-vs-AABB collision detection.
     *
     * @param other Pointer to the other collider.
     * @return true if collision is detected; false otherwise.
     */
    [[nodiscard]] bool CheckCollision(const Collider* other) const override;

    /**
     * @brief Performs precise collision detection between two circle colliders.
     *
     * @details
     * Compares the distance between the two circle centers to the sum of their radii.
     * If the distance is less than or equal to the combined radii, a collision is detected.
     *
     * This is called by the other collider through double dispatch.
     *
     * @param other The other CircleCollider to test against.
     * @return true if the circles intersect; false otherwise.
     */
    [[nodiscard]] bool DispatchAgainst(const CircleCollider& other) const override;

    /**
     * @brief Performs precise collision detection between a circle and an AABB.
     *
     * @details
     * Projects the circle center onto the nearest point on the AABB boundary,
     * then checks whether the distance to that point is within the circle's radius.
     *
     * Called during double dispatch by the AABBCollider.
     *
     * @param other The AABBCollider to test against.
     * @return true if the shapes intersect; false otherwise.
     */
    [[nodiscard]] bool DispatchAgainst(const AABBCollider& other) const override;

    /**
     * @brief Recalculates the scaled radius based on the owner's transform scale.
     *
     * @details
     * If transform scaling is enabled, this function multiplies the base radius
     * by the owner's uniform scale to update the scaledRadius field.
     * Otherwise, scaledRadius is set equal to baseRadius.
     *
     * Called automatically each frame during collider updates.
     */
    void SyncWithTransformScale() override;

    /**
     * @brief Renders the circle collider for debug visualization.
     *
     * @details
     * Uses the RenderManager to draw a circle outline at the collider's world position.
     * The radius used is the scaledRadius. Only visible if debug rendering is enabled.
     *
     * @param rm Pointer to the RenderManager handling the draw call.
     * @param cam Active camera for converting world-to-screen space.
     * @param color Optional RGBA color (default: red).
     */
    void DrawDebug(RenderManager* rm, Camera2D* cam, const glm::vec4& color) const override;

    float baseRadius = 0.5f;
    float scaledRadius = 0.5f;
};

/**
 * @brief A 2D collider with axis-aligned bounding box shape.
 *
 * @details
 * AABBCollider stores half-size in both base and scaled forms and performs
 * collision detection using axis-aligned rectangular bounds.
 *
 * Supports transform-based scaling and collision with both AABB and Circle colliders.
 * The center is aligned with the owner's world position.
 */
class AABBCollider : public Collider
{
    friend CircleCollider;
    friend SpatialHashGrid;
public:
    /**
     * @brief Constructs an AABB collider with the specified size.
     *
     * @details
     * The given size vector represents the full width and height of the box.
     * Both baseHalfSize and scaledHalfSize are initialized as half of the input size.
     * The scaled size will be updated later if transform scaling is applied.
     *
     * @param owner Pointer to the Object that owns this collider.
     * @param size Full size of the AABB (width and height in world units).
     */
    AABBCollider(Object* owner, const glm::vec2& size)
        : Collider(owner), baseHalfSize(size/glm::vec2(2)), scaledHalfSize(size / glm::vec2(2)) {
    }

    /**
     * @brief Returns the current scaled half-size of the AABB.
     *
     * @details
     * The half-size represents half the width and height from the center.
     * This value includes any transform scaling applied.
     *
     * @return Half-size vector in world units.
     */
    [[nodiscard]] glm::vec2 GetHalfSize() const;

    /**
     * @brief Returns the full width and height of the AABB.
     *
     * @details
     * Computed as GetHalfSize() * 2. Represents the total size of the box.
     *
     * @return Full size vector in world units.
     */
    [[nodiscard]] glm::vec2 GetSize() const;

    /**
     * @brief Sets a new base size for the AABB collider.
     *
     * @details
     * The input is interpreted as full width and height.
     * Internally, the baseHalfSize and scaledHalfSize are updated to size / 2.
     *
     * This resets any previous size and overrides scaling results.
     *
     * @param hs New full size vector (width and height in world units).
     */
    void SetSize(const glm::vec2& hs);

    /**
     * @brief Checks if a point lies inside the AABB bounds.
     *
     * @details
     * Compares the point's position against the min/max extents of the box,
     * based on the collider's center (world position) and scaled half-size.
     *
     * Useful for point picking, hitboxes, etc.
     *
     * @param point The world-space point to check.
     * @return true if the point is inside the AABB; false otherwise.
     */
    [[nodiscard]] bool CheckPointCollision(const glm::vec2& point) const override;

private:
    /**
     * @brief Identifies this collider as an AABB shape.
     *
     * @details
     * This function overrides Collider::GetType() and always returns ColliderType::AABB.
     * Used internally to route collision logic via double dispatch.
     *
     * @return ColliderType::AABB
     */
    [[nodiscard]] ColliderType GetType() const override { return ColliderType::AABB; }

    /**
     * @brief Returns the bounding radius of the AABB for early collision rejection.
     *
     * @details
     * The radius is computed as the length of the scaled half-size vector,
     * effectively forming a bounding circle around the box.
     *
     * Used for approximate culling before precise collision checks.
     *
     * @return Bounding radius in world units.
     */
    [[nodiscard]] float GetBoundingRadius() const override;

    /**
     * @brief Performs collision detection with another collider using double dispatch.
     *
     * @details
     * Determines the type of the other collider and dispatches to the appropriate
     * DispatchAgainst() overload to execute shape-specific collision logic.
     *
     * Supports AABB-vs-AABB and AABB-vs-Circle collisions.
     *
     * @param other The other collider to test against.
     * @return true if the two colliders intersect; false otherwise.
     */
    [[nodiscard]] bool CheckCollision(const Collider* other) const override;

    /**
     * @brief Checks collision between this AABB and a CircleCollider.
     *
     * @details
     * Projects the circle center onto the nearest point on the AABB boundary,
     * and compares the distance to the circle's radius.
     *
     * This function is invoked by the CircleCollider using double dispatch.
     *
     * @param other The CircleCollider to test against.
     * @return true if the circle intersects with the AABB; false otherwise.
     */
    [[nodiscard]] bool DispatchAgainst(const CircleCollider& other) const override;

    /**
     * @brief Performs collision detection between two axis-aligned bounding boxes (AABBs).
     *
     * @details
     * Compares the projections of both AABBs along the X and Y axes.
     * If their intervals overlap on both axes, a collision is detected.
     *
     * Used for rectangle-rectangle intersection tests.
     *
     * @param other The other AABBCollider to test against.
     * @return true if the AABBs intersect; false otherwise.
     */
    [[nodiscard]] bool DispatchAgainst(const AABBCollider& other) const override;

    /**
     * @brief Recalculates the scaled radius based on the owner's transform scale.
     *
     * @details
     * If transform scaling is enabled, this function multiplies the base radius
     * by the owner's uniform scale to update the scaledRadius field.
     * Otherwise, scaledRadius is set equal to baseRadius.
     *
     * Called automatically each frame during collider updates.
     */
    void SyncWithTransformScale() override;

    /**
     * @brief Renders the AABB for debug visualization.
     *
     * @details
     * Uses RenderManager to draw a rectangle centered at the collider's world position,
     * with extents defined by the scaled half-size. Visible only when debug rendering is enabled.
     *
     * @param rm RenderManager instance used to submit draw calls.
     * @param cam Camera used to convert world coordinates to screen space.
     * @param color Optional RGBA color for the debug outline (default: red).
     */
    void DrawDebug(RenderManager* rm, Camera2D* cam, const glm::vec4& color) const override;

    glm::vec2 baseHalfSize = { 0.5f, 0.5f };
    glm::vec2 scaledHalfSize = { 0.5f, 0.5f };
};

struct Vec2Hash
{
    size_t operator()(const glm::ivec2& v) const
    {
        return std::hash<int>()(v.x * 73856093 ^ v.y * 19349663);
    }
};

/**
 * @brief A spatial hash grid used to accelerate broad-phase collision detection.
 *
 * @details
 * SpatialHashGrid divides the 2D world into fixed-size grid cells and maps objects
 * into those cells based on their collider positions.
 *
 * It reduces the number of collision checks by limiting narrow-phase testing
 * to objects that share or neighbor the same cell.
 *
 * Used internally by ObjectManager and the collision system.
 */
class SpatialHashGrid
{
public:
    /**
     * @brief Clears all cells in the spatial hash grid.
     *
     * @details
     * Removes all object entries from every grid cell, effectively resetting the grid state.
     * This should be called before re-inserting updated objects each frame.
     *
     * @note
     * Typically called at the start of each frame by ObjectManager before collision computation.
     */
    void Clear();

    /**
     * @brief Inserts an object into the spatial grid based on its collider's position.
     *
     * @details
     * Computes the object's grid cell coordinates using its world position,
     * then stores the object pointer in that cell.
     *
     * The object must have a valid Collider, and its world position should already be synced.
     * This function supports coarse spatial partitioning for broad-phase collision optimization.
     *
     * @param obj Pointer to the object to insert.
     *
     * @note
     * Called once per object per frame before ComputeCollisions().
     */
    void Insert(Object* obj);

    /**
     * @brief Checks for potential collisions between objects within the same grid cells.
     *
     * @details
     * Iterates over each populated cell and performs pairwise narrow-phase collision checks
     * between objects in that cell.
     *
     * If a collision is detected, the provided callback function is invoked with both objects.
     * This approach significantly reduces the number of collision checks versus a full O(n^2) pass.
     *
     * @param onCollision Callback function to invoke upon confirmed collision between two objects.
     *
     * @note
     * This is the main entry point for broad-phase + narrow-phase collision handling.
     */
    void ComputeCollisions(std::function<void(Object*, Object*)> onCollision);

private:
    /**
     * @brief Computes the grid cell coordinates corresponding to a world position.
     *
     * @details
     * Converts a 2D world-space position into integer grid cell indices by dividing
     * by the cell size and flooring the result. This determines which spatial cell
     * the position falls into.
     *
     * Used internally when inserting objects into the grid.
     *
     * @param pos World-space position of the object.
     * @return Integer grid coordinates (cellX, cellY).
     */
    [[nodiscard]] glm::ivec2 GetCell(const glm::vec2& pos) const;

    /**
     * @brief Inserts an object pointer into a specific grid cell.
     *
     * @details
     * Adds the given object to the internal grid map at the specified cell coordinate.
     * This function assumes the caller has already determined the correct cell.
     *
     * Used internally by Insert(), after calling GetCell().
     *
     * @param obj Object pointer to insert.
     * @param cell Grid cell coordinates to insert into.
     */
    void InsertToCell(Object* obj, const glm::ivec2& cell);

    int cellSize = 50; 
    std::unordered_map<glm::ivec2, std::vector<Object*>, Vec2Hash> grid;
};

/**
 * @brief Manages collision groups using named tags and bitmasks.
 *
 * @details
 * This utility allows mapping string-based collision group tags to unique bit values,
 * which can then be used to construct collision layers, filters, or masks.
 *
 * Useful for implementing collision matrix logic such as "Player only collides with Enemy".
 */
class CollisionGroupRegistry
{
public:
    /**
     * @brief Retrieves or assigns a unique bitmask for a given collision group tag.
     *
     * @details
     * This function maps a string-based collision group name (e.g., "Player", "Enemy") to a unique bit
     * in a 32-bit integer. If the tag is new, the next available bit (1 << N) is automatically assigned.
     *
     * This function is not used directly by users. It is called internally by Object::SetCollision(),
     * which converts user-provided tags into bitmask values for collision category and mask.
     *
     * @param tag A collision group tag string to resolve.
     * @return A bitmask (1 << N) representing the unique group.
     *
     * @note
     * The number of unique groups is limited to 32 (one per bit).
     * This function is only called internally by ObjectManager via SetCollision().
     */
    [[nodiscard]] uint32_t GetGroupBit(const std::string& tag);

    /**
     * @brief Returns the collision group tag associated with a given bitmask.
     *
     * @details
     * Performs a reverse lookup from a single-bit mask (1 << N) to the string tag originally registered.
     * This is primarily useful for debugging, logging, or editor UI where human-readable names are needed.
     *
     * If the bit is not registered, an empty string is returned.
     *
     * @param bit A bitmask value representing a collision group.
     * @return The string tag associated with the bit, or empty if not found.
     *
     * @note
     * This function is used internally. Users typically do not need to call it.
     */
    [[nodiscard]] std::string GetGroupTag(uint32_t bit) const;

private:
    std::unordered_map<std::string, uint32_t> tagToBit;
    std::unordered_map<uint32_t, std::string> bitToTag;
    uint32_t currentBit = 0;
};