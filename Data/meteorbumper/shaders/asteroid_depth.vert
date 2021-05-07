#version 400

in vec3 in_VertexPosition;
in mat4 in_MVP;
in float in_Scale;

void main()
{
	gl_Position = in_MVP * vec4(in_VertexPosition * in_Scale, 1.0);
}
