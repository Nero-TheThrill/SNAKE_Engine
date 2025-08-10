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
 * @brief Owns and orchestrates a collection of Object instances.
 *
 * @details
 * - Accepts ownership of new objects, keeps a tag->object map, and exposes
 *   utility queries/draw helpers.
 * - Provides lifecycle passes (Init/Update/Draw/Free) over managed objects.
 * - Offers simple collision checking and collider debug-drawing.
 */
class ObjectManager
{
    friend GameState;
public:
    /**
     * @brief Adds an object to be managed and (optionally) tags it.
     *
     * @details
     * Ownership is transferred to ObjectManager. A non-empty @p tag is stored
     * for lookups. Returns a raw pointer to the managed instance.
     *
     * @param obj Newly created object (unique ownership).
     * @param tag Optional identifier for lookups.
     * @return Raw pointer to the added object.
     */
    [[maybe_unused]] Object* AddObject(std::unique_ptr<Object> obj, const std::string& tag = "");

    /**
     * @brief Runs initialization pass for managed objects.
     * @param engineContext Engine systems bundle.
     */
    void InitAll(const EngineContext& engineContext);

    /**
     * @brief Runs per-frame update pass for managed objects.
     * @param dt Delta time in seconds.
     * @param engineContext Engine systems bundle.
     */
    void UpdateAll(float dt, const EngineContext& engineContext);

    /**
     * @brief Submits all managed objects for rendering.
     * @param engineContext Engine systems bundle.
     */
    void DrawAll(const EngineContext& engineContext);

    /**
     * @brief Submits only the given objects for rendering.
     * @param engineContext Engine systems bundle.
     * @param objects List of objects to draw.
     */
    void DrawObjects(const EngineContext& engineContext, const std::vector<Object*>& objects);

    /**
     * @brief Submits objects that match the given tag for rendering.
     * @param engineContext Engine systems bundle.
     * @param tag Tag to match.
     */
    void DrawObjectsWithTag(const EngineContext& engineContext, const std::string& tag);

    /**
     * @brief Runs teardown pass for managed objects and releases ownership.
     * @param engineContext Engine systems bundle.
     */
    void FreeAll(const EngineContext& engineContext);

    /**
     * @brief Finds a single object by exact tag.
     * @param tag Tag to look up.
     * @return Matching object pointer, or nullptr if not found.
     */
    [[nodiscard]] Object* FindByTag(const std::string& tag) const;

    /**
     * @brief Collects all alive objects whose tag matches @p tag.
     * @param tag Tag to match.
     * @param result Output vector to append matches into (not cleared).
     */
    void FindByTag(const std::string& tag, std::vector<Object*>& result);

    /**
     * @brief Performs collision checks among managed objects that have colliders.
     *
     * @details
     * Uses the internal spatial structure and group registry to determine
     * candidate pairs and apply checks per current settings.
     */
    void CheckCollision();

    /**
     * @brief Access to the collision group registry.
     * @return Reference to the registry.
     */
    [[nodiscard]] CollisionGroupRegistry& GetCollisionGroupRegistry() { return collisionGroupRegistry; }

    /**
     * @brief Returns the current list of managed objects as raw pointers.
     * @return A copy of the internal raw-pointer list.
     */
    [[nodiscard]] std::vector<Object*> GetAllRawPtrObjects() { return rawPtrObjects; }

private:
    /**
     * @brief Integrates any pending objects into the active list.
     * @param engineContext Engine systems bundle.
     * @note Internal.
     */
    void AddAllPendingObjects(const EngineContext& engineContext);

    /**
     * @brief Removes objects that are no longer alive from management.
     * @param engineContext Engine systems bundle.
     * @note Internal.
     */
    void EraseDeadObjects(const EngineContext& engineContext);

    /**
     * @brief Draws collider debug visuals for alive & visible objects.
     * @param rm Render manager to use.
     * @param cam Camera for debug rendering (nullable for screen space).
     */
    void DrawColliderDebug(RenderManager* rm, Camera2D* cam);

    std::vector<std::unique_ptr<Object>> objects;
    std::vector<std::unique_ptr<Object>> pendingObjects;
    std::unordered_map<std::string, Object*> objectMap;
    std::vector<Object*> rawPtrObjects;
    SpatialHashGrid broadPhaseGrid;
    CollisionGroupRegistry collisionGroupRegistry;
};
