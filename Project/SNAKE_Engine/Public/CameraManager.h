#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include "Camera2D.h"

/**
 * @brief Manages multiple 2D cameras and controls the active one.
 *
 * @details
 * The CameraManager class handles registration and lookup of multiple Camera2D instances,
 * allowing flexible use of tagged cameras (e.g., "main", "UI", "background"). It also tracks
 * an active camera which is typically used for rendering or input conversion.
 *
 * In addition, it provides screen size synchronization for all or specific cameras when the
 * window is resized. This class is commonly used by the engine's StateManager and WindowManager.
 *
 * Example usage:
 * @code
 * CameraManager cm;
 * cm.RegisterCamera("main", std::make_unique<Camera2D>(1280, 720));
 * cm.SetActiveCamera("main");
 * Camera2D* cam = cm.GetActiveCamera();
 * @endcode
 */
class CameraManager
{
public:
    /**
     * @brief Constructs the CameraManager and sets up a default camera.
     *
     * @details
     * Initializes the internal camera map and automatically registers a default camera
     * with the tag "main", using screen size (1, 1) and zoom 1.0. The "main" camera is
     * set as the active camera by default.
     *
     * This ensures that a valid camera is always available for rendering or logic even
     * if no additional cameras are registered.
     *
     * @code
     * CameraManager cm;
     * Camera2D* cam = cm.GetActiveCamera(); // "main" by default
     * @endcode
     */
    CameraManager();

    /**
     * @brief Registers a new Camera2D with a unique tag.
     *
     * @details
     * Stores the given Camera2D instance in the internal map using the specified tag.
     * If a camera with the same tag already exists, it will be overwritten.
     *
     * @param tag Identifier for the camera (e.g., "main", "ui").
     * @param camera A unique pointer to a Camera2D object.
     *
     * @code
     * cameraManager.RegisterCamera("main", std::make_unique<Camera2D>(1280, 720));
     * @endcode
     */
    void RegisterCamera(const std::string& tag, std::unique_ptr<Camera2D> camera)
    {
        cameraMap[tag] = std::move(camera);
    }

    /**
     * @brief Retrieves a registered camera by its tag.
     *
     * @details
     * Looks up a Camera2D by its tag. If no camera is registered with the given tag,
     * returns nullptr.
     *
     * @param tag Identifier of the camera to retrieve.
     * @return Pointer to the Camera2D if found; otherwise, nullptr.
     *
     * @code
     * Camera2D* cam = cameraManager.GetCamera("main");
     * @endcode
     */
    [[nodiscard]] Camera2D* GetCamera(const std::string& tag) const
    {
        auto it = cameraMap.find(tag);
        return (it != cameraMap.end()) ? it->second.get() : nullptr;
    }

    /**
     * @brief Sets the active camera used for rendering or input.
     *
     * @details
     * Marks the camera with the given tag as the currently active one.
     * If the tag is not registered, the call is ignored silently.
     *
     * @param tag Tag of the camera to set as active.
     *
     * @code
     * cameraManager.SetActiveCamera("main");
     * @endcode
     */
    void SetActiveCamera(const std::string& tag)
    {
        if (cameraMap.find(tag) != cameraMap.end())
        {
            activeCameraTag = tag;
        }
    }

    /**
     * @brief Returns the currently active camera.
     *
     * @details
     * Retrieves the Camera2D instance that is marked as active. If no camera is
     * explicitly set as active, the default is "main".
     *
     * This is typically used in rendering or input systems to access the view-related
     * parameters like position or zoom.
     *
     * @return Pointer to the active Camera2D instance; may be nullptr if not found.
     *
     * @code
     * Camera2D* cam = cameraManager.GetActiveCamera();
     * @endcode
     */
    [[nodiscard]] Camera2D* GetActiveCamera() const
    {
        return GetCamera(activeCameraTag);
    }

    /**
     * @brief Gets the tag of the currently active camera.
     *
     * @return A reference to the string tag of the active camera.
     *
     * @code
     * std::string tag = cameraManager.GetActiveCameraTag();
     * @endcode
     */
    [[nodiscard]] const std::string& GetActiveCameraTag() const
    {
        return activeCameraTag;
    }

    /**
    * @brief Sets the screen size for all registered cameras.
     *
     * @details
     * Iterates over all cameras in the map and updates their screen width and height.
     * This is typically called when the application window is resized.
     *
     * @param width New screen width in pixels.
     * @param height New screen height in pixels.
     *
     * @code
     * cameraManager.SetScreenSizeForAll(1920, 1080);
     * @endcode
     */
    void SetScreenSizeForAll(int width, int height)
    {
        for (auto& [tag, cam] : cameraMap)
        {
            if (cam)
                cam->SetScreenSize(width, height);
        }
    }

    /**
     * @brief Sets the screen size for a specific camera by tag.
     *
     * @details
     * Updates the width and height of the Camera2D associated with the given tag.
     * If the tag does not exist or maps to a null pointer, this function does nothing.
     *
     * @param tag The tag of the camera to update.
     * @param width New screen width in pixels.
     * @param height New screen height in pixels.
     *
     * @code
     * cameraManager.SetScreenSize("ui", 1920, 1080);
     * @endcode
     */
    void SetScreenSize(const std::string& tag, int width, int height)
    {
        if (cameraMap[tag])
            cameraMap[tag]->SetScreenSize(width, height);
    }

    /**
     * @brief Removes all registered cameras and resets internal state.
     *
     * @details
     * Clears the internal map of all Camera2D instances and releases ownership.
     * After this call, no cameras will be available until re-registered.
     *
     * @code
     * cameraManager.Clear();
     * @endcode
     */
    void Clear()
    {
        cameraMap.clear();
    }

private:
    std::unordered_map<std::string, std::unique_ptr<Camera2D>> cameraMap;
    std::string activeCameraTag;
};
