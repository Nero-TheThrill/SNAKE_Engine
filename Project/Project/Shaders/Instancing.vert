#version 330 core

layout (location = 0) in vec3 aPos;
layout(location = 1) in vec2 a_UV;
layout(location = 2) in mat4 i_Model;
layout(location = 6) in vec4 i_Color;
out vec2 v_UV;
out vec4 v_Color;
uniform mat4 u_Projection;


void main()
{
    gl_Position = u_Projection * i_Model * vec4(aPos, 1.0);
    v_UV=a_UV;
	v_Color = i_Color;
}
