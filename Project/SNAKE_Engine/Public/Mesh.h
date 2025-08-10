#pragma once
#include <vector>
#include "Material.h"

class ObjectManager;

using GLuint = unsigned int;
using GLsizei = int;

/** @brief Primitive topology for drawing. */
enum class PrimitiveType
{
    Triangles,
    Lines,
    Points,
    TriangleFan,
    TriangleStrip,
    LineStrip
};

/**
 * @brief Interleaved vertex for this engine's 2D/3D sprites/geometry.
 * @details
 * - position: xyz in object-local space.
 * - uv:       texture coordinates.
 */
struct Vertex
{
    glm::vec3 position;
    glm::vec2 uv;
};

/**
 * @brief GPU mesh wrapper with optional index buffer and instancing buffers.
 *
 * @details
 * - Stores VAOs/VBOs (and EBO if indices are provided).
 * - Computes and caches local half-size bounds in XY (see GetLocalBoundsHalfSize()).
 * - Supports instanced rendering via per-instance buffers:
 *   transform (mat4), color (vec4), uv offset (vec2), uv scale (vec2).
 *
 * @note Draw/DrawInstanced are invoked by the renderer; end users typically do not call
 *       private binding/setup APIs directly.
 */
class Mesh {
    friend Material;
    friend RenderManager;

public:
    /**
     * @brief Builds a mesh from vertices and (optionally) indices.
     * @param vertices Interleaved array of Vertex.
     * @param indices  Optional index list; if empty, non-indexed drawing is used.
     * @param primitiveType Primitive topology used when drawing.
     */
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices = {}, PrimitiveType primitiveType = PrimitiveType::Triangles);

    /**
     * @brief Releases GPU resources held by this mesh.
     */
    ~Mesh();

    /**
     * @brief Returns cached local-space half extents in XY.
     * @details
     * Computed from vertex positions (Z is ignored). See ComputeLocalBounds().
     */
    [[nodiscard]] glm::vec2 GetLocalBoundsHalfSize() const { return localHalfSize; }

private:
    /**
     * @brief Binds the appropriate VAO for drawing.
     * @param instanced If true, binds the instanced VAO; otherwise the regular VAO.
     * @note Internal. Used by Material/RenderManager.
     */
    void BindVAO(bool instanced) const;

    /**
     * @brief Prepares per-instance vertex attributes/buffers for instanced drawing.
     * @details
     * Defines attribute streams for: mat4 transforms, vec4 colors, vec2 uv offsets, vec2 uv scales.
     * @note Internal. Called during mesh setup.
     */
    void SetupInstanceAttributes();

    /**
     * @brief Issues a non-instanced draw using the configured primitive type.
     * @note Internal. Used by the renderer.
     */
    void Draw() const;

    /**
     * @brief Issues an instanced draw for @p instanceCount instances.
     * @param instanceCount Number of instances to draw.
     * @note Internal. Before calling, ensure UpdateInstanceBuffer() has been provided with matching arrays.
     */
    void DrawInstanced(GLsizei instanceCount) const;

    /**
     * @brief Uploads vertex/index data to GPU and finalizes the mesh object.
     * @param vertices Vertex array.
     * @param indices  Optional index array; if empty, the mesh draws without indices.
     * @note Internal. Called by the constructor.
     */
    void SetupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);

    /**
     * @brief Calculates local XY half extents from the given vertex list.
     *
     * @details
     * - If @p vertices is empty: sets half size to (0.5, 0.5).
     * - If it has exactly one vertex: sets half size to (0.0001, 0.0001).
     * - Otherwise: scans all positions (XY only) to build an AABB and stores half of its size.
     *
     * @param vertices Source vertices for bounds computation.
     * @note Stored in @ref localHalfSize.
     */
    void ComputeLocalBounds(const std::vector<Vertex>& vertices)
    {
        if (vertices.empty())
        {
            localHalfSize = glm::vec2(0.5f);
            return;
        }
        else if (vertices.size() == 1)
        {
            localHalfSize = glm::vec2(0.0001f);
            return;
        }

        glm::vec2 minPos = vertices[0].position;
        glm::vec2 maxPos = vertices[1].position;

        for (const auto& v : vertices)
        {
            glm::vec2 pos = v.position;
            minPos = glm::min(minPos, pos);
            maxPos = glm::max(maxPos, pos);
        }

        glm::vec2 size = maxPos - minPos;
        localHalfSize = size * 0.5f;
    }

    /**
     * @brief Updates per-instance buffers used by instanced rendering.
     *
     * @param transforms Per-instance model matrices (size = instanceCount).
     * @param colors     Per-instance colors (size = instanceCount).
     * @param uvOffsets  Per-instance UV offsets (size = instanceCount).
     * @param uvScales   Per-instance UV scales (size = instanceCount).
     *
     * @note Internal. Called by the renderer before DrawInstanced().
     */
    void UpdateInstanceBuffer(const std::vector<glm::mat4>& transforms, const std::vector<glm::vec4>& colors, const std::vector<glm::vec2>& uvOffsets, const std::vector<glm::vec2>& uvScales) const;

    GLuint vao;          ///< Regular VAO.
    GLuint vbo;          ///< Vertex buffer.
    GLuint ebo;          ///< Index buffer (if used).
    GLsizei indexCount;  ///< Number of indices (0 for non-indexed).

    GLuint instanceVAO;  ///< VAO configured for instanced attributes.
    GLuint instanceVBO[4]; ///< Instance buffers: [0]=mat4, [1]=vec4 color, [2]=vec2 uvOffset, [3]=vec2 uvScale.

    bool useIndex;       ///< Whether indices are used.

    PrimitiveType primitiveType; ///< Primitive topology used in Draw*().
    glm::vec2 localHalfSize;     ///< Cached XY half extents in local space.
};
