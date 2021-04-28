#version 400

float g_TYPE_OBJECT;
void g_set_diffuse(const in vec3 diffuse);
void g_set_object_type(const in float id);

in vec4 vert_Color;

void main()
{
	g_set_diffuse(vert_Color.rgb * vert_Color.a); // TODO: not this! needs to be in transparent render pass
	g_set_object_type(g_TYPE_OBJECT);
}
