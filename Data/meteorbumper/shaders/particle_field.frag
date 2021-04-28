#version 400

float g_TYPE_OBJECT;
void g_set_diffuse(const in vec3 diffuse);
void g_set_object_type(const in float id);

in vec3 vert_Color;
in float vert_Alpha;

void main()
{
	// scale alpha with distance from center so particles look round, not square
	float shape_alpha = 1.0 - pow(distance(gl_PointCoord.xy, vec2(0.5)) * 2.0, 2.0);

	//out_Color = vec4(vert_Color, vert_Alpha * shape_alpha);

	g_set_object_type(g_TYPE_OBJECT);
	g_set_diffuse(vert_Color * vert_Alpha * shape_alpha); // todo: NOT THIS!!! needs to be in transparent pass
}
