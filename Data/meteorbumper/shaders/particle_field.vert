#version 400

in vec3 in_VertexPosition;

uniform mat4 u_MVP;

uniform float u_Radius;
uniform vec3 u_Offset;

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


void main()
{
	// get vector from vertex to field position
	// if we're more than 0.5 away, we need to move that particle over by one radius
	vec3 d = (u_Offset - in_VertexPosition);
	bvec3 wrap = greaterThan(abs(d), vec3(0.5));
	vec3 pos = mix(in_VertexPosition, in_VertexPosition + sign(d), wrap);
	gl_Position = u_MVP * vec4(pos * 2.0 * u_Radius, 1.0);

	float rand_01 = float_construct(hash(gl_VertexID + 867));
	vert_Color = rand_01 * vec3(1.0);

	// scale alpha with distance between min_d (1.0) and max_d (0.0)
	float min_d = 0.1;
	float max_d = 0.5;
	vert_Alpha = 1.0 - clamp((distance(pos, u_Offset) - min_d) / (max_d - min_d), 0.0, 1.0);
	
	// scale point size with distance between min_d and max_d
	float min_ps = 0.25;
	float max_ps = 0.5 + rand_01 * 5.0; // shouldn't use same random number for this as color (?)
	gl_PointSize = mix(min_ps, max_ps, vert_Alpha);
}
