#include "Engine.h"
#include "gl.h"


GLenum ToGL(PrimitiveType type)
{
    switch (type)
    {
    case PrimitiveType::Triangles: return GL_TRIANGLES;
    case PrimitiveType::Lines: return GL_LINES;
    case PrimitiveType::Points: return GL_POINTS;
    case PrimitiveType::TriangleFan: return GL_TRIANGLE_FAN;
    case PrimitiveType::TriangleStrip: return GL_TRIANGLE_STRIP;
    case PrimitiveType::LineStrip: return GL_LINE_STRIP;
    }
    return GL_TRIANGLES;
}

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, PrimitiveType primitiveType_) :vao(0), vbo(0), ebo(0), instanceVAO(0), indexCount(0), useIndex(false), primitiveType(primitiveType_)
{
    instanceVBO[0] = instanceVBO[1] = instanceVBO[2] = instanceVBO[3] = 0;
    SetupMesh(vertices, indices);
    ComputeLocalBounds(vertices);
}

void Mesh::Draw() const
{
    BindVAO(false);
    GLenum mode = ToGL(primitiveType);

    if (useIndex)
    {
        glDrawElements(mode, indexCount, GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawArrays(mode, 0, indexCount);
    }
}

void Mesh::DrawInstanced(GLsizei instanceCount) const
{
    BindVAO(true);
    GLenum mode = ToGL(primitiveType);

    if (useIndex)
        glDrawElementsInstanced(mode, indexCount, GL_UNSIGNED_INT, nullptr, instanceCount);
    else
        glDrawArraysInstanced(mode, 0, indexCount, instanceCount);
}

void Mesh::BindVAO(bool instanced) const
{
    instanced ? glBindVertexArray(instanceVAO) : glBindVertexArray(vao);
}

Mesh::~Mesh()
{
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(4, instanceVBO);
    glDeleteVertexArrays(1, &instanceVAO);
    instanceVAO = vao = 0;
    ebo = vbo = 0;
    instanceVBO[0] = instanceVBO[1] = instanceVBO[2] = instanceVBO[3] = 0;
}

void Mesh::SetupInstanceAttributes() 
{
    if (!instanceVAO)
        glCreateVertexArrays(1, &instanceVAO);

    glVertexArrayVertexBuffer(instanceVAO, 0, vbo, 0, sizeof(Vertex));

    glEnableVertexArrayAttrib(instanceVAO, 0);
    glVertexArrayAttribFormat(instanceVAO, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
    glVertexArrayAttribBinding(instanceVAO, 0, 0);

    glEnableVertexArrayAttrib(instanceVAO, 1);
    glVertexArrayAttribFormat(instanceVAO, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
    glVertexArrayAttribBinding(instanceVAO, 1, 0);

    if (useIndex && ebo)
        glVertexArrayElementBuffer(instanceVAO, ebo);

    if (!instanceVBO[0])
        glGenBuffers(4, instanceVBO);
    GLuint loc;
    glVertexArrayVertexBuffer(instanceVAO, 1, instanceVBO[0], 0, sizeof(glm::mat4));

    for (int i = 0; i < 4; i++)
    {
        loc = 2 + i;
        glEnableVertexArrayAttrib(instanceVAO, loc);
        glVertexArrayAttribFormat(instanceVAO, loc, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4) * i);
        glVertexArrayAttribBinding(instanceVAO, loc, 1);
    }
    glVertexArrayBindingDivisor(instanceVAO, 1, 1);

    loc = 6;
    glVertexArrayVertexBuffer(instanceVAO, 2, instanceVBO[1], 0, sizeof(glm::vec4));
    glEnableVertexArrayAttrib(instanceVAO, loc);
    glVertexArrayAttribFormat(instanceVAO, loc, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(instanceVAO, loc, 2);
    glVertexArrayBindingDivisor(instanceVAO, 2, 1);

    loc = 7;
    glVertexArrayVertexBuffer(instanceVAO, 3, instanceVBO[2], 0, sizeof(glm::vec2));
    glEnableVertexArrayAttrib(instanceVAO, loc);
    glVertexArrayAttribFormat(instanceVAO, loc, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(instanceVAO, loc, 3);
    glVertexArrayBindingDivisor(instanceVAO, 3, 1);

    loc = 8;
    glVertexArrayVertexBuffer(instanceVAO, 4, instanceVBO[3], 0, sizeof(glm::vec2));
    glEnableVertexArrayAttrib(instanceVAO, loc);
    glVertexArrayAttribFormat(instanceVAO, loc, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(instanceVAO, loc, 4);
    glVertexArrayBindingDivisor(instanceVAO, 4, 1);
}


void Mesh::SetupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
{
    useIndex = !indices.empty();
    indexCount = useIndex ? static_cast<GLsizei>(indices.size()) : static_cast<GLsizei>(vertices.size());

    // Create VAO
    glCreateVertexArrays(1, &vao);

    // Create and bind VBO
    glCreateBuffers(1, &vbo);
    glNamedBufferData(vbo, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // Bind VBO to VAO
    glVertexArrayVertexBuffer(vao, 0, vbo, 0,  sizeof(Vertex));

    glEnableVertexArrayAttrib(vao, 0); // position
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
    glVertexArrayAttribBinding(vao, 0, 0);

    glEnableVertexArrayAttrib(vao, 1); // uv
    glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
    glVertexArrayAttribBinding(vao, 1, 0);

    // EBO (Element Buffer)
    if (useIndex)
    {
        glCreateBuffers(1, &ebo);
        glNamedBufferData(ebo, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        glVertexArrayElementBuffer(vao, ebo);
    }
}

void Mesh::UpdateInstanceBuffer(const std::vector<glm::mat4>& transforms, const std::vector<glm::vec4>& colors, const std::vector<glm::vec2>& uvOffsets, const std::vector<glm::vec2>& uvScales) const
{
    BindVAO(true);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(glm::mat4), transforms.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO[1]);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec4), colors.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO[2]);
    glBufferData(GL_ARRAY_BUFFER, uvOffsets.size() * sizeof(glm::vec2), uvOffsets.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO[3]);
    glBufferData(GL_ARRAY_BUFFER, uvScales.size() * sizeof(glm::vec2), uvScales.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
