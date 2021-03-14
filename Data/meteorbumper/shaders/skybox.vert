#version 400

in vec3 in_VertexPosition;

uniform mat4 u_MVP;
uniform float u_Scale;

out vec3 vert_Normal;

void main()
{
	vert_Normal = in_VertexPosition * 2.0;

	gl_Position = u_MVP * vec4(in_VertexPosition * u_Scale, 1.0);
}