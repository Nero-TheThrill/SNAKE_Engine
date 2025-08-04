#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <array>

#include "Debug.h"

/**
 * @brief Manages user-defined named render layers and maps them to internal layer IDs.
 *
 * @details
 * This class allows users to register named render layers (e.g., "UI", "Background") and internally
 * maps them to small integer IDs for optimized sorting and rendering.
 *
 * The system supports up to MAX_LAYERS layers (default: 16), each with a unique name and ID.
 * Layers are used in RenderManager to control draw order across multiple categories of objects.
 *
 * @note RenderLayerManager is used internally by the RenderManager. Users register layers through RenderManager::RegisterRenderLayer().
 */
class RenderLayerManager
{
    friend RenderManager;
public:
    static constexpr uint8_t MAX_LAYERS = 16;

    /**
     * @brief Retrieves the internal ID of a registered render layer by name.
     *
     * @details
     * Looks up the numeric layer ID associated with the given layer name.
     * If the name is not registered, std::nullopt is returned.
     *
     * @param name Name of the render layer.
     * @return The ID of the layer if found, otherwise std::nullopt.
     *
     * @code
     * auto id = manager.GetLayerID("UI");
     * if (id) useLayer(*id);
     * @endcode
     */
    [[nodiscard]] std::optional<uint8_t> GetLayerID(const std::string& name) const
    {
        auto it = nameToID.find(name);
        if (it != nameToID.end())
            return it->second;
        return std::nullopt;
    }

    /**
     * @brief Retrieves the name of a render layer by its internal ID.
     *
     * @details
     * Returns the string name associated with the given layer ID.
     * This is used for reverse lookup or debugging.
     *
     * @param id Numeric render layer ID.
     * @return The corresponding name of the render layer.
     *
     * @note Throws std::out_of_range if the ID is invalid.
     */
    [[nodiscard]] const std::string& GetLayerName(uint8_t id) const
    {
        return idToName.at(id);
    }

private:
    /**
     * @brief Registers a new named render layer.
     *
     * @details
     * Associates the given name with the next available layer ID (0 to MAX_LAYERS-1).
     * If the name is already registered or the layer limit is exceeded,
     * a warning or error will be printed and the operation is skipped.
     *
     * This method is called internally by RenderManager.
     *
     * @param name Unique name of the render layer to register.
     *
     * @note Maximum number of layers is limited to MAX_LAYERS (default: 16).
     */
    void RegisterLayer(const std::string& name)
    {
        auto it = nameToID.find(name);
        if (it != nameToID.end())
        {
            SNAKE_WRN("Already have '" << name << "' tag.\n");
            return;
        }

        if (nextID >= MAX_LAYERS)
        {
            SNAKE_ERR("Already have max count of layers.\n");
            return;
        }

        nameToID[name] = nextID;
        idToName[nextID] = name;
        nextID++;
    }

    std::unordered_map<std::string, uint8_t> nameToID;
    std::array<std::string, MAX_LAYERS> idToName;
    uint8_t nextID = 0;
};
