#version 400

uniform sampler2D u_Texture;

in vec2 vert_UV;

layout(location = 0) out vec4 out_Color;

void main()
{
	vec3 color = texture(u_Texture, vert_UV).rgb;

	float exposure = 2.0; // todo: make this configurable?
	vec3 mapped = vec3(1.0) - exp(-color * exposure);

	out_Color = vec4(mapped, 1.0);
}
