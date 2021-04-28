#version 400

float g_TYPE_NOTHING;
void g_set_diffuse(const in vec3 diffuse);
void g_set_object_type(const in float id);

in vec3 vert_Normal;

uniform samplerCube u_CubemapTexture;

void main()
{
	g_set_object_type(g_TYPE_NOTHING);
	g_set_diffuse(texture(u_CubemapTexture, vert_Normal).rgb);
	// todo: normal
}
