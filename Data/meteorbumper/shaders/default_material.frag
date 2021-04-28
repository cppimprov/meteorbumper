#version 400

float g_TYPE_OBJECT;
void g_set_diffuse(const in vec3 diffuse);
void g_set_object_type(const in float id);

uniform vec3 u_Color;

//layout(location = 0) out vec4 out_Color;

void main()
{
	//out_Color = vec4(u_Color, 1.0);

	g_set_object_type(g_TYPE_OBJECT);
	g_set_diffuse(u_Color);
	// todo: normal
	// todo: depth
}
