#version 400

in vec2 vert_UV;

uniform vec3 u_Color; // todo: vec4

layout(location = 0) out vec4 out_Color;

void main()
{
	out_Color = vec4(u_Color, 1.0);
}
