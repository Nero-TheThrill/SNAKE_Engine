#pragma once
#include "EngineContext.h"

class SNAKE_Engine
{
public:
	/**
	 * @brief Default constructor for the engine.
	 *
	 * @details
	 * Initializes the engine object without setting up any internal subsystems.
	 * All subsystems (WindowManager, InputManager, etc.) will remain uninitialized
	 * until Init() is called explicitly.
	 */
    SNAKE_Engine() = default;

    /**
	 * @brief Initializes all core engine systems and the main window.
	 *
	 * @details
	 * Initializes the GLFW context and creates the application window with the given dimensions.
	 * Also sets up the EngineContext by linking core subsystems like InputManager, RenderManager, etc.
	 * If window initialization fails, logs an error and returns false.
	 *
	 * @param windowWidth Width of the application window in pixels.
	 * @param windowHeight Height of the application window in pixels.
	 * @return true if all systems were initialized successfully, false otherwise.
	 *
	 * @code
	 * SNAKE_Engine engine;
	 * if (!engine.Init(1280, 720))
	 * {
	 *     return -1;
	 * }
	 * @endcode
	 */
    [[nodiscard]] bool Init(int windowWidth, int windowHeight);

	/**
	 * @brief Starts the main game loop.
	 *
	 * @details
	 * Runs the engine's core update and rendering loop until the window is closed
	 * or RequestQuit() is called. Manages delta time, window events, input polling,
	 * rendering, and state updates. Also displays FPS in the window title.
	 *
	 * @note This function blocks until the application exits.
	 *
	 * @code
	 * SNAKE_Engine engine;
	 * engine.Init(1280, 720);
	 * engine.Run();
	 * @endcode
	 */
    void Run();

	/**
	 * @brief Signals the engine to exit the main loop.
	 *
	 * @details
	 * Sets an internal flag that causes the Run() loop to exit gracefully.
	 * This does not immediately terminate the application but ensures that
	 * the engine finishes the current frame and exits on the next check.
	 *
	 * @code
	 * if (inputManager->IsKeyPressed(GLFW_KEY_ESCAPE))
	 * {
	 *     engine.RequestQuit();
	 * }
	 * @endcode
	 */
    void RequestQuit();

	/**
	 * @brief Returns the current engine context.
	 *
	 * @details
	 * Provides access to the shared EngineContext which contains references
	 * to all core subsystems such as StateManager, InputManager, RenderManager, etc.
	 * Useful for passing context between systems or during GameState operations.
	 *
	 * @return Reference to the internal EngineContext.
	 *
	 * @code
	 * EngineContext& engineContext = engine.GetEngineContext();
	 * engineContext.stateManager->ChangeState(...);
	 * @endcode
	 */
    [[nodiscard]] EngineContext& GetEngineContext() { return engineContext; }

	/**
	 * @brief Enables or disables rendering of debug visuals.
	 *
	 * @details
	 * Controls whether debug visualizations (e.g., collider outlines, guide lines)
	 * should be drawn during rendering. The debug flag can be toggled at runtime.
	 *
	 * @param shouldShow If true, debug visuals will be rendered. Otherwise, they will be hidden.
	 *
	 * @code
	 * engine.RenderDebugDraws(true); // Enable debug lines
	 * @endcode
	 */
    void RenderDebugDraws(bool shouldShow) { showDebugDraw = shouldShow; }

	/**
	 * @brief Returns whether debug draw mode is currently active.
	 *
	 * @details
	 * This function is intended for internal use only. It is used by systems like
	 * RenderManager to determine whether to flush debug visualizations such as
	 * collider outlines. End users typically do not need to call this function.
	 *
	 * @return true if debug draws are enabled, false otherwise.
	 *
	 * @note This is an internal helper and not meant to be called externally.
	 */
    [[nodiscard]] bool ShouldRenderDebugDraws() const { return showDebugDraw; }
private:
	/**
	 * @brief Frees global engine resources and shuts down the graphics context.
	 *
	 * @details
	 * Terminates the underlying GLFW system. Should be called only once when
	 * the application is exiting. This function is automatically called at the
	 * end of Run(), so end users typically do not need to call it directly.
	 *
	 * @note This function is called automatically at shutdown.
	 */
    void Free() const;

	/**
	 * @brief Initializes internal EngineContext with references to core systems.
	 *
	 * @details
	 * Assigns pointers for WindowManager, InputManager, StateManager, SoundManager,
	 * RenderManager, and this engine itself into the internal EngineContext structure.
	 * This allows all subsystems to access each other via the shared context.
	 *
	 * @note This function is called internally during Init().
	 */
    void SetEngineContext();

    EngineContext engineContext;
    StateManager stateManager;
    WindowManager windowManager;
    InputManager inputManager;
    RenderManager renderManager;
    SoundManager soundManager;
    bool shouldRun = true;
    bool showDebugDraw = false;
};
