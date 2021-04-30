#version 400

float g_TYPE_SKYBOX;
vec2 g_get_target_size();
vec3 g_get_diffuse(const in vec2 tex_coords);
float g_get_object_type(const in vec2 tex_coords);

layout(location = 0) out vec4 out_Color;

void main()
{
	vec2 uv = gl_FragCoord.xy / g_get_target_size();

	if (g_get_object_type(uv) != g_TYPE_SKYBOX)
		discard;

	vec3 d = g_get_diffuse(uv);

	out_Color = vec4(d, 1.0);
}
