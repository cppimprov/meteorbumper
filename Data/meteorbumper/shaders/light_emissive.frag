#version 400

// g_buffers_read.frag:
vec2 g_get_target_size();
vec3 g_get_diffuse(const in vec2 tex_coords);
void g_get_material(const in vec2 tex_coords, out float metallic, out float roughness, out float emissive);

layout(location = 0) out vec4 out_Color;

void main()
{
	vec2 uv = gl_FragCoord.xy / g_get_target_size();

	vec3 d = g_get_diffuse(uv);
	
	float metallic, roughness, emissive;
	g_get_material(uv, metallic, roughness, emissive);

	vec3 color = emissive * d;
	
	out_Color = vec4(color, 1.0);
}
