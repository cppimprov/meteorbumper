#version 400

in vec3 in_VertexPosition;

uniform mat4 u_MVP;

uniform float u_Radius;
uniform vec3 u_Offset;

void main()
{
	vec3 d = (u_Offset - in_VertexPosition);
	bvec3 wrap = greaterThan(abs(d), vec3(0.5));
	vec3 pos = mix(in_VertexPosition, in_VertexPosition + sign(d), wrap);

	float diameter = 2.0 * u_Radius;
	gl_Position = u_MVP * vec4(pos * diameter, 1.0);
}
