#version 400

in vec3 in_Color;
in vec3 in_Position;
in vec3 in_Direction;
in float in_BeamLength;

uniform mat4 u_MVP;

out vec3 vert_Color;
out vec3 vert_Position;
out vec3 vert_Direction;
out float vert_BeamLength;

void main()
{
	vert_Color = in_Color;
	vert_Position = in_Position;
	vert_Direction = in_Direction;
	vert_BeamLength = in_BeamLength;

	gl_Position = u_MVP * vec4(in_Position, 1.0); // todo: is this necessary?
}
