#pragma once

#include "bump_die.hpp"

#include <glm/glm.hpp>

#include <map>
#include <random>

namespace bump
{
	
	namespace random
	{

		// get a uniformly distributed random point in a ring
		// min_radius -> inner radius of ring (points are outside this)
		// max_radius -> outer radius of ring (points are inside this)
		template<class RNG>
		glm::vec2 point_in_ring_2d(RNG& rng, float min_radius, float max_radius)
		{
			die_if(max_radius < min_radius);

			auto d = std::uniform_real_distribution<float>(0.f, 1.f);
			auto angle = d(rng) * 2.f * glm::pi<float>();
			auto min_r2 = min_radius * min_radius;
			auto max_r2 = max_radius * max_radius;
			auto radius = std::sqrt(d(rng) * (max_r2 - min_r2) + min_r2);

			return { radius * std::cos(angle), radius * std::sin(angle) };
		}

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

		// get a random rgb color no more than max_offset from base_color
		// each component of the result is clamped from 0.0 to 1.0
		// todo: use hsv?
		template<class RNG>
		glm::vec3 color_offset_rgb(RNG& rng, glm::vec3 base_color, glm::vec3 max_offset)
		{
			auto const dist = std::uniform_real_distribution<float>(-1.f, 1.f);
			auto const color = glm::vec3(dist(rng), dist(rng), dist(rng));

			return glm::clamp(base_color + color * max_offset, 0.f, 1.f);
		}

		// get a random float no more than max_offset from base_scale
		template<class RNG>
		float scale(RNG& rng, float base_scale, float max_offset)
		{
			auto const dist = std::uniform_real_distribution<float>(-1.f, 1.f);
			auto const scale = base_scale + dist(rng) * max_offset;
			return scale;
		}

		// get a random type with the provided distribution
		// float values represent the upper probability limit for each type
		// e.g. { 0.3, t1 }, { 0.5, t2 }, { 1.0, t3 }
		// has a 0.3 probability of returning t1, a 0.2 probability of returning t2, and a 0.5 probability of returning t3
		// the last value in the map should always be 1.0
		template<class RNG, class Type>
		Type type_from_probability_map(RNG& rng, std::map<float, Type> const& bounds)
		{
			auto const dist = std::uniform_real_distribution<float>(0.f, 1.f);
			return bounds.lower_bound(dist(rng))->second;
		}

	} // unnamed
	
} // bump