#version 400

// lighting.frag:
float shadow(vec3 p_ws, mat4 light_matrix, sampler2D shadow_map);

in vec4 vert_Color;
in vec3 vert_Position;

uniform mat4 u_LightViewProjMatrix;
uniform sampler2D u_Shadows;
uniform float u_EnableShadows;

layout(location = 0) out vec4 out_Color;

void main()
{
	float s = shadow(vert_Position, u_LightViewProjMatrix, u_Shadows);
	vec3 color = mix(vert_Color.rgb, vert_Color.rgb * 0.15, s * u_EnableShadows);

	out_Color = vec4(color, vert_Color.a);
}
