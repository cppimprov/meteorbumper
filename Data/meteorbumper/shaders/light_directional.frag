#version 400

float g_TYPE_NOTHING;
vec3 g_get_diffuse(const in vec2 tex_coords);
float g_get_object_type(const in vec2 tex_coords);
vec3 g_get_ws_normal(const in vec2 tex_coords);
float g_get_depth(const in vec2 tex_coords);
vec3 g_get_ws_position(const in vec2 tex_coords, const in float depth, const in mat4 inv_p_matrix, const in mat4 inv_v_matrix);

in vec2 vert_UV;
in vec3 vert_LightDirection;
in vec3 vert_LightColor;

uniform mat4 u_InvProjMatrix;
uniform mat4 u_InvViewMatrix;

layout(location = 0) out vec4 out_Color;

const float two_sided = 0.0;

void main()
{
	vec3 d = g_get_diffuse(vert_UV);

	if (g_get_object_type(vert_UV) == g_TYPE_NOTHING)
	{
		out_Color = vec4(d, 1.0);
		return;
	}

	vec3 n = g_get_ws_normal(vert_UV);
	float depth = g_get_depth(vert_UV);
	vec3 p = g_get_ws_position(vert_UV, depth, u_InvProjMatrix, u_InvViewMatrix);

	vec3 v = normalize(-p); // pixel to viewer // NOPE!!!! this would only be correct in view space!!!
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
