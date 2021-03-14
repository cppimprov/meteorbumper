#version 400

in vec3 vert_Normal;

uniform samplerCube u_CubemapTexture;

layout(location = 0) out vec4 out_Color;

void main()
{
	//out_Color = vec4(vert_Normal * 0.5 + 0.5, 1.0);
	out_Color = vec4(texture(u_CubemapTexture, vert_Normal).rgb, 1.0);
}
