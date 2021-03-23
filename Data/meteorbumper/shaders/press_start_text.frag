#version 400

in vec2 vert_UV;

uniform float u_Time;
uniform vec3 u_Color;
uniform sampler2D u_TextTexture;

layout(location = 0) out vec4 out_Color;

void main()
{
	// constants
	const float PI = 3.14159;
	const vec3 c1 = vec3(1.0, 0.8157, 0.0); // yellow
	const vec3 c2 = vec3(1.0, 0.2471, 0.0); // orangey-red
	const vec3 hc1 = vec3(1.0, 0.96, 0.79); // v. bright yellow
	const vec3 hc2 = vec3(0.9); // silver

	{
		// rescale texture coords
		vec2 tex_size = textureSize(u_TextTexture, 0);
		vec2 coord = vert_UV * tex_size; // back to pixel coords

		vec2 uv = coord / 500.0; // set pixel scale
		uv = floor(uv * 64.0) / 64.0; // blockify

		// diagonal, repeating, yellow / red color gradient
		float d = uv.x + -uv.y + floor((u_Time / 10.0) * 64.0) / 64.0;
		float s = sin(d * 10.0) * 0.5 + 0.5;
		vec3 c = mix(c1, c2, s);
		c = floor(c * 16.0) / 16.0; // only use pixel values of 256 / 16

		// intermittent highlight
		float hd = uv.x + -uv.y + floor((u_Time / 2.0) * 64.0) / 64.0;
		float hs = sin(hd * 10.0) * step(fract(((hd * 20.0) / PI) * 0.05), 0.1); // long space between sin peaks
		vec3 hc = mix(hc1, hc2, hs) * hs;

		c += hc;

		out_Color = vec4(c, texture(u_TextTexture, vert_UV).r);
	}
}
