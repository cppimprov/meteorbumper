#version 400

in vec4 vert_Color;

layout(location = 0) out vec4 out_Color;

void main()
{
	out_Color = vec4(vert_Color.rgb * vert_Color.a, 1.0);
}
