#pragma once

/**
 * @file
 * @brief Umbrella header that aggregates SNAKE Engine public headers.
 *
 * @details
 * Include this header in application code to access core engine systems
 * (engine loop, state/object/camera/render/sound managers) and common
 * game-side types (Object, GameObject, TextObject, Material/Mesh/Texture,
 * shaders, transforms, fonts, colliders, animations, timers, etc.).
 *
 * This header only re-exports other headers; it declares no new symbols.
 * If you need finer compile-time control, include specific headers instead.
 *
 * @see SNAKE_Engine for engine lifecycle (Init/Run/RequestQuit)
 * @see GameState for per-state hooks (Init/Update/Draw/Free/Unload)
 * @see RenderManager, InputManager, SoundManager, WindowManager
 * @see Object, GameObject, TextObject
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
#include "RenderLayerManager.h"
#include "EngineTimer.h"

#include "Object.h"
#include "TextObject.h"
#include "GameObject.h"

#include "Texture.h"
#include "Shader.h"
#include "Transform.h"
#include "Material.h"
#include "Mesh.h"
#include "Font.h"
#include "Camera2D.h"
#include "Collider.h"
#include "Animation.h"

#include "Debug.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
