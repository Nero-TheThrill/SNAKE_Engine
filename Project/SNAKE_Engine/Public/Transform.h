#pragma once
#include "glm.hpp"
/**
 * @brief Represents a 2D transformation including position, rotation, and scale.
 *
 * @details
 * Transform2D stores 2D spatial information used by game objects such as position, rotation, and scale.
 * It computes a model matrix on demand when changes occur, and avoids recomputation unless necessary.
 * This class is widely used in rendering, physics, and animation systems to represent object transforms.
 *
 * @note All matrix updates are lazy; GetMatrix() triggers recalculation only if changes were made.
 */
class Transform2D
{
public:
    Transform2D()
        : position(0.f), rotation(0.f), scale(1.f),
        matrix(1.f), isChanged(true)
    {
    }

    /**
	 * @brief Sets the object's world position.
	 *
	 * @details
	 * Replaces the current position with the provided value and marks the transform as changed.
	 * This will trigger a matrix recomputation on the next GetMatrix() call.
	 *
	 * @param pos The new world position.
	 *
	 * @code
	 * transform.SetPosition({100.0f, 200.0f});
	 * @endcode
	 */
    void SetPosition(const glm::vec2& pos)
    {
        position = pos;
        isChanged = true;
    }

    /**
	 * @brief Offsets the current position by a given amount.
	 *
	 * @details
	 * Adds the specified vector to the current position and flags the transform as changed.
	 * This ensures the matrix will be updated on the next retrieval.
	 *
	 * @param pos The amount to offset the position by.
	 *
	 * @code
	 * transform.AddPosition({5.0f, -10.0f});
	 * @endcode
	 */
    void AddPosition(const glm::vec2& pos)
    {
        position += pos;
        isChanged = true;
    }
    /**
	 * @brief Sets the object's rotation angle in radians.
	 *
	 * @details
	 * Replaces the current rotation value with the given angle in radians.
	 * Marks the transform as changed, so the model matrix will be recalculated on next access.
	 *
	 * @param rot Rotation angle in radians.
	 *
	 * @code
	 * transform.SetRotation(glm::radians(90.0f));
	 * @endcode
	 */
    void SetRotation(float rot)
    {
        rotation = rot;
        isChanged = true;
    }

    /**
	 * @brief Adds an angle to the current rotation.
	 *
	 * @details
	 * Increments the current rotation angle by the given amount in radians.
	 * Marks the transform as dirty, so the matrix will be updated when needed.
	 *
	 * @param rot Angle to add in radians.
	 *
	 * @code
	 * transform.AddRotation(glm::radians(15.0f));
	 * @endcode
	 */
    void AddRotation(float rot)
    {
        rotation += rot;
        isChanged = true;
    }

	/**
	 * @brief Sets the object's scale factor.
	 *
	 * @details
	 * Replaces the current scale with the given value and flags the transform as changed.
	 * The scale is applied separately in the X and Y axes.
	 *
	 * @param scl New scale value as a 2D vector.
	 *
	 * @code
	 * transform.SetScale({1.0f, 2.0f});
	 * @endcode
	 */
    void SetScale(const glm::vec2& scl)
    {
        scale = scl;
        isChanged = true;
    }

	/**
	 * @brief Increments the object's current scale.
	 *
	 * @details
	 * Adds the given vector to the current scale, affecting the object's size on the X and Y axes.
	 * Marks the transform as changed so the model matrix is updated later.
	 *
	 * @param scl Amount to add to the current scale.
	 *
	 * @code
	 * transform.AddScale({0.5f, -0.25f});
	 * @endcode
	 */
    void AddScale(const glm::vec2& scl)
    {
        scale += scl;
        isChanged = true;
    }

	/**
	 * @brief Returns the current world position.
	 *
	 * @return 2D vector representing the position.
	 */
    [[nodiscard]] const glm::vec2& GetPosition() const { return position; }

	/**
	 * @brief Returns the current rotation angle in radians.
	 *
	 * @return Rotation value in radians.
	 */
    [[nodiscard]] float GetRotation() const { return rotation; }

	/**
	 * @brief Returns the current scale value.
	 *
	 * @return 2D vector representing the X and Y scale factors.
	 */
    [[nodiscard]] const glm::vec2& GetScale() const { return scale; }

	/**
	 * @brief Returns the 2D transformation matrix.
	 *
	 * @details
	 * Computes and returns the combined translation, rotation, and scale matrix.
	 * If the transform hasn't changed since the last call, returns the cached matrix.
	 *
	 * @return Reference to the transformation matrix.
	 *
	 * @code
	 * glm::mat4 model = transform.GetMatrix();
	 * @endcode
	 */
    [[nodiscard]] glm::mat4& GetMatrix();

private:
    glm::vec2 position;
    float rotation;
    glm::vec2 scale;
    glm::mat4 matrix;
    bool isChanged;
};
