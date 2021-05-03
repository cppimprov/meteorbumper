
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>

#include <iostream>
#include <random>

void die() { __debugbreak(); }
void die_if(bool condition) { if (condition) die(); }

template<class RNG>
glm::vec3 point_in_ring_3d(RNG& rng, float min_radius, float max_radius)
{
	die_if(max_radius < min_radius);

	auto d = std::uniform_real_distribution<float>(0.f, 1.f);
	auto theta = d(rng) * 2.f * glm::pi<float>();
	auto phi = std::acosf(d(rng) * 2.f - 1.f);
	auto min_r3 = min_radius * min_radius * min_radius;
	auto max_r3 = max_radius * max_radius * max_radius;
	auto radius = std::sqrt(d(rng) * (max_r3 - min_r3) + min_r3);

	return { 
		radius * std::cos(theta) * std::sin(phi),
		radius * std::sin(theta) * std::sin(phi),
		radius * std::cos(phi)
	};
}

std::vector<glm::vec3> generate_normals(std::size_t count)
{
	auto rng = std::mt19937(std::random_device()());

	auto out = std::vector<glm::vec3>();
	out.reserve(count);

	for (auto i = std::size_t{ 0 }; i != count; ++i)
		out.push_back(glm::normalize(point_in_ring_3d(rng, 1.f, 1.f)));

	return out;
}

std::vector<glm::vec3> convert_normals_8bit(std::vector<glm::vec3> const& in)
{
	using rgb8_t = glm::vec<3, std::uint8_t>;
	auto buffer = std::vector<rgb8_t>();
	buffer.reserve(in.size());

	for (auto const& n : in)
	{
		auto np = n * 0.5f + 0.5f;
		buffer.push_back(rgb8_t(np * 255.f));
	}
	
	auto out = std::vector<glm::vec3>();
	out.reserve(in.size());

	for (auto const& b : buffer)
	{
		auto np = glm::vec3(b) / 255.f;
		auto n = np * 2.0f - 1.0f;
		out.push_back(glm::normalize(n));
	}
	
	return out;
}

std::vector<glm::vec3> convert_normals_spherical(std::vector<glm::vec3> const& in)
{
	auto const PI = 3.14159265358979323846f;

	auto const normal_to_spherical_normal = [&] (glm::vec3 n)
	{
		glm::vec2 ret = glm::vec2(glm::atan(n.y, n.x) / PI, n.z);
		return (ret + 1.f) * 0.5f;
	};

	auto const spherical_normal_to_normal = [&] (glm::vec2 sn)
	{
		auto angles = (sn * 2.f) - 1.f;
		angles.x *= PI;

		auto const sc_theta = glm::vec2{ glm::sin(angles.x), glm::cos(angles.x) };
		auto const sc_phi = glm::sqrt(1.0f - angles.y * angles.y);

		return glm::vec3(sc_theta.y * sc_phi, sc_theta.x * sc_phi, angles.y);
	};
	
	auto const pack_bit_shift = glm::vec4{ 256.f * 256.f * 256.f, 256.f * 256.f, 256.f, 1.f };
	auto const unpack_bit_shift = 1.f / pack_bit_shift;
	auto const bit_mask = glm::vec4(0.f, glm::vec3(1.f / 256.f));

	auto const float_to_vec2 = [&] (float value)
	{
		auto res = glm::fract(value * glm::vec2{ pack_bit_shift.z, pack_bit_shift.w });
		return res - glm::vec2{ res.x, res.x } * glm::vec2{ bit_mask.x, bit_mask.y };
	};

	auto const vec2_to_float = [&] (glm::vec2 value)
	{
		return glm::dot(value, glm::vec2{ unpack_bit_shift.z, unpack_bit_shift.w });
	};

	using rgba8_t = glm::vec<4, std::uint8_t>;
	auto buffer = std::vector<rgba8_t>();
	buffer.reserve(in.size());

	for (auto const& n : in)
	{
		auto sn = normal_to_spherical_normal(n);
		auto rgba = glm::vec4(float_to_vec2(sn.x), float_to_vec2(sn.y)); // todo: should we be changing range to 0.0 to 1.0?
		buffer.push_back(rgba8_t(rgba * 255.f));
	}

	auto out = std::vector<glm::vec3>();
	out.reserve(in.size());

	for (auto const& b : buffer)
	{
		auto sn4 = glm::vec4(b) / 255.f;
		auto sn = glm::vec2{ vec2_to_float({ sn4.x, sn4.y }), vec2_to_float({ sn4.z, sn4.w }) };
		auto n = spherical_normal_to_normal(sn);
		out.push_back(glm::normalize(n));
	}

	return out;
}

// std::vector<glm::vec3> convert_normals_xy_only(std::vector<glm::vec3> const& in)
// {
// 	auto const pack_bit_shift = glm::vec4{ 256.f * 256.f * 256.f, 256.f * 256.f, 256.f, 1.f };
// 	auto const unpack_bit_shift = 1.f / pack_bit_shift;
// 	auto const bit_mask = glm::vec4(0.f, glm::vec3(1.f / 256.f));

// 	auto const float_to_vec2 = [&] (float value)
// 	{
// 		auto res = glm::fract(value * glm::vec2{ pack_bit_shift.z, pack_bit_shift.w });
// 		return res - glm::vec2{ res.x, res.x } * glm::vec2{ bit_mask.x, bit_mask.y };
// 	};

// 	auto const vec2_to_float = [&] (glm::vec2 value)
// 	{
// 		return glm::dot(value, glm::vec2{ unpack_bit_shift.z, unpack_bit_shift.w });
// 	};

// 	using rgba8_t = glm::vec<4, std::uint8_t>;
// 	auto buffer = std::vector<rgba8_t>();
// 	buffer.reserve(in.size());

// 	for (auto const& n : in)
// 	{
// 		auto rgba = glm::vec4(float_to_vec2(n.x * 0.5f + 0.5f), float_to_vec2(n.y * 0.5f + 0.5f));
// 		buffer.push_back(rgba8_t(rgba * 255.f));
// 	}

// 	auto out = std::vector<glm::vec3>();
// 	out.reserve(in.size());

// 	for (auto const& b : buffer)
// 	{
// 		auto rgba = glm::vec4(b) / 255.f;
// 		auto x = vec2_to_float({ rgba.x, rgba.y }) * 2.0 - 1.0;
// 		auto y = vec2_to_float({ rgba.z, rgba.w }) * 2.0 - 1.0;
// 		auto z = glm::sqrt(1.f - glm::dot(glm::vec2{ x, y }, glm::vec2{ x, y }));
// 		auto n = glm::vec3{ x, y, z };
// 		// in view-space, so dot with view vector, flip z if it's wrong?
// 		out.push_back(n);
// 	}

// 	return out;
// }


int main()
{
	// generate:
	auto const normals = generate_normals(10000);

	// convert:
	auto const rgb8 = convert_normals_8bit(normals);
	auto const spherical = convert_normals_spherical(normals);

	std::cout << "rgb8, spherical\n";

	auto count = 0u;

	for (auto i = std::size_t{ 0 }; i != normals.size(); ++i)
	{
		auto rgb8_error = glm::length(normals[i] - rgb8[i]);
		auto spherical_error = glm::length(normals[i] - spherical[i]);

		if (rgb8_error < spherical_error)
			++count;
	}

	std::cout << "rbg8 better in: " << count << " out of: " << normals.size() << " cases.\n";

	std::cout << "done!" << std::endl;
}
