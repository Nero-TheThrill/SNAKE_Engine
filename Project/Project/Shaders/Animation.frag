#version 460 core

out vec4 FragColor;
in vec2 v_UV;

uniform vec4 u_Color;
uniform sampler2D u_Texture;

void main()
{
    FragColor = texture(u_Texture, v_UV) * u_Color;
}
