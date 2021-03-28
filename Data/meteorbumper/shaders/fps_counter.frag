#version 400

in vec2 vert_UV;

uniform vec3 u_Color;
uniform vec2 u_CharOrigin;
uniform vec2 u_CharSize;
uniform sampler2D u_CharTexture;

layout(location = 0) out vec4 out_Color;

void main()
{
	vec2 texel = u_CharOrigin + vert_UV * u_CharSize;
	out_Color = vec4(u_Color, texelFetch(u_CharTexture, ivec2(texel), 0).r);
}
