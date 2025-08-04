#pragma once
#include <memory>
#include <string>

#include "Animation.h"
#include "Collider.h"
#include "Mesh.h"
#include "Transform.h"
class FrustumCuller;
struct EngineContext;
enum class ObjectType
{
    GAME,
    TEXT
};
/**
 * @brief Abstract base class for all renderable and updateable engine objects.
 *
 * @details
 * Object serves as the root class for all drawable, updateable entities within the engine.
 * It provides common interfaces for transform, rendering, collision, animation, and lifetime control.
 *
 * This class is not meant to be directly inherited or instantiated by users.
 * Instead, users should derive from engine-defined subclasses such as GameObject or TextObject,
 * which provide concrete implementations and safe access points.
 *
 * Object enforces lifecycle stages (Init, Update, Draw, Free) and maintains internal flags
 * like isAlive, isVisible, and ignoreCamera, while delegating rendering and collision behavior
 * to RenderManager and ObjectManager respectively.
 *
 * @note
 * - Object is abstract (contains pure virtual methods).
 * - Default constructor is deleted; only protected constructor with ObjectType allowed.
 * - Use GameObject/TextObject as base classes instead.
 *
 * @code
 * // Recommended usage:
 * class MyPlayer : public GameObject
 * {
 * public:
 *     void Init(const EngineContext& engineContext) override;
 *     void Update(float dt, const EngineContext& engineContext) override;
 *     void Draw(const EngineContext& engineContext) override;
 *     void OnCollision(Object* other) override;
 * };
 * @endcode
 */

class Object
{
    friend FrustumCuller;
public:
    /**
	 * @brief Prevents direct instantiation of Object.
	 *
	 * @details
	 * Object is an abstract base class and should not be constructed directly.
	 * Subclasses must implement all pure virtual functions.
	 */
    Object() = delete;

    /**
     * @brief Called once when the object is registered to ObjectManager.
     *
     * @details
     * This is the first stage of the object's initialization process.
     * ObjectManager internally calls this through InitAll() or AddAllPendingObjects().
     * Override in derived classes to set up mesh, material, render layer, collider, and more.
     *
     * @param engineContext Global engine context for accessing managers and systems.
     *
     * @code
     * void MyPlayer::Init(const EngineContext& engineContext)
     * {
     *     SetMesh(engineContext, "Quad");
     *     SetMaterial(engineContext, "PlayerMat");
     *     SetRenderLayer(engineContext, "Game");
     * }
     * @endcode
     */
    virtual void Init([[maybe_unused]] const EngineContext& engineContext) = 0;

    /**
	 * @brief Called once after Init() to finalize setup.
	 *
	 * @details
	 * LateInit() is useful for initialization steps that depend on other objects also being initialized.
	 * ObjectManager ensures LateInit is called only after Init completes for all objects.
	 *
	 * @param engineContext Shared engine systems and resource access.
	 *
	 * @code
	 * void MyObject::LateInit(const EngineContext& engineContext)
	 * {
	 *     auto other = engineContext.objectManager->FindByTag("SomeObject");
	 *     if (other) { referenceToOther = other; }
	 * }
	 * @endcode
	 */
    virtual void LateInit([[maybe_unused]] const EngineContext& engineContext) = 0;

    /**
	 * @brief Called every frame if the object is alive.
	 *
	 * @details
	 * ObjectManager::UpdateAll() calls this method on each object whose IsAlive() returns true.
	 * Use this function to implement per-frame game logic like movement, animation progression, etc.
	 *
	 * @param dt Delta time in seconds since last frame.
	 * @param engineContext Engine-wide access to context (input, time, etc).
	 *
	 * @code
	 * void MyEnemy::Update(float dt, const EngineContext& engineContext)
	 * {
	 *     transform2D.position.x += speed * dt;
	 * }
	 * @endcode
	 */
    virtual void Update([[maybe_unused]] float dt, [[maybe_unused]] const EngineContext& engineContext) = 0;

    /**
	 * @brief Called during the rendering phase to draw this object.
	 *
	 * @details
	 * This function is invoked by RenderManager::Submit() internally, where the object has been
	 * filtered as visible and assigned a rendering layer. Override this method to bind material uniforms.
	 *
	 * Called once per frame for visible, alive objects.
	 *
	 * @param engineContext Engine-wide context used to access RenderManager and other systems.
	 *
	 * @code
	 * void MyPlayer::Draw(const EngineContext& engineContext)
	 * {
	 *     material->SetUniform("u_Variable", var);
	 * }
	 * @endcode
	 */
    virtual void Draw([[maybe_unused]] const EngineContext& engineContext) = 0;

    /**
	 * @brief Releases GPU and engine-related resources owned by this object.
	 *
	 * @details
	 * This is the first stage of object destruction, called inside ObjectManager::EraseDeadObjects() or FreeAll().
	 * Use this method to nullify raw pointers, or deallocate temporary data.
	 *
	 * Called only once before LateFree().
	 *
	 * @param engineContext Shared context for accessing managers (e.g., RenderManager).
	 */
    virtual void Free([[maybe_unused]] const EngineContext& engineContext) = 0;

    /**
	 * @brief Final cleanup after Free(), used to detach from systems or unregister tags.
	 *
	 * @details
	 * This method is called after Free() for post-removal tasks.
	 *
	 * Called once just before the object is erased from memory.
	 *
	 * @param engineContext Global access to managers and context.
	 */
    virtual void LateFree([[maybe_unused]] const EngineContext& engineContext) = 0;

    /**
	 * @brief Virtual destructor.
	 *
	 * @details
	 * Required for safe polymorphic deletion of derived Object instances.
	 */
    virtual ~Object() = default;

    /**
	 * @brief Called when this object collides with another object.
	 *
	 * @details
	 * This function is invoked by the collision system whenever a collision is detected involving this object.
	 * The default implementation does nothing. Override in derived classes to implement custom behavior such as
	 * damage, bounce, or event triggering.
	 *
	 * @param other The other Object instance involved in the collision.
	 *
	 * @code
	 * void MyEnemy::OnCollision(Object* other)
	 * {
	 *     if (other->GetTag() == "Player")
	 *         Kill();
	 * }
	 * @endcode
	 */
    virtual void OnCollision(Object* other) {}

    /**
	  * @brief Returns whether this object is considered alive.
	  *
	  * @details
	  * Dead objects (after calling Kill()) will no longer be updated or drawn,
	  * and are removed from ObjectManager in the next cleanup phase.
	  *
	  * @return True if the object is alive.
	  */
    [[nodiscard]] const bool& IsAlive() const;

	/**
	 * @brief Returns whether this object is currently visible.
	 *
	 * @details
	 * Only objects marked as visible will be passed to RenderManager::Submit().
	 *
	 * @return True if the object is visible this frame.
	 */
    [[nodiscard]] const bool& IsVisible() const;

	/**
	 * @brief Sets the visibility of the object.
	 *
	 * @param _isVisible Whether the object should be rendered this frame.
	 *
	 * @code
	 * if (distanceToCamera > 1000.0f)
	 *     SetVisibility(false);
	 * @endcode
	 */
    void SetVisibility(bool _isVisible);

	/**
	 * @brief Marks this object as dead and schedules it for deletion.
	 *
	 * @details
	 * Once Kill() is called, the object is flagged as inactive. It will be excluded from future updates and rendering,
	 * and will be destroyed during the next call to ObjectManager::EraseDeadObjects() or FreeAll().
	 *
	 * @code
	 * if (health <= 0)
	 *     Kill();
	 * @endcode
	 */
    void Kill();

	/**
	 * @brief Sets the tag used to identify this object within the ObjectManager.
	 *
	 * @details
	 * The tag is used as a unique identifier in ObjectManager::objectMap.
	 * If another object already exists with the same tag, a warning is issued.
	 * The tag is also used in functions like FindByTag or DrawObjectsWithTag
	 * for fast access and filtering.
	 *
	 * @param tag The string identifier to assign to this object.
	 *
	 * @note Must be set before adding the object to the ObjectManager.
	 * @code
	 * obj->SetTag("Player");
	 * objectManager.AddObject(std::move(obj), "Player");
	 * @endcode
	 */
    void SetTag(const std::string& tag);

	/**
	 * @brief Retrieves the current tag assigned to this object.
	 *
	 * @return A reference to the tag string used to identify this object.
	 */
    [[nodiscard]] const std::string& GetTag() const;

	/**
	 * @brief Gets the numeric render layer index assigned to this object.
	 *
	 * @return A reference to the internal render layer index (0–15).
	 *
	 * @note This value is assigned via SetRenderLayer() and internally stored as uint8_t.
	 */
    [[nodiscard]] const uint8_t& GetRenderLayer() const;

	/**
	 * @brief Assigns a render layer to the object using a named tag.
	 *
	 * @details
	 * Queries RenderLayerManager inside RenderManager to get the corresponding
	 * numeric ID for the layer tag and assigns it to this object.
	 * If the tag is invalid, default layer 0 is used.
	 *
	 * @param engineContext The global engine context.
	 * @param tag The string tag of the render layer to assign.
	 *
	 * @code
	 * obj->SetRenderLayer(engineContext, "Foreground");
	 * @endcode
	 */
    void SetRenderLayer(const EngineContext& engineContext, const std::string& tag);

	/**
	 * @brief Sets the object's material using a string tag.
	 *
	 * @details
	 * The material is looked up via RenderManager::GetMaterialByTag(),
	 * which must have been registered beforehand.
	 *
	 * @param engineContext The engine context for accessing the RenderManager.
	 * @param tag The tag used to identify the material.
	 *
	 * @note If the tag is not found, the material remains null.
	 */
    void SetMaterial(const EngineContext& engineContext, const std::string& tag);

	/**
	 * @brief Retrieves the material currently assigned to the object.
	 *
	 * @return A raw pointer to the material (maybe null).
	 */
    [[nodiscard]] Material* GetMaterial() const;

	/**
	 * @brief Sets the object's mesh using a string tag.
	 *
	 * @details
	 * The mesh is retrieved from RenderManager::GetMeshByTag(). The mesh must have
	 * been registered with a matching tag beforehand.
	 *
	 * @param engineContext The engine context for accessing the RenderManager.
	 * @param tag The tag used to look up the mesh.
	 *
	 * @note If the mesh tag is not found, the mesh remains null.
	 */
    void SetMesh(const EngineContext& engineContext, const std::string& tag);

	/**
	 * @brief Retrieves the mesh currently assigned to the object.
	 *
	 * @return A raw pointer to the mesh (maybe null).
	 */
    [[nodiscard]] Mesh* GetMesh() const;

	/**
	 * @brief Returns whether this object supports GPU instancing.
	 *
	 * @details
	 * Instanced objects share the same mesh and material across multiple entities
	 * and are batched together by the RenderManager for efficient rendering.
	 * Instancing is only possible for objects with compatible shaders.
	 *
	 * The shader must define the following vertex attributes to support instancing:
	 * @code
	 * layout (location = 2) in mat4 i_Model;
	 * layout (location = 6) in vec4 i_Color;
	 * layout (location = 7) in vec2 i_UVOffset;
	 * layout (location = 8) in vec2 i_UVScale;
	 * @endcode
	 *
	 * @return True if the object can be drawn using instancing.
	 */
    [[nodiscard]] bool CanBeInstanced() const;

	/**
	 * @brief Returns the 2D transformation matrix of the object.
	 *
	 * @details
	 * Combines translation, rotation, and scale into a glm::mat4 matrix
	 * used in the vertex shader's model transformation.
	 *
	 * @return 4x4 transformation matrix for the object.
	 */
    [[nodiscard]] glm::mat4 GetTransform2DMatrix();

	/**
	 * @brief Accessor for the Transform2D component of the object.
	 *
	 * @return Reference to the internal Transform2D instance.
	 *
	 * @note Modifying this allows direct control of position, rotation, and scale.
	 */
    [[nodiscard]] Transform2D& GetTransform2D();

	/**
	 * @brief Sets the tint color used for rendering this object.
	 *
	 * @param color_ RGBA color value (0~1 float).
	 *
	 * @code
	 * obj->SetColor(glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));
	 * @endcode
	 */
    void SetColor(const  glm::vec4& color_);

	/**
	 * @brief Returns the current tint color of the object.
	 *
	 * @return RGBA color value.
	 */
    [[nodiscard]] const glm::vec4& GetColor();

	/**
	 * @brief Checks whether the object has an animator attached.
	 *
	 * @return True if spriteAnimator is not null.
	 */
    [[nodiscard]] virtual bool HasAnimation() const { return spriteAnimator != nullptr; }

	/**
	 * @brief Returns the animator component, if any.
	 *
	 * @return Pointer to the SpriteAnimator, or nullptr if none attached.
	 */
    [[nodiscard]] virtual SpriteAnimator* GetAnimator() { return spriteAnimator.get(); }

	/**
	 * @brief Attach a pre-constructed SpriteAnimator to this object.
	 *
	 * @details
	 * Transfers ownership of the provided SpriteAnimator to this Object,
	 * enabling animated sprite rendering. This is typically used when the animator
	 * is constructed externally or reused between objects.
	 *
	 * @param anim The unique pointer to the SpriteAnimator to attach.
	 * @note Automatically enables animation for this object.
	 * @code
	 * auto animator = std::make_unique<SpriteAnimator>(sheet, 0.1f);
	 * object->AttachAnimator(std::move(animator));
	 * @endcode
	 */
    void AttachAnimator(std::unique_ptr<SpriteAnimator> anim) { spriteAnimator = std::move(anim); }

	/**
	 * @brief Create and attach a SpriteAnimator with specific parameters.
	 *
	 * @details
	 * Internally constructs a SpriteAnimator using the given SpriteSheet,
	 * frame duration, and loop setting. The created animator is then attached
	 * to the object. This method simplifies animator setup for most cases.
	 *
	 * @param sheet Pointer to the SpriteSheet used for animation frames.
	 * @param frameTime Time duration (in seconds) of each frame.
	 * @param loop Whether the animation should loop. Default is true.
	 * @note Equivalent to constructing a SpriteAnimator and calling AttachAnimator().
	 * @code
	 * object->AttachAnimator(sheet, 0.2f, true);
	 * @endcode
	 */
	void AttachAnimator(SpriteSheet* sheet, float frameTime, bool loop = true) { spriteAnimator = std::make_unique<SpriteAnimator>(sheet, frameTime, loop); }

	/**
	 * @brief Set a collider for this object.
	 *
	 * @details
	 * Assigns a Collider to this object, transferring ownership. Used in conjunction
	 * with collision masks/categories via SetCollision(). The collider is required
	 * for collision detection and spatial partitioning.
	 *
	 * @param c A unique pointer to the Collider to assign.
	 * @note Typically called during Init or LateInit.
	 * @code
	 * object->SetCollider(std::make_unique<CircleCollider>(radius));
	 * @endcode
	 */
    void SetCollider(std::unique_ptr<Collider> c) { collider = std::move(c); }

	/**
	 * @brief Get the current Collider of the object.
	 *
	 * @return Pointer to the Collider if set, or nullptr if no collider is attached.
	 */
	[[nodiscard]] Collider* GetCollider() const { return collider.get(); }

	/**
	 * @brief Sets the collision category and mask for this object.
	 *
	 * @details
	 * This function assigns a collision category based on the given tag.
	 * If the tag is not previously registered in the CollisionGroupRegistry,
	 * it will automatically create and assign a new collision bit.
	 *
	 * The collision mask is then constructed by OR-ing all the bits associated
	 * with the tags provided in `checkCollisionList`. This mask determines which
	 * categories this object will check collision against.
	 *
	 * This system allows flexible and tag-based collision group management without
	 * requiring manual registration beforehand.
	 *
	 * @param objectManager Reference to the ObjectManager for accessing collision registry.
	 * @param tag The category tag of this object. Auto-registers if not present.
	 * @param checkCollisionList A list of tags to define which categories this object should detect collision with.
	 *
	 * @code
	 * obj->SetCollision(objectManager, "enemy", { "player", "projectile" });
	 * @endcode
	 */
	void SetCollision(ObjectManager& objectManager, const std::string& tag, const std::vector<std::string>& checkCollisionList);

	/**
	 * @brief Returns the collision mask for this object.
	 *
	 * @details
	 * The mask defines which collision categories this object will test against.
	 * This value is computed via SetCollision() using the list of target tags.
	 *
	 * @return Bitmask representing all collision categories this object should detect.
	 */
    [[nodiscard]] uint32_t GetCollisionMask() const { return collisionMask; }

	/**
	 * @brief Returns the collision category of this object.
	 *
	 * @details
	 * The category defines which group this object belongs to for collision filtering.
	 * It is set by calling SetCollision() with a tag, and automatically assigned via CollisionGroupRegistry.
	 *
	 * @return A single-bit value representing this object's group.
	 */
    [[nodiscard]] uint32_t GetCollisionCategory() const { return collisionCategory; }

	/**
	 * @brief Checks whether this object ignores camera transformations when rendering.
	 *
	 * @details
	 * When true, the object is rendered in screen-space rather than world-space.
	 * This is useful for UI or HUD elements. See SetIgnoreCamera() to enable.
	 *
	 * @return True if the object should ignore camera transforms.
	 */
    [[nodiscard]] bool ShouldIgnoreCamera() const;

	/**
	 * @brief Toggles whether this object should ignore camera transformations.
	 *
	 * @details
	 * When enabled, the object's world position is treated as screen space (no camera offset),
	 * making it suitable for fixed-screen elements like HUD text or health bars.
	 * Optionally takes a reference camera for resolving transform once when toggled on.
	 *
	 * @param shouldIgnoreCamera Whether to enable camera ignoring mode.
	 * @param cameraForTransformCalc Optional camera to use for calculating screen-space transform immediately.
	 *
	 * @code
	 * obj->SetIgnoreCamera(true, mainCamera);
	 * @endcode
	 */
    void SetIgnoreCamera(bool shouldIgnoreCamera, Camera2D* cameraForTransformCalc = nullptr);

	/**
	 * @brief Returns the object type enum (GAME or TEXT).
	 *
	 * @details
	 * This allows external systems to distinguish between game objects and
	 * engine-defined utility objects like TextObject.
	 *
	 * @return Enum value indicating the type of this object.
	 */
    [[nodiscard]] ObjectType GetType() const { return type; }

	/**
	 * @brief Returns the reference camera used for screen-space calculations.
	 *
	 * @details
	 * If the object has ignoreCamera enabled, this pointer stores the camera
	 * that was used to calculate its screen-space position when SetIgnoreCamera was called.
	 *
	 * @return Pointer to the reference Camera2D, or nullptr if not set.
	 */
    [[nodiscard]] Camera2D* GetReferenceCamera() const { return referenceCamera; }

	/**
	 * @brief Returns the world position of the object in 2D space.
	 *
	 * @details
	 * Used for spatial queries and rendering placement. By default,
	 * this returns transform2D.position, but can be overridden by subclasses
	 * like UI elements or text for custom positioning logic.
	 *
	 * @return 2D world position as a glm::vec2.
	 */
    [[nodiscard]] virtual glm::vec2 GetWorldPosition() const;

	/**
	 * @brief Returns the world scale of the object in 2D space.
	 *
	 * @details
	 * By default, this returns transform2D.scale, but can be overridden
	 * for objects that need non-uniform or dynamic scaling.
	 *
	 * @return 2D scale as a glm::vec2.
	 */
    [[nodiscard]] virtual glm::vec2 GetWorldScale() const;

	/**
	 * @brief Enables or disables horizontal UV flipping for this object.
	 *
	 * @param shouldFlip If true, the U coordinates will be mirrored.
	 *
	 * @code
	 * player->SetFlipUV_X(true); // flips sprite horizontally
	 * @endcode
	 */
    void SetFlipUV_X(bool shouldFlip) { flipUV_X = shouldFlip; }

	/**
	 * @brief Enables or disables vertical UV flipping for this object.
	 *
	 * @param shouldFlip If true, the V coordinates will be mirrored.
	 *
	 * @code
	 * icon->SetFlipUV_Y(true); // flips sprite vertically
	 * @endcode
	 */
    void SetFlipUV_Y(bool shouldFlip) { flipUV_Y = shouldFlip; }

	/**
	 * @brief Returns the UV flip direction vector.
	 *
	 * @details
	 * (1,1) means no flipping. (-1,1) flips horizontally.
	 * This value is used in the shader to mirror UVs if needed.
	 *
	 * @return A glm::vec2 containing flip factors for X and Y.
	 */
    [[nodiscard]] glm::vec2 GetUVFlipVector() const;

protected:
	/**
	 * @brief Initializes an Object with a specific type (used by subclasses only).
	 *
	 * @details
	 * This constructor is intended to be called only by system-level subclasses
	 * such as GameObject or TextObject. It stores the ObjectType enum
	 * to distinguish between renderable types in the engine.
	 *
	 * Direct instantiation of Object is disallowed (constructor is protected and default constructor is deleted).
	 *
	 * @param objectType Logical classification of the object (e.g., GAME, TEXT).
	 *
	 * @note Users should derive from GameObject or TextObject instead of Object directly.
	 */
    Object(ObjectType objectType) : type(objectType) {}
    ObjectType type;

	/**
	 * @brief Returns the bounding radius of the object for collision or culling.
	 *
	 * @details
	 * This is a virtual function and should be overridden by derived classes
	 * to provide a meaningful bounding radius used for frustum culling, broad-phase
	 * collision detection, or debug drawing.
	 *
	 * @return Bounding radius in world units.
	 */
    [[nodiscard]] virtual float GetBoundingRadius() const;

    bool isAlive = true;
    bool isVisible = true;

    bool ignoreCamera = false;
    Camera2D* referenceCamera = nullptr;

    std::string objectTag;

    Transform2D transform2D;
    Material* material = nullptr;
    Mesh* mesh = nullptr;
    uint8_t renderLayer = 0;

    glm::vec4 color = glm::vec4(1);

    std::unique_ptr<SpriteAnimator> spriteAnimator;
    std::unique_ptr<Collider> collider;
   
    uint32_t collisionCategory = 0;
    uint32_t collisionMask = 0;

    bool flipUV_X = false;
    bool flipUV_Y = false;
};
