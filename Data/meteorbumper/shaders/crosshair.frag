#version 400

in vec2 vert_UV;

uniform vec4 u_Color;

layout(location = 0) out vec4 out_Color;

void main()
{
	vec2 uv = vert_UV;

    float radius = distance(uv, vec2(0.5)) * 2.0;
    
    float ring = 1.0 - smoothstep(distance(radius, 0.75), 0.0, 0.02);
    float center = 1.0 - smoothstep(distance(radius, 0.0), 0.0, 0.04);
    
    float pips = 1.0 - smoothstep(distance(radius, 0.75), 0.0, 0.1);
    float out_border = smoothstep(min(radius, 0.9999), 1.0, 0.95);
    float in_border = smoothstep(max(radius, 0.5001), 0.50, 0.55);
    float pips_x = out_border * in_border * pips * (1.0 - smoothstep(distance(uv.x, 0.5), 0.0, 0.01));
    float pips_y = out_border * in_border * pips * (1.0 - smoothstep(distance(uv.y, 0.5), 0.0, 0.01));

	float crosshair = ring + center + pips_x + pips_y;

	out_Color = vec4(u_Color.rgb, u_Color.a * crosshair);
}
