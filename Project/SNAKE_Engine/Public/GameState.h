#pragma once
#include "CameraManager.h"
#include "EngineContext.h"
#include "ObjectManager.h"
#include "SNAKE_Engine.h"
class StateManager;
struct EngineContext;

/**
 * @brief Base class for representing a single game state or scene.
 *
 * @details
 * GameState defines the standard lifecycle for a self-contained game state such as a title screen,
 * gameplay level, or pause menu. It encapsulates its own ObjectManager and CameraManager, and provides
 * virtual methods for initialization, updates, rendering, loading, and cleanup.
 *
 * The engine's StateManager controls which GameState is active and invokes its lifecycle methods:
 * - Load() -> Init() -> Update() / Draw() -> Free() -> Unload()
 *
 * Users should inherit from GameState and override these methods to define scene-specific behavior.
 * ObjectManager and CameraManager are scoped per state, allowing clean separation of logic.
 *
 * @note
 * System-level lifecycle methods like SystemInit() and SystemUpdate() are managed by StateManager and
 * should not be overridden directly.
 *
 * @code
 * class Level1State : public GameState
 * {
 *     void Init(const EngineContext& engineContext) override
 *     {
 *         objectManager.AddObject(...);
 *     }
 *
 *     void Update(float dt, const EngineContext& engineContext) override
 *     {
 *         // per-frame logic
 *     }
 * };
 * @endcode
 */
class GameState
{
    friend StateManager;

public:
    /**
     * @brief Virtual destructor for safe polymorphic deletion.
     *
     * @details
     * Ensures proper cleanup when deleting GameState-derived instances through base pointers.
     * Subclasses may implement additional cleanup in Free() or Unload().
     */
    virtual ~GameState() = default;

    /**
    * @brief Returns a reference to this state's ObjectManager.
    *
    * @details
    * The ObjectManager manages all game objects for this state. Objects are
    * updated, drawn, and cleaned up according to the state's lifecycle.
    *
    * This manager is scoped per-state and is automatically handled by the engine.
    *
    * @return Reference to the state's ObjectManager.
    */
    [[nodiscard]] virtual ObjectManager& GetObjectManager() { return objectManager; }

    /**
     * @brief Returns a reference to this state's CameraManager.
     *
     * @details
     * Provides access to camera registration, screen resizing, and active camera control.
     * Each GameState owns its own CameraManager to maintain isolated camera contexts.
     *
     * @return Reference to the state's CameraManager.
     */
    [[nodiscard]] CameraManager& GetCameraManager() { return cameraManager; }

    /**
     * @brief Returns the currently active camera for this state.
     *
     * @details
     * Returns the camera selected via SetActiveCamera(). This is used by the rendering
     * system and culling logic to determine the current viewpoint.
     *
     * @return Pointer to the active Camera2D; may be nullptr if unset.
     */
    [[nodiscard]] Camera2D* GetActiveCamera() const { return cameraManager.GetActiveCamera(); }

    /**
     * @brief Sets the active camera for this state.
     *
     * @details
     * Changes which camera will be used for rendering and visibility checks.
     * If the tag is invalid, this call has no effect.
     *
     * @param tag Tag name of the camera to activate.
     *
     * @code
     * SetActiveCamera("main_camera");
     * @endcode
     */
    void SetActiveCamera(const std::string& tag) { cameraManager.SetActiveCamera(tag); }

protected:
    /**
     * @brief Initializes the game state before any update or rendering occurs.
     *
     * @details
     * Called once when the state is first activated (after Load). This method is intended
     * for setting up game logic, assigning materials, positioning objects, etc.
     *
     * ObjectManager::InitAll() will be called automatically right after this function.
     *
     * @param engineContext Global engine context passed by the engine.
     *
     * @note Override this function in your subclass to define setup behavior.
     */
    virtual void Init([[maybe_unused]] const EngineContext& engineContext) {}

    /**
     * @brief Finalizes initialization after all objects are fully initialized.
     *
     * @details
     * Called after Init() and after all objects have completed ObjectManager::InitAll().
     * Use this to perform logic that depends on other objects being present.
     *
     * ObjectManager::AddAllPendingObjects() will be called right after this.
     *
     * @param engineContext Global engine context.
     */
    virtual void LateInit([[maybe_unused]] const EngineContext& engineContext) {}

    /**
     * @brief Called once per frame to update game logic.
     *
     * @details
     * This function is part of the main game loop. It updates all game objects
     * by default using ObjectManager::UpdateAll(). Override to insert state-level logic.
     *
     * This is the primary method for per-frame gameplay behavior.
     *
     * @param dt Delta time (in seconds) since last frame.
     * @param engineContext Global engine context.
     *
     * @note You can still call objectManager.UpdateAll() manually if overriding completely.
     */
    virtual void Update(float dt, [[maybe_unused]] const EngineContext& engineContext) { objectManager.UpdateAll(dt, engineContext); }

    /**
     * @brief Called after Update() and collision handling.
     *
     * @details
     * Use this function to apply post-update logic such as cleanup, state transitions,
     * or camera tracking. Called after ObjectManager::CheckCollision() in the frame.
     *
     * @param dt Delta time (in seconds).
     * @param engineContext Global engine context.
     */
    virtual void LateUpdate([[maybe_unused]] float dt, [[maybe_unused]] const EngineContext& engineContext) {}

    /**
     * @brief Called once when the state is first loaded into memory.
     *
     * @details
     * This method is meant for loading persistent resources like shaders, textures,
     * or prefabs before Init() and before any objects are initialized.
     *
     * It is typically invoked by StateManager::ChangeState().
     *
     * @param engineContext Global engine context.
     *
     * @note Override to preload shared assets or setup camera layers.
     */
    virtual void Load([[maybe_unused]] const EngineContext& engineContext) {}

    /**
     * @brief Called once per frame to render this state.
     *
     * @details
     * By default, this function delegates drawing to ObjectManager using the currently
     * active camera. Override it if custom render logic is needed (e.g., layered rendering).
     *
     * This is typically called after Update() and LateUpdate().
     *
     * @param engineContext Global engine context.
     */
    virtual void Draw([[maybe_unused]] const EngineContext& engineContext)
    {
        objectManager.DrawAll(engineContext, cameraManager.GetActiveCamera());
    }

    /**
     * @brief Called when the state is being deactivated or restarted.
     *
     * @details
     * Used for cleaning up logic before object memory is released. This function is
     * called before ObjectManager::FreeAll() and before Unload().
     *
     * Override to release temporary references.
     *
     * @param engineContext Global engine context.
     */
    virtual void Free([[maybe_unused]] const EngineContext& engineContext) {}

    /**
     * @brief Called once when the state is removed from memory.
     *
     * @details
     * Use this to unload resources loaded in Load().
     * This is the final cleanup step after Free() and ObjectManager::FreeAll().
     *
     * @param engineContext Global engine context.
     */
    virtual void Unload([[maybe_unused]] const EngineContext& engineContext) {}

    /**
     * @brief Restarts the current state by freeing and reinitializing it.
     *
     * @details
     * Internally calls SystemFree() and then SystemInit(), preserving the loaded resources.
     * Useful when restarting a level or resetting gameplay without reloading textures.
     *
     * @param engineContext Global engine context.
     *
     * @code
     * if (player.Died()) Restart(engineContext);
     * @endcode
     */
    void Restart(const EngineContext& engineContext)
    {
        SystemFree(engineContext);
        SystemInit(engineContext);
    }

    ObjectManager objectManager;
    CameraManager cameraManager;

private:
    /**
     * @brief Internal engine call to trigger Load() during state transition.
     *
     * @details
     * Called by StateManager when this state becomes active. Automatically invokes
     * the user-defined Load() function.
     *
     * @param engineContext Global engine context.
     *
     * @note This function is for internal use only. Do not override.
     */
    virtual void SystemLoad(const EngineContext& engineContext)
    {
        Load(engineContext);
    }

    /**
     * @brief Internal engine call to initialize the state.
     *
     * @details
     * This sequence is automatically called by StateManager in the following order:
     * 1. Init()
     * 2. objectManager.InitAll()
     * 3. LateInit()
     * 4. objectManager.AddAllPendingObjects()
     *
     * @param engineContext Global engine context.
     *
     * @note For internal use only. Do not override this method.
     */
    virtual void SystemInit(const EngineContext& engineContext)
    {
        Init(engineContext);
        objectManager.InitAll(engineContext);
        LateInit(engineContext);
        objectManager.AddAllPendingObjects(engineContext);
    }

    /**
     * @brief Internal engine call to update and simulate this state for one frame.
     *
     * @details
     * Called every frame by the engine. Executes the following in order:
     * 1. Update()
     * 2. objectManager.CheckCollision()
     * 3. Draw debug colliders (if enabled)
     * 4. LateUpdate()
     *
     * @param dt Delta time since last frame (in seconds).
     * @param engineContext Global engine context.
     *
     * @note For internal engine use only. Do not override this function.
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
     * @brief Internal cleanup logic called before the state is destroyed or restarted.
     *
     * @details
     * Calls Free() and then clears all objects using ObjectManager::FreeAll().
     *
     * @param engineContext Global engine context.
     *
     * @note Internal engine use only. Do not override this function.
     */
    virtual void SystemFree(const EngineContext& engineContext)
    {
        Free(engineContext);
        objectManager.FreeAll(engineContext);
    }

    /**
     * @brief Final internal cleanup when the state is permanently unloaded.
     *
     * @details
     * Called after Free() and ObjectManager cleanup. Invokes the user-defined Unload() method.
     *
     * @param engineContext Global engine context.
     *
     * @note Internal use only. Do not override directly.
     */
    virtual void SystemUnload(const EngineContext& engineContext)
    {
        Unload(engineContext);
    }
};
