#version 400

in vec2 in_VertexPosition;

uniform mat4 u_MVP;
uniform vec2 u_Size;

void main()
{
	gl_Position = u_MVP * vec4(in_VertexPosition * u_Size, 0.0, 1.0);
}