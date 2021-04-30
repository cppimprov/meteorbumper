#version 400

uniform sampler2D u_Texture;

in vec2 vert_UV;

layout(location = 0) out vec4 out_Color;

void main()
{
	vec3 color = texture(u_Texture, vert_UV).rgb;

	out_Color = vec4(color, 1.0);
}
