#version 400

vec2 g_get_target_size();
vec3 g_get_diffuse(const in vec2 tex_coords);
vec3 g_get_normal(const in vec2 tex_coords);
float g_get_depth(const in vec2 tex_coords);
vec3 g_get_vs_position(const in vec2 tex_coords, const in float depth, const in mat4 inv_p_matrix);
void g_get_material(const in vec2 tex_coords, out float metallic, out float roughness, out float emissive);

in vec3 vert_LightDirection;
in vec3 vert_LightColor;

uniform mat4 u_InvProjMatrix;

layout(location = 0) out vec4 out_Color;

const float two_sided = 0.0;

void main()
{
	vec2 uv = gl_FragCoord.xy / g_get_target_size();

	//if (g_get_object_type(uv) == g_TYPE_SKYBOX) // todo: check depth?
	//	discard;

	vec3 d = g_get_diffuse(uv);
	vec3 n = g_get_normal(uv);
	float depth = g_get_depth(uv);
	vec3 p = g_get_vs_position(uv, depth, u_InvProjMatrix);

	vec3 v = normalize(-p); // pixel to viewer
	vec3 l = normalize(-vert_LightDirection); // pixel to light
	vec3 h = normalize(v + l);

	vec3 lc = vert_LightColor;

	float n_dot_h = dot(n, h);
	n_dot_h = n_dot_h < 0.0 ? two_sided * -n_dot_h : n_dot_h; // todo: mix?
	
	float n_dot_l = dot(n, l);
	n_dot_l = n_dot_l < 0.0 ? two_sided * -n_dot_l : n_dot_l; // todo: mix?
	
	vec3 color = (d + d * pow(n_dot_h, 100.0)) * lc * n_dot_l;

	out_Color = vec4(color, 1.0);
}
