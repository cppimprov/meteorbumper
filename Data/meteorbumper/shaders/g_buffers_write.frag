#version 400

layout(location = 0) out vec4 g_buffer_1;
layout(location = 1) out vec4 g_buffer_2;
layout(location = 2) out vec4 g_buffer_3;

// buffer1: diffuse.xyz, metallic
// buffer2: normal.xyz, roughness
// buffer3: depth (float as vec3), emissive


// Packing technique based on a (now non-existent) gamedev.net topic that used to be here:
// http://www.gamedev.net/community/forums/topic.asp?topic_id=463075&whichpage=1&#3054958

const vec4 pack_bit_shift = vec4(256.0 * 256.0 * 256.0, 256.0 * 256.0, 256.0, 1.0);
const vec4 unpack_bit_shift = 1.0 / pack_bit_shift;
const vec4 bit_mask = vec4(0.0, vec3(1.0 / 256.0));

// pack [0-1] float into vec with 8-bits components
vec4 float_to_vec4(const in float value) {
	vec4 res = fract(value * pack_bit_shift.xyzw);
	return res - res.xxyz * bit_mask.xyzw;
}
vec3 float_to_vec3(const in float value) {
	vec3 res = fract(value * pack_bit_shift.yzw);
	return res - res.xxy * bit_mask.xyz;
}
vec2 float_to_vec2(const in float value) {
	vec2 res = fract(value * pack_bit_shift.zw);
	return res - res.xx * bit_mask.xy;
}


void g_set_diffuse(const in vec3 diffuse) {
	g_buffer_1.xyz = diffuse;
}

void g_set_normal(const in vec3 normal) {
	g_buffer_2.xyz = normal.xyz * 0.5 + 0.5;
}

void g_set_depth(const in float depth) {
	g_buffer_3.xyz = float_to_vec3(depth);
}

void g_set_material(const in float metallic, const in float roughness, const in float emissive) {
	g_buffer_1.w = metallic;
	g_buffer_2.w = roughness;
	g_buffer_3.w = emissive;
}
