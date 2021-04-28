#version 400

float g_TYPE_OBJECT;
void g_set_diffuse(const in vec3 diffuse);
void g_set_object_type(const in float id);

in vec3 geom_Color;

//layout(location = 0) out vec4 out_Color;

void main()
{
	//out_Color = vec4(geom_Color.rgb, 1.0);

	g_set_object_type(g_TYPE_OBJECT); // TODO: not this!!!! needs to be in transparent render pass!
	g_set_diffuse(geom_Color);
}
