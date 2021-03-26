#version 400

in vec3 in_VertexPosition;
in vec3 in_Color;
in vec3 in_Position;
in vec3 in_Direction;
in float in_BeamLength;

uniform mat4 u_MVP;

out vec3 vert_Color;

void main()
{
	vert_Color = in_Color;

	// TEMP!!!! just use everything for now!!!
	gl_Position = u_MVP * vec4(in_Position + (in_VertexPosition + in_Color + in_Position + in_Direction + in_BeamLength) * 0.000001, 1.0);
}
