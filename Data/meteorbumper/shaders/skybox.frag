#version 400

float g_TYPE_NOTHING;
void g_set_diffuse(const in vec3 diffuse);
void g_set_object_type(const in float id);

in vec3 vert_Normal;

uniform samplerCube u_CubemapTexture;

//layout(location = 0) out vec4 out_Color;

void main()
{
	//out_Color = vec4(vert_Normal * 0.5 + 0.5, 1.0);
	//out_Color = vec4(texture(u_CubemapTexture, vert_Normal).rgb, 1.0);

	g_set_object_type(g_TYPE_NOTHING);
	g_set_diffuse(texture(u_CubemapTexture, vert_Normal).rgb);
	// todo: normal
	// todo: depth
}
