#version 400

// lighting.frag:
float attenuation_inv_sqr(float l_distance);
vec3 cook_torrance(vec3 n, vec3 v, vec3 l, vec3 l_color, vec3 albedo, float metallic, float roughness, float emissive);

in vec3 vert_Position;
in vec3 vert_Normal;
in vec2 vert_Depth;

uniform vec3 u_Color;
uniform float u_Metallic;
uniform float u_Roughness;
uniform float u_Emissive;
uniform float u_Opacity;

const int dir_light_max = 3;
uniform vec3 u_DirLightDirection[dir_light_max];
uniform vec3 u_DirLightColor[dir_light_max];

const int point_light_max = 5;
uniform vec3 u_PointLightPosition[point_light_max];
uniform vec3 u_PointLightColor[point_light_max];
uniform float u_PointLightRadius[point_light_max];

layout(location = 0) out vec4 out_Color;

void main()
{
	vec3 d = u_Color;
	vec3 n = vert_Normal;
	vec3 p = vert_Position;
	vec3 v = normalize(-p);

	vec3 color = vec3(0.0);
	
	for (int i = 0; i != dir_light_max; ++i)
	{
		vec3 l = normalize(-u_DirLightDirection[i]);
		vec3 c = u_DirLightColor[i];
		
		color += cook_torrance(n, v, l, c, d, u_Metallic, u_Roughness, u_Emissive);
	}

	for (int i = 0; i != point_light_max; ++i)
	{
		vec3  o = u_PointLightPosition[i];
		vec3  l = normalize(o - p);
		vec3  c = u_PointLightColor[i];
		float r = u_PointLightRadius[i];

		float x = length(o - p);
		float a = attenuation_inv_sqr(x) * step(x, r);
		color += cook_torrance(n, v, l, c * a, d, u_Metallic, u_Roughness, u_Emissive);
	}

	color += u_Emissive * d;

	out_Color = vec4(color, u_Opacity);
}
