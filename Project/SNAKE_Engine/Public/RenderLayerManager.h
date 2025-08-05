#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <array>

#include "Debug.h"

class RenderLayerManager
{
    friend RenderManager;
public:
    static constexpr uint8_t MAX_LAYERS = 16;

    [[nodiscard]] std::optional<uint8_t> GetLayerID(const std::string& name) const
    {
        auto it = nameToID.find(name);
        if (it != nameToID.end())
            return it->second;
        return std::nullopt;
    }

    [[nodiscard]] const std::string& GetLayerName(uint8_t id) const
    {
        return idToName.at(id);
    }

private:
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

    void UnregisterLayer(const std::string& name)
    {
        auto it = nameToID.find(name);
        if (it == nameToID.end())
        {
            SNAKE_WRN("Cannot unregister: layer '" << name << "' not found");
            return;
        }

        uint8_t id = it->second;
        nameToID.erase(it);
        idToName[id].clear();
    }
    std::unordered_map<std::string, uint8_t> nameToID;
    std::array<std::string, MAX_LAYERS> idToName;
};
