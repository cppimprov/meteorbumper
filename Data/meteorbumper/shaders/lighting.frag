
const float PI = 3.141592653589793;

float distribution_trowbridge_reitz_ggx(float roughness, float ndoth)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float nh2 = ndoth * ndoth;
    float x = (nh2 * (a2 - 1.0) + 1.0);
    float D = a2 / (PI * x * x);

    return D;
}

float geometry_schlick_ggx(float k, float ndotv)
{
    return ndotv / (ndotv * (1.0 - k) + k);
}

float geometry_smith(float roughness, float ndotv, float ndotl)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float Gv = geometry_schlick_ggx(k, ndotv);
    float Gl = geometry_schlick_ggx(k, ndotl);

    return Gv * Gl;
}

vec3 fresnel(vec3 f0, float vdoth)
{
    float p = (-5.55473 * vdoth - 6.98316) * vdoth;
    vec3 F = f0 + (1.0 - f0) * pow(2.0, p);

    return F;
}

vec3 cook_torrance(vec3 n, vec3 v, vec3 l, vec3 light_color, vec3 albedo, float metallic, float roughness, float emissive)
{
	vec3 c = light_color;

	float m = metallic;
	float r = roughness;
	float e = emissive;

	vec3 f0_i = vec3(0.04); // f0 for non-metallic surfaces
	vec3 f0 = mix(f0_i, albedo, m);

	vec3 h = normalize(l + v);

	float ndoth = max(dot(n, h), 0.0);
	float ndotv = max(dot(n, v), 0.0);
	float ndotl = max(dot(n, l), 0.0);
	float vdoth = max(dot(v, h), 0.0);

	float D = distribution_trowbridge_reitz_ggx(r, ndoth);
	float G = geometry_smith(r, ndotv, ndotl);
	vec3  F = fresnel(f0, vdoth);

	vec3 CT = (D * F * G) / max(4.0 * ndotl * ndotv, 0.0000001);

	vec3 kS = F;
	vec3 kD = (1.0 - kS) * (1.0 - m) * (1.0 - e);

	vec3 color = (((kD * albedo) / PI) + CT) * c * ndotl;

	return color;
}
