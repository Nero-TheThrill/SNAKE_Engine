#version 460 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform vec2 u_UVScale;
uniform vec2 u_UVOffset;
out vec2 v_UV;

void main()
{
    v_UV = aUV * u_UVScale + u_UVOffset;
    gl_Position = u_Projection * u_View * u_Model * vec4(aPos, 0.0, 1.0);
}
