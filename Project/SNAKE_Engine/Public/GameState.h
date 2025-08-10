#pragma once
#include "CameraManager.h"
#include "EngineContext.h"
#include "ObjectManager.h"
#include "SNAKE_Engine.h"
class StateManager;
struct EngineContext;

/**
 * @brief Base class for a game state (scene/screen).
 *
 * @details
 * - Owns an ObjectManager and a CameraManager.
 * - Override the protected virtual hooks (Init/LateInit/Update/LateUpdate/Load/Draw/Free/Unload)
 *   to implement state behavior.
 * - Internal "System*" methods coordinate the call order and are used by StateManager.
 */
class GameState
{
    friend StateManager;

public:
    virtual ~GameState() = default;

    /**
     * @brief Access this state's ObjectManager.
     */
    [[nodiscard]] virtual ObjectManager& GetObjectManager() { return objectManager; }

    /**
     * @brief Access this state's CameraManager.
     */
    [[nodiscard]] CameraManager& GetCameraManager() { return cameraManager; }

    /**
     * @brief Returns the currently active camera (may be nullptr).
     */
    [[nodiscard]] Camera2D* GetActiveCamera() const { return cameraManager.GetActiveCamera(); }

    /**
     * @brief Activates a camera by tag.
     * @param tag Camera tag registered in CameraManager.
     */
    void SetActiveCamera(const std::string& tag) { cameraManager.SetActiveCamera(tag); }

protected:
    /**
     * @brief Called before objects are initialized. Override to set up state resources.
     */
    virtual void Init([[maybe_unused]] const EngineContext& engineContext) {}

    /**
     * @brief Called after Init() and after ObjectManager.InitAll(). Override as needed.
     */
    virtual void LateInit([[maybe_unused]] const EngineContext& engineContext) {}
    /**
     * @brief Per-frame update. Default: updates all managed objects.
     * @param dt Delta time in seconds.
     */
    virtual void Update(float dt, [[maybe_unused]] const EngineContext& engineContext) { objectManager.UpdateAll(dt, engineContext); }

    /**
     * @brief Post-update hook for per-frame logic after Update().
     */
    virtual void LateUpdate([[maybe_unused]] float dt, [[maybe_unused]] const EngineContext& engineContext) {}

    /**
     * @brief Load assets or persistent data. Called by SystemLoad().
     */
    virtual void Load([[maybe_unused]] const EngineContext& engineContext) {}

    /**
     * @brief Per-frame draw. Default: draws all managed objects.
     */
    virtual void Draw([[maybe_unused]] const EngineContext& engineContext)
    {
        objectManager.DrawAll(engineContext);
    }

    /**
     * @brief Free transient resources (before objects are freed).
     */
    virtual void Free([[maybe_unused]] const EngineContext& engineContext) {}

    /**
     * @brief Unload assets or persistent data. Called by SystemUnload().
     */
    virtual void Unload([[maybe_unused]] const EngineContext& engineContext) {}

    /**
     * @brief Restarts the state by calling SystemFree() then SystemInit().
     */
    void Restart(const EngineContext& engineContext)
    {
        SystemFree(engineContext);
        SystemInit(engineContext);
    }

    ObjectManager objectManager;   ///< Owned object manager for this state.
    CameraManager cameraManager;   ///< Owned camera manager for this state.

private:
    /**
     * @brief INTERNAL: Calls Load(). Used by StateManager.
     */
    virtual void SystemLoad(const EngineContext& engineContext)
    {
        Load(engineContext);
    }

    /**
     * @brief INTERNAL: Runs Init(), ObjectManager.InitAll(), LateInit(), then adds pending objects.
     */
    virtual void SystemInit(const EngineContext& engineContext)
    {
        Init(engineContext);
        objectManager.InitAll(engineContext);
        LateInit(engineContext);
        objectManager.AddAllPendingObjects(engineContext);
    }

    /**
     * @brief INTERNAL: Runs Update(), collision check, optional collider debug draw, then LateUpdate().
     *
     * @details
     * - Always calls objectManager.CheckCollision().
     * - If engineContext.engine->ShouldRenderDebugDraws() is true, calls
     *   objectManager.DrawColliderDebug(engineContext.renderManager, cameraManager.GetActiveCamera()).
     */
    virtual void SystemUpdate(float dt, const EngineContext& engineContext)
    {
        Update(dt, engineContext);

        objectManager.CheckCollision();
        if (engineContext.engine->ShouldRenderDebugDraws())
            objectManager.DrawColliderDebug(engineContext.renderManager, cameraManager.GetActiveCamera());

        LateUpdate(dt, engineContext);
    }

    /**
     * @brief INTERNAL: Calls Free() then ObjectManager.FreeAll().
     */
    virtual void SystemFree(const EngineContext& engineContext)
    {
        Free(engineContext);
        objectManager.FreeAll(engineContext);
    }

    /**
     * @brief INTERNAL: Calls Unload().
     */
    virtual void SystemUnload(const EngineContext& engineContext)
    {
        Unload(engineContext);
    }
};
