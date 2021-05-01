#version 400

in vec3 in_VertexPosition;
in vec3 in_VertexNormal;
in mat4 in_MVP;
in mat3 in_NormalMatrix;
in vec3 in_Color;
in float in_Scale;

out vec3 vert_Color;
out vec3 vert_Normal;
out vec2 vert_Depth;

void main()
{
	vert_Color = in_Color;
	vert_Normal = in_NormalMatrix * in_VertexNormal;

	gl_Position = in_MVP * vec4(in_VertexPosition * in_Scale, 1.0);
	vert_Depth = gl_Position.zw;
}
