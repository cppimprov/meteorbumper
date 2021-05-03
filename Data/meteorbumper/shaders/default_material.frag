#version 400

void g_set_diffuse(const in vec3 diffuse);
void g_set_normal(const in vec3 normal);
void g_set_depth(const in float depth);
void g_set_material(const in float metallic, const in float roughness, const in float emissive);

in vec3 vert_Normal;
in vec2 vert_Depth;

uniform vec3 u_Color;
uniform float u_Metallic;
uniform float u_Roughness;
uniform float u_Emissive;

void main()
{
	g_set_diffuse(u_Color);
	g_set_normal(vert_Normal);
	g_set_depth(vert_Depth.x / vert_Depth.y);
	g_set_material(u_Metallic, u_Roughness, u_Emissive);
}
