#pragma once

#include "InputManager.h"
#include "RenderManager.h"
#include "SoundManager.h"
#include "StateManager.h"
#include "WindowManager.h"

struct GLFWwindow;
class SNAKE_Engine;

/**
 * @brief Core context struct that provides access to all major engine systems.
 *
 * @details
 * EngineContext is a central struct used throughout the engine to pass references to major subsystems
 * such as state, rendering, input, sound, and window management. This context is typically set once
 * during engine initialization in SNAKE_Engine::SetEngineContext() and passed to systems such as
 * ObjectManager, GameState, and others.
 *
 * This pattern helps avoid global variables and simplifies dependency injection.
 *
 * @note Should be initialized once after window creation and reused across all systems.
 */
struct EngineContext
{
    StateManager* stateManager = nullptr;   /**< Manages game states and handles transitions. */
    WindowManager* windowManager = nullptr; /**< Handles window creation, resizing, and OpenGL context. */
    InputManager* inputManager = nullptr;   /**< Manages keyboard and mouse input states. */
    RenderManager* renderManager = nullptr; /**< Manages all rendering operations and resources. */
    SoundManager* soundManager = nullptr;   /**< Controls sound playback, volume, and instance IDs. */
    SNAKE_Engine* engine = nullptr;         /**< Reference to the main engine that controls the game loop. */
};
