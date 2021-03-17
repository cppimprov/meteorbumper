#version 400

uniform vec3 u_Color;

layout(location = 0) out vec4 out_Color;

void main()
{
	out_Color = vec4(u_Color, 1.0);
}