#version 400

in vec3 in_VertexPosition;
in vec3 in_LightPosition;
in vec3 in_LightColor;
in vec3 in_LightRadius;

uniform mat4 u_MVP;

out vec3 vert_LightPosition;
out vec3 vert_LightColor;
out vec3 vert_LightRadius;

void main()
{
	vert_LightPosition = in_LightPosition;
	vert_LightColor = in_LightColor;
	vert_LightRadius = in_LightRadius;

	gl_Position = u_MVP * vec4(in_LightPosition + in_VertexPosition * in_LightRadius, 1.0);
}
