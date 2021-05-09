#version 400

in vec3 in_Position;
in vec4 in_Color;
in float in_Size;

uniform mat4 u_MVP;

out vec4 vert_Color;
out vec3 vert_Position;

void main()
{
	vert_Color = in_Color;
	vert_Position = in_Position;

	gl_PointSize = in_Size;
	gl_Position = u_MVP * vec4(in_Position, 1.0);
}
