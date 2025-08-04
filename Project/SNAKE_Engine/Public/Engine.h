#pragma once
/**
 * @file Engine.h
 * @brief Centralized include header for all core engine modules.
 *
 * @details
 * This file aggregates all essential engine headers into a single point of inclusion.
 * It simplifies development for tools, gameplay logic, and testing code that need access
 * to most engine systems. Should only be included in implementation files (e.g., '.cpp')
 * or by editors/tools - not in public engine headers.
 *
 * Be cautious of increased compile time due to heavy inclusion.
 *
 * Included systems:
 * - Engine core: SNAKE_Engine, EngineContext, EngineTimer
 * - Managers: StateManager, ObjectManager, InputManager, RenderManager, SoundManager, WindowManager, CameraManager
 * - Components: Transform, Texture, Material, Mesh, Shader
 * - Objects: Object, GameObject, TextObject
 * - UI/Text: Font
 * - Rendering: Camera2D, glm
 *
 * @note Do not include this file in header files to avoid unnecessary recompilation.
 */
#include "SNAKE_Engine.h"
#include "GameState.h"
#include "EngineContext.h"
#include "InputManager.h"
#include "WindowManager.h"
#include "StateManager.h"
#include "ObjectManager.h"
#include "RenderManager.h"
#include "SoundManager.h"
#include "CameraManager.h"

#include "Texture.h"
#include "Shader.h"
#include "Transform.h"
#include "Material.h"
#include "Mesh.h"
#include "Object.h"
#include "TextObject.h"
#include "GameObject.h"
#include "Font.h"

#include "EngineTimer.h"

#include "Camera2D.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
