#version 400

in vec3 in_VertexPosition;
in mat4 in_MVP;
in vec3 in_Color;

out vec3 vert_Color;

void main()
{
	vert_Color = in_Color;
	gl_Position = in_MVP * vec4(in_VertexPosition, 1.0);
}