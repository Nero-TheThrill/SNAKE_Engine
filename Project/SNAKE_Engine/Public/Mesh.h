#pragma once
#include <vector>
#include "Material.h"

class ObjectManager;

using GLuint = unsigned int;
using GLsizei = int;

enum class PrimitiveType
{
    Triangles,
    Lines,
    Points,
    TriangleFan,
    TriangleStrip,
    LineStrip
};

struct Vertex
{
    glm::vec3 position;
    glm::vec2 uv;
};

/**
 * @brief Represents a GPU mesh composed of vertices and optional indices.
 *
 * @details
 * The Mesh class manages vertex and index buffers (VBO, EBO) and their associated VAO.
 * It supports multiple primitive types (e.g., triangles, lines, points), and enables both
 * regular and instanced rendering through OpenGL.
 *
 * Meshes are constructed from a list of vertices and optional indices.
 * The class also computes a local bounding box for efficient culling or layout.
 *
 * @note Mesh instances are usually managed by RenderManager and used by Material.
 */
class Mesh {
    friend Material;
    friend RenderManager;

public:
    /**
     * @brief Constructs a mesh from vertex and optional index data.
     *
     * @details
     * Initializes OpenGL buffers (VAO, VBO, and optionally EBO) using the provided vertex data.
     * The mesh can be configured to use triangles, lines, points, etc., through the primitiveType.
     * If index data is not provided, the mesh will be rendered using only vertices.
     *
     * @param vertices List of vertex data including position and UV.
     * @param indices Optional list of indices. If empty, rendering will be non-indexed.
     * @param primitiveType The type of geometric primitive to render (default: Triangles).
     *
     * @code
     * std::vector<Vertex> quadVerts = { ... };
     * std::vector<unsigned> indices = { 0, 1, 2, 2, 3, 0 };
     * Mesh mesh(quadVerts, indices);
     * @endcode
     */
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices = {}, PrimitiveType primitiveType = PrimitiveType::Triangles);

    /**
     * @brief Releases OpenGL resources associated with this mesh.
     *
     * @details
     * Deletes the VAO, VBO, and EBO buffers from GPU memory.
     */
    ~Mesh();

    /**
     * @brief Returns half of the mesh's local axis-aligned bounding box.
     *
     * @details
     * Used for culling, collision approximation, or layout alignment.
     * The result is computed during mesh setup based on vertex positions.
     *
     * @return Half-size of the local bounding box as a 2D vector.
     */
    [[nodiscard]] glm::vec2 GetLocalBoundsHalfSize() const { return localHalfSize; }

private:
    /**
     * @brief Binds the mesh's VAO for rendering.
     *
     * @details
     * Activates the VAO previously created during mesh setup.
     * Required before issuing draw calls.
     *
     * @note Called internally by RenderManager or Material before rendering.
     */
    void BindVAO() const;

    /**
     * @brief Configures instance-specific vertex attributes.
     *
     * @details
     * Binds per-instance data buffers (e.g., transforms) to the mesh's VAO.
     * This enables hardware instancing for efficient rendering of multiple copies.
     *
     * @param instanceVBO Pointer to the instance VBO containing instance data.
     *
     * @note Called only for instanced rendering.
     */
    void SetupInstanceAttributes(GLuint* instanceVBO) const;

    /**
     * @brief Issues a non-instanced draw call for this mesh.
     *
     * @details
     * Draws the mesh using glDrawArrays or glDrawElements depending on whether indices are used.
     * This is the standard rendering path for single objects.
     */
    void Draw() const;

    /**
     * @brief Issues an instanced draw call for multiple instances of this mesh.
     *
     * @details
     * Uses glDrawArraysInstanced or glDrawElementsInstanced to render many copies efficiently.
     *
     * @param instanceCount Number of instances to render.
     */
    void DrawInstanced(GLsizei instanceCount) const;

    /**
     * @brief Initializes OpenGL buffers and configures vertex attributes.
     *
     * @details
     * Creates VAO, VBO, and optionally EBO for the provided vertex and index data.
     * Also computes the local bounding box.
     *
     * @param vertices Vertex data to upload.
     * @param indices Optional index data.
     */
    void SetupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);

    /**
     * @brief Computes the axis-aligned bounding box of the mesh.
     *
     * @details
     * Calculates the min/max extents from the vertex positions and stores half-size.
     * Used for culling decision.
     *
     * @param vertices The vertex data to analyze.
     */
    void ComputeLocalBounds(const std::vector<Vertex>& vertices)
    {
        if (vertices.empty())
        {
            localHalfSize = glm::vec2(0.5f);
            return;
        }
        else if (vertices.size()==1)
        {
            localHalfSize = glm::vec2( 0.0001f);
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

    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLsizei indexCount;

    bool useIndex;

    PrimitiveType primitiveType;
    glm::vec2 localHalfSize;
};
