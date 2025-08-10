#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <array>

#include "Debug.h"

/**
 * @brief Bi-directional registry for named render layers.
 *
 * @details
 * Maps layer names to 0..MAX_LAYERS-1 IDs and back.
 * MAX_LAYERS is fixed at 16.
 */
class RenderLayerManager
{
    friend RenderManager;
public:
    /** @brief Maximum number of layers (IDs 0..15). */
    static constexpr uint8_t MAX_LAYERS = 16;

    /**
     * @brief Looks up a layer ID by name.
     * @param name Registered layer name.
     * @return Layer ID if found; std::nullopt otherwise.
     */
    [[nodiscard]] std::optional<uint8_t> GetLayerID(const std::string& name) const
    {
        auto it = nameToID.find(name);
        if (it != nameToID.end())
            return it->second;
        return std::nullopt;
    }

    /**
     * @brief Returns the layer name for a given ID.
     * @param id Layer ID.
     * @return Reference to the stored name string.
     * @note Expects a valid, registered ID (0..MAX_LAYERS-1).
     */
    [[nodiscard]] const std::string& GetLayerName(uint8_t id) const
    {
        return idToName.at(id);
    }

private:
    /**
     * @brief Registers a name->ID mapping.
     *
     * @details
     * - Fails (returns false) and logs a warning if the name already exists.
     * - Fails (returns false) and logs an error if @p layer is out of range
     *   or the ID is already used.
     * - On success, stores both name->ID and ID->name.
     *
     * @param tag Layer name to register.
     * @param layer Numeric ID (0..MAX_LAYERS-1).
     * @return true if registered; false otherwise.
     * @note Internal. Used by RenderManager.
     */
    [[maybe_unused]] bool RegisterLayer(const std::string& tag, uint8_t layer)
    {
        if (nameToID.find(tag) != nameToID.end())
        {
            SNAKE_WRN("Layer already exists: " << tag);
            return false;
        }

        if (layer >= MAX_LAYERS || !idToName[layer].empty())
        {
            SNAKE_ERR("Layer ID " << layer << " is already in use or out of range");
            return false;
        }

        nameToID[tag] = layer;
        idToName[layer] = tag;
        return true;
    }

    /**
     * @brief Unregisters a layer by name.
     *
     * @details
     * - If the name is not found, logs and returns without changes.
     * - Otherwise, erases both the name->ID and ID->name entries.
     *
     * @param name Previously registered layer name.
     * @note Internal. Used by RenderManager.
     */
    void UnregisterLayer(const std::string& name)
    {
        auto it = nameToID.find(name);
        if (it == nameToID.end())
        {
            SNAKE_LOG("Cannot unregister: layer '" << name << "' not found");
            return;
        }

        uint8_t id = it->second;
        nameToID.erase(it);
        idToName[id].clear();
    }

    /** @brief Name ¡æ ID map. */
    std::unordered_map<std::string, uint8_t> nameToID;
    /** @brief ID -> Name array (size = MAX_LAYERS). Empty string means unused. */
    std::array<std::string, MAX_LAYERS> idToName;
};
