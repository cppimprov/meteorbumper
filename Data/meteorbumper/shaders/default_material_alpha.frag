#version 400

in vec3 vert_Normal;
in vec2 vert_Depth;

uniform vec3 u_Color;
uniform float u_Metallic;
uniform float u_Roughness;
uniform float u_Emissive;
uniform float u_Opacity;

layout(location = 0) out vec4 out_Color;

void main()
{
	//g_set_diffuse(u_Color);
	//g_set_normal(vert_Normal);
	//g_set_depth(vert_Depth.x / vert_Depth.y);
	//g_set_material(u_Metallic, u_Roughness, u_Emissive);

	out_Color = vec4(u_Color, u_Opacity);
}
