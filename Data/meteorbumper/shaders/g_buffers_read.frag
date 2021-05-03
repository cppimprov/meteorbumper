#version 400

uniform sampler2D g_buffer_1, g_buffer_2, g_buffer_3;

// buffer1: diffuse.xyz, object_type_id
// buffer2: normal.xyz, undef
// buffer3: depth (float as vec3), undef


// Packing technique based on a (now non-existent) gamedev.net topic that used to be here:
// http://www.gamedev.net/community/forums/topic.asp?topic_id=463075&whichpage=1&#3054958

const vec4 pack_bit_shift = vec4(256.0 * 256.0 * 256.0, 256.0 * 256.0, 256.0, 1.0);
const vec4 unpack_bit_shift = 1.0 / pack_bit_shift;
const vec4 bit_mask = vec4(0.0, vec3(1.0 / 256.0));

// unpack [0-1] float from vec with 8-bit components
float vec4_to_float(const in vec4 value) {
	return dot(value, unpack_bit_shift.xyzw);
}
float vec3_to_float(const in vec3 value) {
	return dot(value, unpack_bit_shift.yzw);
}
float vec2_to_float(const in vec2 value) {
	return dot(value, unpack_bit_shift.zw);
}


const float g_TYPE_SKYBOX = 0.0;
const float g_TYPE_OBJECT = 1.0 / 255.0;

vec2 g_get_target_size() {
	return vec2(textureSize(g_buffer_1, 0));
}

vec3 g_get_diffuse(const in vec2 tex_coords) {
	return texture(g_buffer_1, tex_coords).rgb;
}

float g_get_object_type(const in vec2 tex_coords) {
	return texture(g_buffer_1, tex_coords).a;
}

vec3 g_get_normal(const in vec2 tex_coords) {
	return texture(g_buffer_2, tex_coords).xyz * 2.0 - 1.0;
}

float g_get_depth(const in vec2 tex_coords) {
	return vec3_to_float(texture(g_buffer_3, tex_coords).rgb);
}

vec3 g_get_ws_position(const in vec2 tex_coords, const in float depth, const in mat4 inv_p_matrix, const in mat4 inv_v_matrix) {
	vec4 ss_pos = vec4(tex_coords * 2.0 - 1.0, depth, 1.0);

	vec4 vs_pos = inv_p_matrix * ss_pos;

	vs_pos.xyz /= vs_pos.w;
	vs_pos.w = 1.0;

	return (inv_v_matrix * vs_pos).xyz;
}

vec3 g_get_vs_position(const in vec2 tex_coords, const in float depth, const in mat4 inv_p_matrix) {
	vec4 ss_pos = vec4(tex_coords * 2.0 - 1.0, depth, 1.0);
	
	vec4 vs_pos = inv_p_matrix * ss_pos;
	
	vs_pos.xyz /= vs_pos.w;
	
	return vs_pos.xyz;
}
