#pragma once
#include "GameObject.h"
#include "Mesh.h"
#include "Material.h"

/**
 * @brief Represents a unique key for grouping objects by mesh and material.
 *
 * @details
 * This struct is used to group GameObjects that share the same Mesh and Material.
 * It serves as a key in containers such as std::map or std::unordered_map,
 * allowing instanced rendering to efficiently batch draw calls with minimal GPU state changes.
 *
 * The comparison operators ('==' and '<') allow it to be used in both hashed and ordered containers.
 * Note that pointer identity is used for comparison-objects must share exact Mesh and Material instances.
 *
 * @note Used internally in RenderManager's instanced rendering pipeline.
 */
struct InstanceBatchKey
{
    Mesh* mesh;
    Material* material;

    /**
     * @brief Checks whether two batch keys refer to the exact same mesh and material.
     *
     * @details
     * Used in hash-based containers like std::unordered_map to determine key equality.
     * Comparison is performed using raw pointer equality, not value comparison.
     *
     * @param other Another InstanceBatchKey to compare with.
     * @return true if both mesh and material pointers are identical.
     */
    bool operator==(const InstanceBatchKey& other) const {
        return mesh == other.mesh && material == other.material;
    }

    /**
     * @brief Defines a strict ordering of batch keys for use in ordered containers.
     *
     * @details
     * Compares mesh pointers first; if equal, compares material pointers.
     * This allows use of InstanceBatchKey as a key in std::map or std::set.
     *
     * @param other Another InstanceBatchKey to compare with.
     * @return true if this key is ordered before the other.
     */
    bool operator<(const InstanceBatchKey& other) const {
        if (mesh != other.mesh)
            return mesh < other.mesh;
        return material < other.material;
    }
};


namespace std
{
    /**
     * @brief Custom hash function to support using InstanceBatchKey in unordered_map.
     *
     * @details
     * Combines the hash values of the mesh and material pointers.
     * This hash function allows InstanceBatchKey to be used as a key in std::unordered_map.
     *
     * @note Uses XOR and left shift to mix two pointer hashes.
     */
    template<>
    struct hash<InstanceBatchKey>
    {
        std::size_t operator()(const InstanceBatchKey& key) const noexcept
        {
            return hash<Mesh*>()(key.mesh) ^ (hash<Material*>()(key.material) << 1);
        }
    };
}

/**
 * @brief Type alias for organizing GameObjects into instancing-compatible groups.
 *
 * @details
 * This two-level map groups GameObjects first by render layer (int),
 * then by unique mesh-material pair (InstanceBatchKey).
 * Used by RenderManager to batch render instances efficiently.
 *
 * Structure:
 * - outer key: render layer (e.g., UI, Background)
 * - inner key: InstanceBatchKey (mesh + material)
 * - value: list of GameObject pointers to render
 *
 * @note This structure is rebuilt every frame before instanced draw calls.
 */
using InstancedBatchMap = std::unordered_map<int, std::unordered_map<InstanceBatchKey, std::vector<GameObject*>>>;
