#pragma once

#include "InputManager.h"
#include "RenderManager.h"
#include "SoundManager.h"
#include "StateManager.h"
#include "WindowManager.h"

struct GLFWwindow;
class SNAKE_Engine;

/**
 * @brief Lightweight bundle of non-owning pointers to core engine systems.
 *
 * @details
 * - Populated in SNAKE_Engine::SetEngineContext().
 * - Passed to StateManager::Update/Draw/Free(const EngineContext&).
 *   - In Draw(): calls RenderManager::FlushDrawCommands(engineContext) and,
 *     if engine.ShouldRenderDebugDraws(), calls
 *     RenderManager::FlushDebugLineDrawCommands(engineContext).
 *   - In Free(): forwards to the current state's SystemFree/SystemUnload.
 * - Used by the GLFW framebuffer-size callback (WindowManager.cpp):
 *   - Updates WindowManager::SetWidth/SetHeight().
 *   - Propagates size to the current state's CameraManager::SetScreenSizeForAll(width, height).
 *
 * @note
 * This struct has no functions and owns no resources. It only aggregates pointers.
 *
 * @code
 * // Filled once (SNAKE_Engine::SetEngineContext)
 * engineContext.stateManager  = &stateManager;
 * engineContext.windowManager = &windowManager;
 * engineContext.inputManager  = &inputManager;
 * engineContext.renderManager = &renderManager;
 * engineContext.soundManager  = &soundManager;
 * engineContext.engine        = this;
 * @endcode
 */
struct EngineContext
{
    /**
     * @brief Pointer to StateManager.
     * @details Used by StateManager::Update/Draw/Free() call chain that operates on the current GameState.
     */
    StateManager* stateManager = nullptr;

    /**
     * @brief Pointer to WindowManager.
     * @details Updated by framebuffer-size callback via SetWidth/SetHeight().
     */
    WindowManager* windowManager = nullptr;

    /**
     * @brief Pointer to InputManager.
     * @details Window callbacks (scroll/key/mouse) forward into InputManager (see WindowManager.cpp).
     */
    InputManager* inputManager = nullptr;

    /**
     * @brief Pointer to RenderManager.
     * @details Used in StateManager::Draw() to flush draw and debug-line commands.
     */
    RenderManager* renderManager = nullptr;

    /**
     * @brief Pointer to SoundManager.
     */
    SoundManager* soundManager = nullptr;

    /**
     * @brief Pointer to the engine instance.
     * @details Queried in StateManager::Draw() (e.g., ShouldRenderDebugDraws()).
     */
    SNAKE_Engine* engine = nullptr;
};
