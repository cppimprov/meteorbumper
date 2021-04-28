#version 400

in vec3 in_VertexPosition;
in vec3 in_VertexNormal;

uniform mat4 u_MVP;
uniform mat3 u_NormalMatrix;

out vec3 vert_Normal;

void main()
{
	vert_Normal = u_NormalMatrix * in_VertexNormal;
	gl_Position = u_MVP * vec4(in_VertexPosition, 1.0);
}