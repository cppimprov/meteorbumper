#version 400

// g_buffers_read.frag:
vec2 g_get_target_size();
vec3 g_get_diffuse(const in vec2 tex_coords);
vec3 g_get_normal(const in vec2 tex_coords);
float g_get_depth(const in vec2 tex_coords);
vec3 g_get_vs_position(const in vec2 tex_coords, const in float depth, const in mat4 inv_p_matrix);
void g_get_material(const in vec2 tex_coords, out float metallic, out float roughness, out float emissive);

// lighting.frag:
float attenuation_inv_sqr(float l_distance);
vec3 cook_torrance(vec3 n, vec3 v, vec3 l, vec3 l_color, vec3 albedo, float metallic, float roughness, float emissive);

in vec3 vert_LightPosition; // vs
in vec3 vert_LightColor;
in float vert_LightRadius;

uniform mat4 u_InvProjMatrix;

layout(location = 0) out vec4 out_Color;

void main()
{
	vec2 uv = gl_FragCoord.xy / g_get_target_size();

	vec3  c = vert_LightColor;
	vec3  o = vert_LightPosition;
	float r = vert_LightRadius;

	vec3 d = g_get_diffuse(uv);
	vec3 n = g_get_normal(uv);
	vec3 p = g_get_vs_position(uv, g_get_depth(uv), u_InvProjMatrix);
	vec3 v = normalize(-p); // pixel to viewer
	vec3 l = normalize(o - p); // pixel to light

	float metallic, roughness, emissive;
	g_get_material(uv, metallic, roughness, emissive);
	
	float a = attenuation_inv_sqr(length(o - p));
	vec3 color = cook_torrance(n, v, l, c * a, d, metallic, roughness, emissive);

	out_Color = vec4(color, 1.0);
}
