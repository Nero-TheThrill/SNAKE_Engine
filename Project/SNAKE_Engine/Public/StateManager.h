#pragma once
#include <memory>

class GameState;
class SNAKE_Engine;
struct EngineContext;

/**
 * @brief Manages the active GameState and orchestrates state transitions.
 *
 * @details
 * - Holds the current state and a queued next state.
 * - The actual transition occurs in Update(): the previous state is freed/unloaded,
 *   then the next state is loaded/initialized and becomes current.
 * - On transition, all sounds are stopped via SoundManager::ControlAll(Stop).
 * - After a state's Draw(), RenderManager::FlushDrawCommands() is called; if the
 *   engine requests debug draw, FlushDebugLineDrawCommands() is also called.
 * - On transition, the new state's CameraManager receives the current window
 *   size via CameraManager::SetScreenSizeForAll(WindowManager::GetWidth/Height()).
 *
 * @note Update(), Draw(), Free() are internal (called by SNAKE_Engine).
 */
class StateManager
{
    friend SNAKE_Engine;
public:
    /**
     * @brief Returns the current GameState pointer.
     * @return Pointer to the current state, or nullptr if none.
     */
    [[nodiscard]] GameState* GetCurrentState() const;

    /**
     * @brief Queues a new state to be activated on the next Update() call.
     *
     * @details
     * Ownership is transferred; the switch is deferred until Update().
     *
     * @param newState Unique ownership of the next GameState.
     * @code
     * std::unique_ptr<GameState> gs =  <create your state> ;
     * stateManager.ChangeState(std::move(gs)); // takes effect in Update()
     * @endcode
     */
    void ChangeState(std::unique_ptr<GameState> newState);

private:
    /**
     * @brief Per-frame update and (if queued) state transition.
     *
     * @details
     * If nextState exists:
     * - If currentState exists, calls currentState->SystemFree() and SystemUnload(),
     *   then engineContext.soundManager->ControlAll(SoundManager::SoundControlType::Stop).
     * - Moves nextState into currentState.
     * - Calls currentState->SystemLoad(engineContext) and SystemInit(engineContext).
     * - Calls currentState->GetCameraManager().SetScreenSizeForAll(
     *     engineContext.windowManager->GetWidth(),
     *     engineContext.windowManager->GetHeight()).
     * Then, if currentState exists, calls currentState->SystemUpdate(dt, engineContext).
     *
     * @param dt Delta time in seconds.
     * @param engineContext Aggregated access to engine systems.
     * @note Internal. Called by SNAKE_Engine each frame.
     */
    void Update(float dt, const EngineContext& engineContext);

    /**
     * @brief Per-frame drawing for the current state.
     *
     * @details
     * If currentState exists:
     * - Calls currentState->Draw(engineContext).
     * - Calls engineContext.renderManager->FlushDrawCommands(engineContext).
     * - If engineContext.engine->ShouldRenderDebugDraws() is true, also calls
     *   engineContext.renderManager->FlushDebugLineDrawCommands(engineContext).
     *
     * @param engineContext Aggregated access to engine systems.
     * @note Internal. Called by SNAKE_Engine each frame.
     */
    void Draw(const EngineContext& engineContext);

    /**
     * @brief Releases the current state's resources.
     *
     * @details
     * If currentState exists:
     * - Calls currentState->SystemFree(engineContext) and SystemUnload(engineContext).
     *
     * @param engineContext Aggregated access to engine systems.
     * @note Internal. Called by SNAKE_Engine during shutdown.
     */
    void Free(const EngineContext& engineContext);

    /** @brief Active state. */
    std::unique_ptr<GameState> currentState;
    /** @brief Queued state to switch into on Update(). */
    std::unique_ptr<GameState> nextState;
};
