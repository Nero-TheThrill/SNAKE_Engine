#pragma once
#include "glm.hpp"

class Transform2D
{
public:
    /**
     * @brief Initializes position(0,0), rotation(0), scale(1,1), matrix(identity), and marks dirty.
     */
    Transform2D()
        : position(0.f), rotation(0.f), scale(1.f),
        matrix(1.f), isChanged(true)
    {
    }

    /**
     * @brief Sets world-space position and marks the transform dirty.
     * @param pos New position (x,y).
     */
    void SetPosition(const glm::vec2& pos)
    {
        position = pos;
        isChanged = true;
    }

    /**
     * @brief Adds an offset to the current position and marks dirty.
     * @param pos Delta to add (x,y).
     */
    void AddPosition(const glm::vec2& pos)
    {
        position += pos;
        isChanged = true;
    }

    /**
     * @brief Sets rotation and marks dirty.
     * @details Passed directly to glm::rotate in GetMatrix().
     * @param rot Angle value required by glm::rotate.
     */
    void SetRotation(float rot)
    {
        rotation = rot;
        isChanged = true;
    }

    /**
     * @brief Adds to the current rotation and marks dirty.
     * @details Passed directly to glm::rotate in GetMatrix().
     * @param rot Angle delta.
     */
    void AddRotation(float rot)
    {
        rotation += rot;
        isChanged = true;
    }

    /**
     * @brief Sets non-uniform scale and marks dirty.
     * @param scl New scale (sx, sy).
     */
    void SetScale(const glm::vec2& scl)
    {
        scale = scl;
        isChanged = true;
    }

    /**
     * @brief Adds to the current scale and marks dirty.
     * @param scl Scale delta (sx, sy).
     */
    void AddScale(const glm::vec2& scl)
    {
        scale += scl;
        isChanged = true;
    }

    /**
     * @brief Returns the current position.
     */
    [[nodiscard]] const glm::vec2& GetPosition() const { return position; }

    /**
     * @brief Returns the current rotation value.
     */
    [[nodiscard]] float GetRotation() const { return rotation; }

    /**
     * @brief Returns the current scale.
     */
    [[nodiscard]] const glm::vec2& GetScale() const { return scale; }

    /**
     * @brief Returns the 4x4 transform matrix (lazy-recomputed if dirty).
     *
     * @details
     * Rebuilds the matrix when 'isChanged' is true using:
     * T = translate(position.x, position.y, 0)
     * R = rotate around Z by 'rotation' (as required by glm::rotate)
     * S = scale(scale.x, scale.y, 1)
     * Final composition: matrix = T * R * S; then clears 'isChanged'.
     *
     * @return Modifiable reference to the internal matrix.
     */
    [[nodiscard]] glm::mat4& GetMatrix();

private:
    glm::vec2 position;
    float rotation;
    glm::vec2 scale;
    glm::mat4 matrix;
    bool isChanged;
};
