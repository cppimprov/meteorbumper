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
float shadow(vec3 p_ws, vec3 n, vec3 l, mat4 light_matrix, sampler2D shadow_map);

in vec3 vert_LightDirection;
in vec3 vert_LightColor;
in float vert_LightShadows;

uniform mat4 u_LightViewProjMatrix;
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

	vec3 p_ws = vec3(u_InvViewMatrix * vec4(p, 1.0));

	float s = shadow(p_ws, n, l, u_LightViewProjMatrix, u_Shadows);
	color = mix(color, vec3(0.0), s * vert_LightColor);
	
	out_Color = vec4(color, 1.0);
}
