#pragma once
#include "EngineContext.h"

class SNAKE_Engine
{
public:
    /**
     * @brief Default constructor.
     */
    SNAKE_Engine() = default;

    /**
     * @brief Initializes core systems and creates the window.
     *
     * @details
     * - Calls WindowManager::Init(windowWidth, windowHeight, *this).
     * - Populates EngineContext via SetEngineContext().
     * - Initializes InputManager with the window handle.
     * - Initializes SoundManager.
     * - Initializes RenderManager with EngineContext.
     *
     * @param windowWidth Initial window width (pixels).
     * @param windowHeight Initial window height (pixels).
     * @return true on success; false if window initialization fails.
     */
    [[nodiscard]] bool Init(int windowWidth, int windowHeight);

    /**
     * @brief Runs the main loop until RequestQuit() is called.
     *
     * @details
     * Per-frame, processes events, updates input/state/sound, clears the screen,
     * draws the active state, and swaps buffers. Updates the window title with
     * an FPS readout during the loop. On exit, frees Sound/State/Window and
     * then calls Free() for final shutdown.
     */
    void Run();

    /**
     * @brief Requests the main loop to exit at the next opportunity.
     * @details Sets an internal flag checked by Run().
     */
    void RequestQuit();

    /**
     * @brief Returns the shared EngineContext.
     * @details Used by subsystems and callbacks (e.g., framebuffer resize).
     */
    [[nodiscard]] EngineContext& GetEngineContext() { return engineContext; }

    /**
     * @brief Enables or disables debug-line rendering.
     * @details Queried in StateManager::Draw() to flush debug draw commands.
     * @param shouldShow true to enable debug draws; false to disable.
     */
    void RenderDebugDraws(bool shouldShow) { showDebugDraw = shouldShow; }

    /**
     * @brief Returns whether debug-line rendering is enabled.
     */
    [[nodiscard]] bool ShouldRenderDebugDraws() const { return showDebugDraw; }
private:
    /**
     * @brief Final shutdown of GLFW.
     * @note Internal. Called at the end of Run().
     */
    void Free() const;

    /**
     * @brief Fills EngineContext pointers to subsystems.
     * @note Internal. Called during Init().
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
