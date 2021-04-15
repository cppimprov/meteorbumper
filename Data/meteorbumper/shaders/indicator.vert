#version 400

in vec2 in_VertexPosition;
in vec2 in_Direction;
in vec3 in_Color;

uniform mat4 u_MVP;
uniform float u_Size;

out vec3 vert_Color;

void main()
{
	vert_Color = in_Color;

	gl_PointSize = u_Size;
	gl_Position = u_MVP * vec4(in_VertexPosition + in_Direction * 0.0000001, 0.0, 1.0);
}
