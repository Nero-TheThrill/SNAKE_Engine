#version 460 core

layout (location = 0) in vec3 aPos;
layout(location = 1) in vec2 a_UV;
out vec2 v_UV;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;


void main()
{
    gl_Position = u_Projection * u_View * u_Model * vec4(aPos, 1.0);
    v_UV = a_UV;
}
