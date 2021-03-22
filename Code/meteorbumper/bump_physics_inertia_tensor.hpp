#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace bump
{
	
	namespace physics
	{
		
		inline glm::mat3 make_sphere_inertia_tensor(float mass, float radius)
		{
			auto s = (2.f * mass * (radius * radius)) / 5.f;
			return glm::mat3(s);
		}

		inline glm::mat3 make_cuboid_inertia_tensor(float mass, glm::vec3 size)
		{
			auto m = mass / 12.f;
			auto s = size * size;
			return glm::mat3(glm::scale(glm::vec3{ m * (s.y + s.z), m * (s.x + s.z), m * (s.x + s.y) }));
		}

		inline glm::mat3 make_cylinder_inertia_tensor(float mass, float radius, float height)
		{
			auto v1 = (mass / 12.f) * (height * height) + (mass / 4.f) * (radius * radius);
			auto v2 = (mass / 2.f) * (radius * radius);

			return glm::mat3(glm::scale(glm::vec3{ v1, v2, v1 }));
		}
		
	} // physics
	
} // bump
