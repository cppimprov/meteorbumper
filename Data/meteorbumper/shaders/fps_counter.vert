#version 400

in vec2 in_VertexPosition;

uniform mat4 u_MVP;
uniform vec2 u_Position;
uniform vec2 u_Size;

out vec2 vert_UV;

void main()
{
	vert_UV = in_VertexPosition;
	
	gl_Position = u_MVP * vec4(u_Position + in_VertexPosition * u_Size, 0.0, 1.0);
}
