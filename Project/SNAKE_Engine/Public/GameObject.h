#pragma once
#include "Object.h"

/**
 * @brief Minimal Object subclass with no-op lifecycle methods.
 *
 * @details
 * - Constructs the base Object with ObjectType::GAME.
 * - All lifecycle hooks (Init/LateInit/Update/Draw/Free/LateFree) are empty by default,
 *   so users can override only the ones they need.
 */
class GameObject : public Object
{
public:
    /** @brief Sets the base type to ObjectType::GAME. */
    GameObject() : Object(ObjectType::GAME) {}
    ~GameObject() override = default;

    /** @brief No-op. Override to initialize resources. */
    void Init([[maybe_unused]] const EngineContext& engineContext) override {}

    /** @brief No-op. Override for post-initialization setup. */
    void LateInit([[maybe_unused]] const EngineContext& engineContext) override {}

    /** @brief No-op. Override to implement per-frame logic. */
    void Update([[maybe_unused]] float dt, [[maybe_unused]] const EngineContext& engineContext) override {}

    /** @brief No-op. Override to submit draw calls or set per-object uniforms. */
    void Draw([[maybe_unused]] const EngineContext& engineContext) override {}

    /** @brief No-op. Override to release resources. */
    void Free([[maybe_unused]] const EngineContext& engineContext) override {}

    /** @brief No-op. Override for teardown steps after Free(). */
    void LateFree([[maybe_unused]] const EngineContext& engineContext) override {}
};
