#include "Mesh.h"

#include <iostream>
#include <glad/gl.h>
#include "../ThirdParty/glm/glm.hpp"

Mesh::Mesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) :vao(0), vbo(0), ebo(0), indexCount(0), useIndex(false)
{
    SetupMesh(vertices, indices);
    ComputeLocalBounds(vertices);
}

void Mesh::Draw() const
{
    glBindVertexArray(vao);

    if (useIndex)
    {
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, indexCount);
    }
}

void Mesh::DrawInstanced(GLsizei instanceCount) const
{
    if (useIndex)
        glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr, instanceCount);
    else
        glDrawArraysInstanced(GL_TRIANGLES, 0, indexCount, instanceCount);
}

void Mesh::BindVAO() const
{
    glBindVertexArray(vao);
}

Mesh::~Mesh()
{
    if (ebo) glDeleteBuffers(1, &ebo);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
}

void Mesh::SetupInstanceAttributes(GLuint instanceVBO[]) const
{
    glVertexArrayVertexBuffer(vao, 1, instanceVBO[0], 0, sizeof(glm::mat4));

    for (int i = 0; i < 4; i++)
    {
        GLuint loc = 2 + i;
        glEnableVertexArrayAttrib(vao, loc);
        glVertexArrayAttribFormat(vao, loc, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4) * i);
        glVertexArrayAttribBinding(vao, loc, 1);
    }
    glVertexArrayBindingDivisor(vao, 1, 1);

    glVertexArrayVertexBuffer(vao, 2, instanceVBO[1], 0, sizeof(glm::vec4));
    GLuint loc = 6;
    glEnableVertexArrayAttrib(vao, loc);
    glVertexArrayAttribFormat(vao, loc, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao, loc, 2);
    glVertexArrayBindingDivisor(vao, 2, 1);
}


void Mesh::SetupMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices)
{
    useIndex = !indices.empty();
    indexCount = useIndex ? static_cast<GLsizei>(indices.size()) : static_cast<GLsizei>(vertices.size() / 3);

    // Create VAO
    glCreateVertexArrays(1, &vao);

    // Create and bind VBO
    glCreateBuffers(1, &vbo);
    glNamedBufferData(vbo, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Bind VBO to VAO
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, 3 * sizeof(float));

    glEnableVertexArrayAttrib(vao, 0); // position
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao, 0, 0);

    glEnableVertexArrayAttrib(vao, 1); // uv
    glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 3);
    glVertexArrayAttribBinding(vao, 1, 0);

    glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(float) * 5);

    // EBO (Element Buffer)
    if (useIndex)
    {
        glCreateBuffers(1, &ebo);
        glNamedBufferData(ebo, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        glVertexArrayElementBuffer(vao, ebo);
    }
}
