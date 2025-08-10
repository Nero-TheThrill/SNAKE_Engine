#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include "Camera2D.h"

/**
 * @brief Manages a set of named Camera2D instances and an active camera tag.
 *
 * @details
 * - Cameras are owned via std::unique_ptr and stored by string tag.
 * - Registering with an existing tag overwrites the previous camera.
 * - Active camera changes only if the tag exists.
 * - Clear() empties the registry but does not reset the active tag string.
 */
class CameraManager
{
public:
    /** @brief Constructs an empty manager (no cameras, active tag is empty). */
    CameraManager();

    /**
     * @brief Registers (or replaces) a camera under a tag.
     * @param tag Unique name.
     * @param camera Ownership is transferred into the manager.
     * @note If @p tag already exists, the previous camera is replaced.
     */
    void RegisterCamera(const std::string& tag, std::unique_ptr<Camera2D> camera)
    {
        cameraMap[tag] = std::move(camera);
    }

    /**
     * @brief Finds a camera by tag.
     * @return Camera pointer if found; nullptr otherwise.
     */
    [[nodiscard]] Camera2D* GetCamera(const std::string& tag) const
    {
        auto it = cameraMap.find(tag);
        return (it != cameraMap.end()) ? it->second.get() : nullptr;
    }

    /**
     * @brief Sets the active camera tag if the tag exists.
     * @param tag Must be present in the registry; otherwise unchanged.
     */
    void SetActiveCamera(const std::string& tag)
    {
        if (cameraMap.find(tag) != cameraMap.end())
        {
            activeCameraTag = tag;
        }
    }

    /**
     * @brief Returns the active camera (may be nullptr).
     * @details Looks up the current active tag via GetCamera(activeCameraTag).
     */
    [[nodiscard]] Camera2D* GetActiveCamera() const
    {
        return GetCamera(activeCameraTag);
    }

    /**
     * @brief Returns the current active camera tag (may be empty).
     */
    [[nodiscard]] const std::string& GetActiveCameraTag() const
    {
        return activeCameraTag;
    }

    /**
     * @brief Sets screen size on all registered cameras.
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
     * @brief Sets screen size on the specified camera.
     * @note Uses operator[]: if @p tag does not exist, an empty entry is created
     *       with a null camera and nothing else happens.
     */
    void SetScreenSize(const std::string& tag, int width, int height)
    {
        if (cameraMap[tag])
            cameraMap[tag]->SetScreenSize(width, height);
    }

    /**
     * @brief Removes all cameras from the registry.
     * @note Does not modify the active tag string.
     */
    void Clear()
    {
        cameraMap.clear();
    }

private:
    std::unordered_map<std::string, std::unique_ptr<Camera2D>> cameraMap;
    std::string activeCameraTag;
};
