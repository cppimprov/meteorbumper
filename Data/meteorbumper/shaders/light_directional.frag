#version 400

// g_buffers_read.frag:
vec2 g_get_target_size();
vec3 g_get_diffuse(const in vec2 tex_coords);
vec3 g_get_normal(const in vec2 tex_coords);
float g_get_depth(const in vec2 tex_coords);
vec3 g_get_vs_position(const in vec2 tex_coords, const in float depth, const in mat4 inv_p_matrix);
void g_get_material(const in vec2 tex_coords, out float metallic, out float roughness, out float emissive);

// lighting.frag:
vec3 cook_torrance(vec3 n, vec3 v, vec3 l, vec3 light_color, vec3 albedo, float metallic, float roughness, float emissive);

in vec3 vert_LightDirection;
in vec3 vert_LightColor;

uniform mat4 u_InvProjMatrix;

layout(location = 0) out vec4 out_Color;

void main()
{
	vec2 uv = gl_FragCoord.xy / g_get_target_size();

	//if (g_get_object_type(uv) == g_TYPE_SKYBOX) // todo: check depth?
	//	discard;

	vec3 d = g_get_diffuse(uv);
	vec3 n = g_get_normal(uv);
	vec3 p = g_get_vs_position(uv, g_get_depth(uv), u_InvProjMatrix);
	vec3 v = normalize(-p); // pixel to viewer
	vec3 l = normalize(-vert_LightDirection); // pixel to light
	vec3 c = vert_LightColor;
	
	float metallic, roughness, emissive;
	g_get_material(uv, metallic, roughness, emissive);

	vec3 color = cook_torrance(n, v, l, c, d, metallic, roughness, emissive);

	out_Color = vec4(color, 1.0);
}
