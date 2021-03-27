#version 400

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

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

	// get a perpendicular vector to render a "wide line"
	// (glLineWidth doesn't work, so we have to render a quad here)
	const float beamRadius = 0.15; // in world space!
	vec3 perp = cross(vert_Direction[0], vec3(0.0, 1.0, 0.0));

	gl_Position = u_MVP * vec4(head - perp * beamRadius, 1.0);
	geom_Color = vert_Color[0];
	EmitVertex();
	
	gl_Position = u_MVP * vec4(tail - perp * beamRadius, 1.0);
	geom_Color = vert_Color[0];
	EmitVertex();

	gl_Position = u_MVP * vec4(head + perp * beamRadius, 1.0);
	geom_Color = vert_Color[0];
	EmitVertex();

	gl_Position = u_MVP * vec4(tail + perp * beamRadius, 1.0);
	geom_Color = vert_Color[0];
	EmitVertex();

	EndPrimitive();
}
