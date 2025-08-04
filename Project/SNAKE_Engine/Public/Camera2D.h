#pragma once
#include "glm.hpp"

/**
 * @brief A 2D camera class used for orthographic rendering and view culling.
 *
 * @details
 * The Camera2D class provides basic functionality for controlling the visible area
 * of a 2D scene. It supports panning, zooming, and determining if an object is within
 * the camera's current viewport. The projection matrix is currently constructed using
 * orthographic projection based on the screen size and zoom level.
 *
 * This camera is primarily used during rendering for calculating the visible
 * region and performing frustum culling via IsInView().
 *
 * @note
 * The current implementation returns an orthographic projection matrix via GetProjectionMatrix().
 * In a future update, this function will be removed and replaced with GetViewMatrix().
 *
 * Example usage:
 * @code
 * Camera2D camera(1280, 720);
 * camera.SetPosition({100.0f, 200.0f});
 * glm::mat4 proj = camera.GetProjectionMatrix(); // Deprecated soon
 * @endcode
 */
class Camera2D
{
public:
    /**
     * @brief Constructs a 2D camera with a given screen size.
     *
     * @details
     * Initializes the camera with the specified screen width and height. The default
     * zoom level is set to 1.0, and the initial position is (0, 0).
     *
     * @param screenWidth Width of the camera's viewport in pixels.
     * @param screenHeight Height of the camera's viewport in pixels.
     *
     * @code
     * Camera2D cam(1280, 720);
     * @endcode
     */
    Camera2D(int screenWidth = 800, int screenHeight = 600);

    /**
     * @brief Sets the camera's screen size.
     *
     * @details
     * Updates the internal screen width and height used for calculating the
     * orthographic projection. Should be called whenever the window size changes.
     *
     * @param width New screen width in pixels.
     * @param height New screen height in pixels.
     *
     * @code
     * camera.SetScreenSize(1920, 1080);
     * @endcode
     */
    void SetScreenSize(int width, int height);

    /**
     * @brief Gets the current screen width.
     *
     * @return Screen width in pixels.
     */
    [[nodiscard]] int GetScreenWidth() const { return screenWidth; }

    /**
     * @brief Gets the current screen height.
     *
     * @return Screen height in pixels.
     */
    [[nodiscard]] int GetScreenHeight() const { return screenHeight; }

    /**
     * @brief Sets the camera's world position.
     *
     * @details
     * The position determines the center of the visible viewport in world space.
     * This is used when computing the orthographic projection and view culling.
     *
     * @param pos New camera position in world coordinates.
     *
     * @code
     * camera.SetPosition({ 100.0f, 50.0f });
     * @endcode
     */
    void SetPosition(const glm::vec2& pos);

    /**
     * @brief Moves the camera by a relative offset.
     *
     * @param pos Offset vector to add to the current position.
     *
     * @code
     * camera.AddPosition({ 10.0f, 0.0f }); // move right
     * @endcode
     */
    void AddPosition(const glm::vec2& pos);

    /**
     * @brief Gets the current position of the camera.
     *
     * @return A const reference to the camera's world position.
     */
    [[nodiscard]] const glm::vec2& GetPosition() const;

    /**
     * @brief Sets the zoom level of the camera.
     *
     * @details
     * The zoom factor determines how much of the world is visible on screen.
     * A higher zoom value shows a smaller area (zoom in), and a lower value shows more (zoom out).
     *
     * @param z New zoom level (must be > 0).
     *
     * @code
     * camera.SetZoom(2.0f); // zoom in
     * @endcode
     */
    void SetZoom(float z);

    /**
     * @brief Gets the current zoom level.
     *
     * @return Zoom factor (1.0 by default).
     */
    [[nodiscard]] float GetZoom() const;

    /**
     * @brief Returns the current orthographic projection matrix.
     *
     * @details
     * Builds a projection matrix centered on the camera's position, scaled by the current zoom,
     * and sized according to the screen dimensions.
     *
     * @return A glm::mat4 representing the orthographic projection.
     *
     * @note
     * This function will be deprecated in a future version. Use GetViewMatrix() instead once available.
     */
    [[nodiscard]] glm::mat4 GetProjectionMatrix() const;

    /**
     * @brief Checks whether a circle is within the camera's view.
     *
     * @details
     * Performs a simple bounding sphere check using the camera's position and a viewport size
     * (typically equal to screen size divided by zoom). Used in frustum culling.
     *
     * @param pos World position of the object.
     * @param radius Bounding radius of the object.
     * @param viewportSize Size of the viewport (usually screen size / zoom).
     * @return true if the object is at least partially in view; false otherwise.
     *
     * @code
     * if (camera.IsInView(obj->GetWorldPosition(), obj->GetBoundingRadius(), {1280, 720}))
     *     visibleList.push_back(obj);
     * @endcode
     */
    [[nodiscard]] bool IsInView(const glm::vec2& pos, float radius, glm::vec2 viewportSize) const;

private:
    glm::vec2 position = glm::vec2(0.0f);
    float zoom = 1.0f;
    int screenWidth = 800;
    int screenHeight = 600;
};
