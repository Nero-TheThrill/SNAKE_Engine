#pragma once
#include "glm.hpp"

/**
 * @brief Simple 2D camera storing position, zoom, and screen size.
 *
 * @details
 * Provides a view matrix and a basic visibility test against a circular bound.
 * Screen width/height are stored for reference; callers pass a viewport size
 * to IsInView() when performing culling.
 */
class Camera2D
{
public:
    /**
     * @brief Constructs a camera with an initial screen size.
     * @param screenWidth  Initial screen width in pixels.
     * @param screenHeight Initial screen height in pixels.
     */
    Camera2D(int screenWidth = 800, int screenHeight = 600);

    /**
     * @brief Updates the stored screen size.
     * @param width  New width in pixels.
     * @param height New height in pixels.
     */
    void SetScreenSize(int width, int height);

    /** @brief Returns the stored screen width in pixels. */
    [[nodiscard]] int GetScreenWidth() const { return screenWidth; }
    /** @brief Returns the stored screen height in pixels. */
    [[nodiscard]] int GetScreenHeight() const { return screenHeight; }

    /** @brief Sets the camera position (world space). */
    void SetPosition(const glm::vec2& pos);
    /** @brief Adds an offset to the camera position (world space). */
    void AddPosition(const glm::vec2& pos);
    /** @brief Returns the camera position (world space). */
    [[nodiscard]] const glm::vec2& GetPosition() const;

    /**
     * @brief Sets the zoom factor.
     * @details Used as a scale in the view transform.
     */
    void SetZoom(float z);
    /** @brief Returns the current zoom factor. */
    [[nodiscard]] float GetZoom() const;

    /**
     * @brief Returns the view matrix for this camera.
     * @details The matrix applies at least a scale by the current zoom.
     */
    [[nodiscard]] glm::mat4 GetViewMatrix() const;

    /**
     * @brief Checks if a world-space circle intersects the camera view rectangle.
     *
     * @param pos          Circle center in world space.
     * @param radius       Circle radius in world units.
     * @param viewportSize View rectangle size (width, height) in pixels.
     * @return true if the circle is at least partially inside the view; false otherwise.
     */
    [[nodiscard]] bool IsInView(const glm::vec2& pos, float radius, glm::vec2 viewportSize) const;

private:
    glm::vec2 position = glm::vec2(0.0f);
    float zoom = 1.0f;
    int screenWidth = 800;
    int screenHeight = 600;
};
