#version 400

in vec3 geom_Color;

layout(location = 0) out vec4 out_Color;

void main()
{
	out_Color = vec4(geom_Color.rgb, 1.0);
}
