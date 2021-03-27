#version 400

layout (points) in;
layout (line_strip, max_vertices = 2) out;

in vec3 vert_Color[];
in vec3 vert_Position[];
in vec3 vert_Direction[];
in float vert_BeamLength[];

uniform mat4 u_MVP;

out vec3 geom_Color;

void main()
{
	// turn the point into a line with the specified length
	vec3 head = vert_Position[0];
	vec3 tail = head - (vert_Direction[0] * vert_BeamLength[0]);

	gl_Position = u_MVP * vec4(head, 1.0);
	geom_Color = vert_Color[0];
	EmitVertex();

	gl_Position = u_MVP * vec4(tail, 1.0);
	geom_Color = vert_Color[0];
	EmitVertex();

	EndPrimitive();
}
