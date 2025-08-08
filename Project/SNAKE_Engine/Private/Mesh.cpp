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

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, PrimitiveType primitiveType_) :vao(0), vbo(0), ebo(0), indexCount(0), useIndex(false), primitiveType(primitiveType_)
{
    instanceVBO[0] = instanceVBO[1] = instanceVBO[2] = instanceVBO[3] = 0;
    SetupMesh(vertices, indices);
    ComputeLocalBounds(vertices);
}

void Mesh::Draw() const
{
    glBindVertexArray(vao);
    GLenum mode = ToGL(primitiveType);

    glBindVertexArray(vao);

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
    glBindVertexArray(vao);
    GLenum mode = ToGL(primitiveType);

    if (useIndex)
        glDrawElementsInstanced(mode, indexCount, GL_UNSIGNED_INT, nullptr, instanceCount);
    else
        glDrawArraysInstanced(mode, 0, indexCount, instanceCount);
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

void Mesh::SetupInstanceAttributes() 
{
    if (!instanceVBO[0])
        glGenBuffers(4, instanceVBO);
    GLuint loc;
    glVertexArrayVertexBuffer(vao, 1, instanceVBO[0], 0, sizeof(glm::mat4));

    for (int i = 0; i < 4; i++)
    {
        loc = 2 + i;
        glEnableVertexArrayAttrib(vao, loc);
        glVertexArrayAttribFormat(vao, loc, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4) * i);
        glVertexArrayAttribBinding(vao, loc, 1);
    }
    glVertexArrayBindingDivisor(vao, 1, 1);

    loc = 6;
    glVertexArrayVertexBuffer(vao, 2, instanceVBO[1], 0, sizeof(glm::vec4));
    glEnableVertexArrayAttrib(vao, loc);
    glVertexArrayAttribFormat(vao, loc, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao, loc, 2);
    glVertexArrayBindingDivisor(vao, 2, 1);

    loc = 7;
    glVertexArrayVertexBuffer(vao, 3, instanceVBO[2], 0, sizeof(glm::vec2));
    glEnableVertexArrayAttrib(vao, loc);
    glVertexArrayAttribFormat(vao, loc, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao, loc, 3);
    glVertexArrayBindingDivisor(vao, 3, 1);

    loc = 8;
    glVertexArrayVertexBuffer(vao, 4, instanceVBO[3], 0, sizeof(glm::vec2));
    glEnableVertexArrayAttrib(vao, loc);
    glVertexArrayAttribFormat(vao, loc, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao, loc, 4);
    glVertexArrayBindingDivisor(vao, 4, 1);
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
