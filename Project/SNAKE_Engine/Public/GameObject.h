#pragma once
#include "Object.h"

/**
 * @brief A base class for user-defined game objects in the engine.
 *
 * @details
 * GameObject extends the Object class and represents a typical updatable and drawable
 * entity in the scene. It is intended for general-purpose gameplay logic.
 *
 * This class provides default empty implementations for all lifecycle functions such as
 * Init, Update, and Draw. Users can subclass GameObject to implement their own logic
 * and behaviors by overriding these methods.
 *
 * GameObjects are typically registered through the ObjectManager and updated/rendered automatically.
 *
 * @code
 * class Player : public GameObject
 * {
 *     void Update(float dt, const EngineContext& engineContext) override
 *     {
 *         // custom logic
 *     }
 * };
 * @endcode
 */
class GameObject : public Object
{
public:
    /**
     * @brief Constructs a GameObject with type ObjectType::GAME.
     *
     * @details
     * Initializes the GameObject and sets its internal object type to ObjectType::GAME,
     * which distinguishes it from other object types such as text objects.
     *
     * This type information can be used for categorization, filtering, or specialized behavior
     * within ObjectManager or other engine systems.
     *
     * @code
     * auto obj = std::make_unique<GameObject>();
     * @endcode
     */
    GameObject() : Object(ObjectType::GAME) {}

    /**
     * @brief Virtual destructor for safe polymorphic deletion.
     *
     * @details
     * Ensures that derived GameObject instances are destroyed properly when deleted
     * via a base class pointer. Performs no specific cleanup.
     */
    ~GameObject() override = default;

    /**
     * @brief Called once when the object is first added to the scene.
     *
     * @details
     * This method is intended for one-time setup such as loading resources or configuring state.
     * It is called automatically by ObjectManager after the object is fully constructed.
     *
     * Override this method in subclasses to implement custom initialization logic.
     *
     * @param engineContext Reference to the current engine context.
     *
     * @code
     * void Init(const EngineContext& engineContext) override
     * {
     *     SetMaterial(engineContext, "player_material");
     * }
     * @endcode
     */
    void Init([[maybe_unused]] const EngineContext& engineContext) override {}

    /**
     * @brief Called after all objects have completed their Init() phase.
     *
     * @details
     * This method is useful for logic that depends on other objects already being initialized.
     * It is automatically invoked by ObjectManager after Init() is complete for all objects.
     *
     * Override this method to implement dependency-based setup.
     *
     * @param engineContext Reference to the engine context.
     */
    void LateInit([[maybe_unused]] const EngineContext& engineContext) override {}

    /**
     * @brief Called once per frame to update the object's logic.
     *
     * @details
     * This function is part of the per-frame simulation loop and is used to update the object's state,
     * handle input, animation, or physics.
     *
     * Override this function to implement per-frame behavior.
     *
     * @param dt Delta time since the previous frame.
     * @param engineContext Reference to the engine context.
     *
     * @code
     * void Update(float dt, const EngineContext& engineContext) override
     * {
     *     AddPosition({ 100.0f * dt, 0.0f });
     * }
     * @endcode
     */
    void Update([[maybe_unused]] float dt, [[maybe_unused]] const EngineContext& engineContext) override {}

    /**
     * @brief Called every frame to issue draw commands for this object.
     *
     * @details
     * This function is invoked during the rendering phase, typically after Update().
     * Override it to submit custom draw logic, such as setting uniforms or handling non-standard rendering.
     *
     * The actual mesh and material rendering is often handled automatically if set properly.
     *
     * @param engineContext Reference to the engine context.
     */
    void Draw([[maybe_unused]] const EngineContext& engineContext) override {}

    /**
     * @brief Called once when the object is scheduled for destruction.
     *
     * @details
     * This function is used to release any engine-related resources or cleanup logic
     * before the object is actually removed from memory.
     *
     * It is called before LateFree(), and object is still valid at this point.
     *
     * @param engineContext Reference to the engine context.
     */
    void Free([[maybe_unused]] const EngineContext& engineContext) override {}

    /**
     * @brief Final cleanup called after Free() and just before deletion.
     *
     * @details
     * Called after Free(), this function is used to finalize cleanup when the object
     * is already being removed from all systems. Useful for removing references or
     * delayed disposal logic.
     *
     * @param engineContext Reference to the engine context.
     */
    void LateFree([[maybe_unused]] const EngineContext& engineContext) override {}
};
