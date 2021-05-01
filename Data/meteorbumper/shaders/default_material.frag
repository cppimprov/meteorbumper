#version 400

float g_TYPE_OBJECT;
void g_set_diffuse(const in vec3 diffuse);
void g_set_object_type(const in float id);
void g_set_normal(const in vec3 normal);
void g_set_depth(const in float depth);

in vec3 vert_Normal;
in vec2 vert_Depth;

uniform vec3 u_Color;

void main()
{
	g_set_diffuse(u_Color);
	g_set_object_type(g_TYPE_OBJECT);
	g_set_normal(vert_Normal);
	g_set_depth(vert_Depth.x / vert_Depth.y);
}
