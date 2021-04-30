#version 400

in vec3 vert_Color;
in float vert_Alpha;

layout(location = 0) out vec4 out_Color;

void main()
{
	// scale alpha with distance from center so particles look round, not square
	float shape_alpha = 1.0 - pow(distance(gl_PointCoord.xy, vec2(0.5)) * 2.0, 2.0);

	out_Color = vec4(vert_Color, vert_Alpha * shape_alpha);
}
