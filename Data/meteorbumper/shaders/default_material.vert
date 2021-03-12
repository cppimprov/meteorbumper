#version 400

in vec3 in_VertexPosition;

uniform mat4 u_MVP;

void main()
{
	gl_Position = u_MVP * vec4(in_VertexPosition, 1.0);
}