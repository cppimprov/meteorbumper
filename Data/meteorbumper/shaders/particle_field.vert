#version 400

in vec3 in_VertexPosition;

uniform mat4 u_MVP;
uniform float u_Radius;
uniform vec3 u_Offset;
uniform vec3 u_BaseColorRGB;
uniform vec3 u_ColorVariationHSV;

out vec3 vert_Color;
out float vert_Alpha;

// Bob Jenkins "One-At-A-Time" hashing algorithm
// from: https://stackoverflow.com/a/17479300/
uint hash(uint x)
{
	x += (x << 10u);
	x ^= (x >>  6u);
	x += (x <<  3u);
	x ^= (x >> 11u);
	x += (x << 15u);
	return x;
}

// construct a float with half-open range [0:1] using low 23 bits
// all zeroes yields 0.0, all ones yields the next smallest representable value below 1.0
// from: https://stackoverflow.com/a/17479300/
float float_construct(uint m)
{
    const uint ieee_mantissa = 0x007FFFFFu;
    const uint ieee_one = 0x3F800000u;
    m &= ieee_mantissa;
    m |= ieee_one;
    return uintBitsToFloat(m) - 1.0;
}

// from: https://stackoverflow.com/a/17897228
// All components are in the range [0…1], including hue.
vec3 rgb_to_hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

// from: https://stackoverflow.com/a/17897228
// All components are in the range [0…1], including hue.
vec3 hsv_to_rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
	// get vector from vertex to field position
	// if we're more than 0.5 away, we need to move that particle over by one radius
	vec3 d = (u_Offset - in_VertexPosition);
	bvec3 wrap = greaterThan(abs(d), vec3(0.5));
	vec3 pos = mix(in_VertexPosition, in_VertexPosition + sign(d), wrap);
	gl_Position = u_MVP * vec4(pos * 2.0 * u_Radius, 1.0);

	// get color using the base and random color variation
	vec3 base_hsv = rgb_to_hsv(u_BaseColorRGB);
	vec3 col_rng = vec3(
		float_construct(hash(gl_VertexID + 867)),
		float_construct(hash(gl_VertexID + 3461)),
		float_construct(hash(gl_VertexID + 125))) * 2.0 - 1.0;
	vec3 col_hsv = base_hsv + col_rng * u_ColorVariationHSV;
	col_hsv.x = fract(col_hsv.x);
	col_hsv.yz = clamp(col_hsv.yz, 0.0, 1.0);
	vert_Color = hsv_to_rgb(col_hsv);

	// scale alpha with distance between min_d (1.0) and max_d (0.0)
	float min_d = 0.1;
	float max_d = 0.5;
	vert_Alpha = 1.0 - clamp((distance(pos, u_Offset) - min_d) / (max_d - min_d), 0.0, 1.0);
	
	// scale point size with distance between min_d and max_d
	float min_ps = 0.25;
	float max_ps = 0.5 + float_construct(hash(gl_VertexID + 1235)) * 5.0;
	gl_PointSize = mix(min_ps, max_ps, vert_Alpha);
}
