#version 400

// g_buffers_read.frag:
vec2 g_get_target_size();
vec3 g_get_diffuse(const in vec2 tex_coords);
vec3 g_get_normal(const in vec2 tex_coords);
float g_get_depth(const in vec2 tex_coords);
vec3 g_get_vs_position(const in vec2 tex_coords, const in float depth, const in mat4 inv_p_matrix);
void g_get_material(const in vec2 tex_coords, out float metallic, out float roughness, out float emissive);

// lighting.frag:
vec3 cook_torrance(vec3 n, vec3 v, vec3 l, vec3 l_color, vec3 albedo, float metallic, float roughness, float emissive);

in vec3 vert_LightDirection;
in vec3 vert_LightColor;

uniform mat4 u_LightViewMatrix;
uniform mat4 u_InvViewMatrix;
uniform mat4 u_InvProjMatrix;

uniform sampler2D u_Shadows;

layout(location = 0) out vec4 out_Color;

void main()
{
	vec2 uv = gl_FragCoord.xy / g_get_target_size();

	vec3 d = g_get_diffuse(uv);
	vec3 n = g_get_normal(uv);
	vec3 p = g_get_vs_position(uv, g_get_depth(uv), u_InvProjMatrix);
	vec3 v = normalize(-p); // pixel to viewer
	vec3 l = normalize(-vert_LightDirection); // pixel to light
	vec3 c = vert_LightColor;
	
	float metallic, roughness, emissive;
	g_get_material(uv, metallic, roughness, emissive);

	vec3 color = cook_torrance(n, v, l, c, d, metallic, roughness, emissive);

	// shadow! todo: put in lighting.frag
	vec4 p_ws = u_InvViewMatrix * vec4(p, 1.0);
	vec4 p_ls = u_LightViewMatrix * p_ws;
	p_ls.xyz /= p_ls.w;
	float shadow_depth = texture(u_Shadows, p_ls.xy * 0.5 + 0.5).x * 2.0 - 1.0;
	color = vec3(shadow_depth > p_ls.z ? 1.0 : 0.0);

	// TODO: only do this for the first light!!!!!!
	
	out_Color = vec4(color, 1.0);
}
