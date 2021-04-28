#version 400

vec3 g_get_diffuse(const in vec2 tex_coords);

in vec2 vert_UV;

//uniform sampler2D u_Texture;

layout(location = 0) out vec4 out_Color;

void main()
{
	vec2 uv = vert_UV;
	//vec3 color = texture(u_Texture, uv).rgb;

	vec3 color = g_get_diffuse(uv);

	out_Color = vec4(color, 1.0);
}
