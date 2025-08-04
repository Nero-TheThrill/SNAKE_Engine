#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

#include "RenderManager.h"

class GameState;
class Object;
struct EngineContext;
class Camera2D;

/**
 * @brief Central manager for handling all Object instances in the engine.
 *
 * @details
 * The ObjectManager is responsible for the creation, lifecycle management,
 * rendering submission, and collision detection of Object-derived instances such as GameObject and TextObject.
 *
 * It maintains internal containers to manage:
 * - `objects`: all active objects
 * - `pendingObjects`: newly added objects that will be initialized in the next update cycle
 * - `rawPtrObjects`: non-owning raw pointers for fast iteration
 * - `objectMap`: tag-based lookup for individual objects
 *
 * Lifecycle is controlled via:
 * - `InitAll()` and `LateInit()` for startup
 * - `UpdateAll()` for per-frame logic and animation update
 * - `EraseDeadObjects()` to remove killed objects (i.e., after `Kill()`)
 * - `FreeAll()` for full cleanup
 *
 * Objects added via `AddObject()` are automatically inserted into pending lists,
 * and are initialized in the next `UpdateAll()` call.
 *
 * Collision is resolved using broad-phase grid and masking system.
 * Each object is inserted into the spatial hash grid and checked using double dispatch.
 *
 * @note
 * This manager assumes ownership of all Object instances.
 *
 * @code
 * auto* obj = objectManager->AddObject(std::make_unique<GameObject>(), "enemy");
 * @endcode
 */
class ObjectManager
{
    friend GameState;
public:

    /**
	 * @brief Adds a new Object to be managed by the engine.
	 *
	 * @details
	 * The Object is added to a pending queue and will be fully initialized
	 * during the next call to UpdateAll(). Ownership is transferred to ObjectManager.
	 *
	 * If a non-empty tag is provided, the object is stored in the objectMap
	 * for lookup via FindByTag(). If a duplicate tag is detected, a warning is logged.
	 *
	 * @param obj The unique_ptr to the Object to be added.
	 * @param tag Optional string tag for retrieval. Defaults to "".
	 *
	 * @return Raw pointer to the newly added object for external reference.
	 *
	 * @note The object will not be updated or drawn until the next frame.
	 * @code
	 * GameObject* enemy = objectManager->AddObject(std::make_unique<GameObject>(), "Enemy01");
	 * @endcode
	 */
    [[maybe_unused]]Object* AddObject(std::unique_ptr<Object> obj, const std::string& tag = "");

	/**
	 * @brief Initializes all pending objects and existing objects.
	 *
	 * @details
	 * This function:
	 * 1. Moves all pending objects into the active list (`objects`)
	 * 2. Initializes them by calling Object::Init()
	 * 3. Inserts their raw pointer into the fast-access list
	 * 4. Updates tag lookup map
	 *
	 * Should be called once during GameState::Init or before the first update.
	 *
	 * @param engineContext Reference to the current engine context.
	 *
	 * @note Automatically clears the pending object queue.
	 */
    void InitAll(const EngineContext& engineContext);

	/**
	 * @brief Updates all active objects in the scene.
	 *
	 * @details
	 * 1. Calls Object::Update() for every alive object.
	 * 2. Adds any newly queued objects.
	 * 3. Removes dead objects (isAlive == false).
	 * 4. Updates internal raw pointer list and tag map.
	 *
	 * This is the main per-frame logic entry point for all objects.
	 *
	 * @param dt Delta time for the frame update.
	 * @param engineContext Reference to the current engine context.
	 */
    void UpdateAll(float dt, const EngineContext& engineContext);

	/**
	 * @brief Submits all active objects to the render pipeline.
	 *
	 * @details
	 * This function submits all currently alive objects stored in rawPtrObjects
	 * to the RenderManager for rendering. Objects will be sorted and drawn based
	 * on their render layer and camera settings.
	 *
	 * Typically called once per frame during GameState rendering.
	 *
	 * @param engineContext The current engine context.
	 * @param camera Pointer to the active camera used for rendering.
	 *
	 * @note Objects marked with ShouldIgnoreCamera() will behave as HUD elements.
	 */
    void DrawAll(const EngineContext& engineContext, Camera2D* camera);

	/**
	 * @brief Submits a custom set of objects to the render pipeline.
	 *
	 * @details
	 * Allows rendering of only a selected list of objects rather than the full object pool.
	 * Useful for debugging, layered rendering, or rendering previews.
	 *
	 * @param engineContext The current engine context.
	 * @param camera The camera to use when rendering.
	 * @param objects A vector of object pointers to render.
	 *
	 * @note Objects must be alive and properly initialized to appear correctly.
	 */
    void DrawObjects(const EngineContext& engineContext, Camera2D* camera, const std::vector<Object*>& objects);

	/**
	 * @brief Submits all alive objects with a specific tag for rendering.
	 *
	 * @details
	 * Searches all currently managed objects for the given tag using FindByTag(),
	 * filters only the alive ones, and submits them to RenderManager for drawing.
	 *
	 * Useful for rendering a subset of tagged objects, like UI groups or effects.
	 *
	 * @param engineContext The current engine context.
	 * @param camera The camera to use for rendering.
	 * @param tag The string tag to match against object tags.
	 */
	void DrawObjectsWithTag(const EngineContext& engineContext, Camera2D* camera, const std::string& tag);

	/**
	 * @brief Finds the first alive object with a given tag.
	 *
	 * @details
	 * Returns a pointer to the first alive object that matches the given tag
	 * using internal `objectMap`. If the object is dead or not found, returns nullptr.
	 *
	 * Tags are assigned via `Object::SetTag()` and stored in the map during `AddObject()`.
	 *
	 * @param tag Tag string to search for.
	 * @return Object* Pointer to the matching object, or nullptr.
	 * @code
	 * Object* player = objectManager.FindByTag("Player");
	 * if (player) player->Kill();
	 * @endcode
	 */
    [[nodiscard]] Object* FindByTag(const std::string& tag) const;

	/**
	 * @brief Finds all alive objects matching the given tag.
	 *
	 * @details
	 * Iterates over all raw pointer objects and pushes all alive ones with the
	 * given tag into the `result` vector. Useful for querying all objects in a group.
	 *
	 * @param tag Tag string to search for.
	 * @param result Output vector to be filled with matching objects.
	 * @code
	 * std::vector<Object*> enemies;
	 * objectManager.FindByTag("Enemy", enemies);
	 * @endcode
	 */
    void FindByTag(const std::string& tag, std::vector<Object*>& result);


	/**
	 * @brief Returns a reference to the collision group registry.
	 *
	 * @details
	 * Provides access to the `CollisionGroupRegistry` used internally for
	 * managing category and mask bit assignments.
	 * Typically used by `Object::SetCollision()` to get bit values.
	 *
	 * @return CollisionGroupRegistry& Reference to registry.
	 * @code
	 * auto& reg = objectManager.GetCollisionGroupRegistry();
	 * uint32_t playerMask = reg.GetGroupBit("Player");
	 * @endcode
	 */
    [[nodiscard]] CollisionGroupRegistry& GetCollisionGroupRegistry() { return collisionGroupRegistry; }
private:
	/**
	 * @brief Frees and clears all managed objects.
	 *
	 * @details
	 * Calls `Free()` and `LateFree()` on all currently managed objects, then
	 * clears internal containers: `objects`, `objectMap`, and `rawPtrObjects`.
	 * After this call, the manager no longer holds any `Object` instances.
	 *
	 * Typically used during a full game state unload or shutdown process.
	 *
	 * @param engineContext Engine-wide services used for cleanup (e.g., rendering).
	 * @code
	 * objectManager.FreeAll(engineContext);
	 * @endcode
	 */
	void FreeAll(const EngineContext& engineContext);

	/**
	 * @brief Draws debug visuals for all live and visible object colliders.
	 *
	 * @details
	 * Calls Collider::DrawDebug() on all active objects with a valid Collider and visibility enabled.
	 * Used during development to visualize collision bounds.
	 *
	 * @param rm Pointer to the RenderManager used for debug rendering.
	 * @param cam Pointer to the active Camera2D.
	 *
	 * @note This does not affect actual gameplay rendering and is typically enabled via debug flags.
	 *
	 * @code
	 * objectManager.DrawColliderDebug(renderManager, camera);
	 * @endcode
	 */
	void DrawColliderDebug(RenderManager* rm, Camera2D* cam);

	/**
	 * @brief Performs broad-phase and narrow-phase collision detection for all collidable objects.
	 *
	 * @details
	 * Uses a spatial hash grid to perform efficient broad-phase culling, followed by narrow-phase
	 * collision checks using each object's Collider. Collision responses are invoked via OnCollision().
	 *
	 * Objects must have valid Collider instances and configured collisionCategory/mask via SetCollision().
	 */
	void CheckCollision();

	/**
	 * @brief Moves and initializes all pending objects.
	 *
	 * @details
	 * Transfers all objects from `pendingObjects` into the main `objects` list.
	 * Each object is initialized via `Init()` and optionally `LateInit()`, then:
	 * - inserted into `objectMap` if it has a tag
	 * - added to `rawPtrObjects` for iteration
	 *
	 * This function is automatically called inside `UpdateAll()` once per frame.
	 *
	 * @param engineContext Reference to the engine services passed to each object.
	 */
    void AddAllPendingObjects(const EngineContext& engineContext);

	/**
	 * @brief Removes and cleans up dead objects from the system.
	 *
	 * @details
	 * Iterates over the `objects` list and removes any object where `IsAlive()` returns false.
	 * Before removal, it calls `Free()` and `LateFree()` on the object.
	 * The `objectMap` and `rawPtrObjects` are also updated accordingly.
	 *
	 * This function is called each frame within `UpdateAll()`.
	 *
	 * @param engineContext Engine context used during the object cleanup process.
	 */
    void EraseDeadObjects(const EngineContext& engineContext);

    std::vector<std::unique_ptr<Object>> objects;
    std::vector<std::unique_ptr<Object>> pendingObjects;
    std::unordered_map<std::string, Object*> objectMap;
    std::vector<Object*> rawPtrObjects;
    SpatialHashGrid broadPhaseGrid;
    CollisionGroupRegistry collisionGroupRegistry;
};
