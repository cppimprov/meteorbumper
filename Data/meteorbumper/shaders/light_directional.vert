#version 400

in vec2 in_VertexPosition;
in vec3 in_LightDirection;
in vec3 in_LightColor;
in float in_LightRadius;

uniform mat4 u_MVP;
uniform vec2 u_Size;

out vec2 vert_UV;
out vec3 vert_LightDirection;
out vec3 vert_LightColor;
out float vert_LightRadius;

void main()
{
	vert_UV = in_VertexPosition;
	vert_LightDirection = in_LightDirection;
	vert_LightColor = in_LightColor;
	vert_LightRadius = in_LightRadius;

	gl_Position = u_MVP * vec4(in_VertexPosition * u_Size, 0.0, 1.0);
}