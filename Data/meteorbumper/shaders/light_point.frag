#version 400

float g_TYPE_NOTHING;
vec3 g_get_diffuse(const in vec2 tex_coords);
float g_get_object_type(const in vec2 tex_coords);
vec3 g_get_ws_normal(const in vec2 tex_coords);
float g_get_depth(const in vec2 tex_coords);
vec3 g_get_ws_position(const in vec2 tex_coords, const in float depth, const in mat4 inv_p_matrix, const in mat4 inv_v_matrix);

in vec3 vert_LightPosition;
in vec3 vert_LightColor;
in float vert_LightRadius;

uniform mat4 u_InvProjMatrix;
uniform mat4 u_InvViewMatrix;

layout(location = 0) out vec4 out_Color;

void main()
{
	vec2 uv = gl_FragCoord.xy / vec2(textureSize(g_buffer_1, 0));

	vec3 d = g_get_diffuse(uv);

	if (g_get_object_type(vert_UV) == g_TYPE_NOTHING)
	{
		out_Color = vec4(d, 1.0);
		return;
	}

	vec3 n = g_get_ws_normal(vert_UV);
	float depth = g_get_depth(vert_UV);
	vec3 p = g_get_ws_position(vert_UV, depth, u_InvProjMatrix, u_InvViewMatrix);
	vec3 p_to_l = (vert_LightPosition - p);

	if (length(p_to_l) > vert_LightRadius)
		discard;

	vec3 v = normalize(-p); // pixel to viewer // is this correct? surely not!!!!
	vec3 l = normalize(p_to_l); // pixel to light
	vec3 h = normalize(v + l);

	vec3 lc = vert_LightColor;

	float n_dot_h = dot(n, h);
	n_dot_h = n_dot_h < 0.0 ? two_sided * -n_dot_h : n_dot_h; // todo: mix?
	
	float n_dot_l = dot(n, l);
	n_dot_l = n_dot_l < 0.0 ? two_sided * -n_dot_l : n_dot_l; // todo: mix?

	vec3 color = (d + d * pow(n_dot_h, 100.0)) * lc * n_dot_l * attenuation;

	out_Color = vec4(color, 1.0);
}
