#version 400

in vec2 vert_UV;

uniform vec3 u_Color;
uniform sampler2D u_TextTexture;

layout(location = 0) out vec4 out_Color;

void main()
{
	out_Color = vec4(u_Color, texture(u_TextTexture, vert_UV).r);
}
