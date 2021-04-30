#version 400

float g_TYPE_OBJECT;
void g_set_diffuse(const in vec3 diffuse);
void g_set_object_type(const in float id);
void g_set_normal(const in vec3 normal);

in vec3 vert_Color;
in vec3 vert_Normal;

void main()
{
	g_set_diffuse(vert_Color);
	g_set_object_type(g_TYPE_OBJECT);
	g_set_normal(vert_Normal);
}
