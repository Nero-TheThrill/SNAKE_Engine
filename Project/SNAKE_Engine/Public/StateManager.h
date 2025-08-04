#pragma once
#include <memory>

class GameState;
class SNAKE_Engine;
struct EngineContext;

/**
 * @brief Manages the active game state and transitions between states.
 *
 * @details
 * StateManager holds the current and next GameState instances.
 * It is responsible for handling state transitions, per-frame updates, and rendering.
 * When a new state is set via ChangeState(), the current state is properly unloaded and freed.
 * The manager ensures that only one state is active at a time.
 *
 * This class is internally used by the engine and is typically accessed via EngineContext.
 *
 * @note Game states must inherit from GameState and implement the virtual system lifecycle methods.
 */
class StateManager
{
    friend SNAKE_Engine;
public:
    /**
	 * @brief Returns the currently active game state.
	 *
	 * @details
	 * If no state has been set yet, returns nullptr.
	 * This function is useful for querying the current state,
	 * such as accessing its camera manager or checking which scene is loaded.
	 *
	 * @return Pointer to the currently active GameState, or nullptr if none.
	 *
	 * @code
	 * GameState* state = engineContext.stateManager->GetCurrentState();
	 * if (state) { state->GetCameraManager().SetZoom(2.0f); }
	 * @endcode
	 */
    [[nodiscard]] GameState* GetCurrentState() const;

	/**
	 * @brief Schedules a transition to a new game state.
	 *
	 * @details
	 * The current state will be unloaded and freed on the next update cycle.
	 * The provided state will be initialized and become the new active state.
	 * Use this to switch between menus, levels, or scenes.
	 *
	 * @param newState The new game state to activate.
	 *
	 * @note The transition is deferred; it occurs during the next call to Update().
	 *
	 * @code
	 * auto mainMenu = std::make_unique<MainMenuState>();
	 * engineContext.stateManager->ChangeState(std::move(mainMenu));
	 * @endcode
	 */
    void ChangeState(std::unique_ptr<GameState> newState);

private:
	/**
	 * @brief Updates the current game state and handles state transitions.
	 *
	 * @details
	 * If a new state was scheduled using ChangeState(), the current state is properly
	 * unloaded and the new one is loaded and initialized. After transition, the current state's
	 * SystemUpdate() function is called with the provided delta time and engine context.
	 *
	 * @param dt Delta time in seconds.
	 * @param engineContext The current engine context.
	 *
	 * @note This function is called once per frame by SNAKE_Engine::Run().
	 */
    void Update(float dt, const EngineContext& engineContext);

	/**
	 * @brief Renders the current game state.
	 *
	 * @details
	 * Calls the current state's Draw() function and flushes all draw commands.
	 * If debug draw mode is enabled in the engine, it also flushes debug lines.
	 *
	 * @param engineContext The current engine context.
	 *
	 * @note This is invoked once per frame after Update().
	 */
    void Draw(const EngineContext& engineContext);

	/**
	 * @brief Unloads and frees the current game state.
	 *
	 * @details
	 * Calls the current state's SystemFree() and SystemUnload() methods to
	 * properly shut down the active state before engine termination.
	 *
	 * @param engineContext The current engine context.
	 *
	 * @note Called by the engine during shutdown.
	 */
    void Free(const EngineContext& engineContext);

    std::unique_ptr<GameState> currentState;
    std::unique_ptr<GameState> nextState;
};
