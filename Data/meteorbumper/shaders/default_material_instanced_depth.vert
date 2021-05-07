#version 400

in vec3 in_VertexPosition;
in mat4 in_MVP;

void main()
{
	gl_Position = in_MVP * vec4(in_VertexPosition, 1.0);
}
