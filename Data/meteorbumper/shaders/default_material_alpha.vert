#version 400

in vec3 in_VertexPosition;
in vec3 in_VertexNormal;

uniform mat4 u_MV;
uniform mat4 u_MVP;
uniform mat3 u_NormalMatrix;

out vec3 vert_Position;
out vec3 vert_Normal;
out vec2 vert_Depth;

void main()
{
	vert_Position = vec3(u_MV * vec4(in_VertexPosition, 1.0));
	vert_Normal = u_NormalMatrix * in_VertexNormal;
	gl_Position = u_MVP * vec4(in_VertexPosition, 1.0);
	vert_Depth = gl_Position.zw;
}
