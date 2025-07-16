#pragma once
#include <vector>
#include "Material.h"


class ObjectManager;
typedef unsigned int GLuint;
typedef int GLsizei;
/**
 * @brief Represents a GPU-side 2D mesh including vertex and optional index data.
 *
 * @details
 * Mesh manages OpenGL VAO/VBO/EBO buffers and provides draw functions
 * for both normal and instanced rendering. It also computes and stores
 * the local 2D bounding size of the mesh to support visibility culling.
 *
 * Mesh instances are owned and drawn by RenderManager.
 */
class Mesh {
    friend Material;
    friend ObjectManager;

public:
    /**
     * @brief Constructs a mesh from vertex data (and optionally indices).
     *
     * @details
     * Vertex layout is assumed to be 3 floats per vertex: (x, y, z).
     * If indices are provided, indexed drawing will be used.
     * Computes local bounds automatically for culling use.
     *
     * @param vertices Flat list of 3D positions (x, y, z).
     * @param indices Optional index list.
     */
    Mesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices = {});

    /**
     * @brief Releases GPU buffers associated with this mesh.
     */
    ~Mesh();

    /**
     * @brief Returns half the width and height of this mesh in local space.
     *
     * @return Half extents of the mesh bounds (used for culling).
     */
    glm::vec2 GetLocalBoundsHalfSize() const { return localHalfSize; }

private:
    /**
     * @brief Binds the VAO of this mesh.
     *
     * @details
     * Required before issuing draw calls or updating instance attributes.
     */
    void BindVAO() const;

    /**
     * @brief Configures instance matrix attributes in the VAO.
     *
     * @details
     * Used by instanced rendering systems to bind a mat4 buffer to this mesh.
     *
     * @param instanceVBO OpenGL buffer holding instance matrix data.
     */
    void SetupInstanceAttributes(GLuint instanceVBO[]) const;

    /**
     * @brief Draws the mesh using either glDrawArrays or glDrawElements.
     */
    void Draw() const;

    /**
     * @brief Draws this mesh multiple times using instanced rendering.
     *
     * @param instanceCount Number of instances to draw.
     */
    void DrawInstanced(GLsizei instanceCount) const;

    /**
     * @brief Initializes OpenGL buffers (VAO, VBO, and optionally EBO).
     *
     * @param vertices Flat vertex list.
     * @param indices Optional index list.
     */
    void SetupMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices);

    /**
     * @brief Computes the 2D bounding box from the provided vertex list.
     *
     * @details
     * Only the x and y components of each vertex are considered.
     * If vertex data is malformed, uses (0.5, 0.5) as fallback.
     *
     * @param vertices Input vertex list.
     */
    void ComputeLocalBounds(const std::vector<float>& vertices)
    {
        if (vertices.empty() || vertices.size() % 3 != 0)
        {
            localHalfSize = glm::vec2(0.5f); // fallback
            return;
        }

        glm::vec2 minPos(vertices[0], vertices[1]);
        glm::vec2 maxPos(vertices[0], vertices[1]);

        for (size_t i = 0; i < vertices.size(); i += 3)
        {
            glm::vec2 pos(vertices[i], vertices[i + 1]);

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
    glm::vec2 localHalfSize;
};
