#pragma once
#include "GameObject.h"
#include "Mesh.h"
#include "Material.h"
#include <unordered_map>
#include <vector>

class SpriteSheet;

/**
 * @brief Key used to batch draw calls for instanced rendering.
 *
 * @details
 * Two objects can be batched together when they share the same Mesh,
 * Material, and SpriteSheet pointers. This key groups such objects.
 */
struct InstanceBatchKey
{
    Mesh* mesh;           ///< Pointer identity used for grouping.
    Material* material;   ///< Pointer identity used for grouping.
    SpriteSheet* spriteSheet; ///< Pointer identity used for grouping.

    /**
     * @brief Equality by pointer identity for all three members.
     */
    bool operator==(const InstanceBatchKey& other) const
    {
        return mesh == other.mesh && material == other.material && spriteSheet == other.spriteSheet;
    }

    /**
     * @brief Strict weak ordering for associative containers.
     * @details Lexicographic compare of (mesh, material, spriteSheet) by pointer value.
     */
    bool operator<(const InstanceBatchKey& other) const
    {
        if (mesh != other.mesh) return mesh < other.mesh;
        if (material != other.material) return material < other.material;
        return spriteSheet < other.spriteSheet;
    }
};

namespace std
{
    /**
     * @brief Hash functor for InstanceBatchKey (pointer-based).
     * @details Mixes member pointer hashes with shifts/xor for use in unordered containers.
     */
    template<>
    struct hash<InstanceBatchKey>
    {
        std::size_t operator()(const InstanceBatchKey& key) const noexcept
        {
            std::size_t h1 = std::hash<Mesh*>()(key.mesh);
            std::size_t h2 = std::hash<Material*>()(key.material);
            std::size_t h3 = std::hash<SpriteSheet*>()(key.spriteSheet);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}

/**
 * @brief Convenience alias for a two-level batch map.
 * @details
 * Maps an int(layer) bucket to { InstanceBatchKey -> vector<GameObject*> }.
 */
using InstancedBatchMap = std::unordered_map<int, std::unordered_map<InstanceBatchKey, std::vector<GameObject*>>>;
