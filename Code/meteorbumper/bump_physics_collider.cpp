#include "bump_physics_collider.hpp"

#include "bump_physics_rigidbody.hpp"

#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <limits>
#include <optional>

namespace bump
{
	
	namespace physics
	{
		
		
		namespace
		{

			std::optional<collision_data> flip_collision(std::optional<collision_data> hit)
			{
				if (hit)
				{
					hit.value().m_normal = -hit.value().m_normal;
					hit.value().m_point += hit.value().m_normal * hit.value().m_penetration;
				}

				return hit;
			}

			struct find_collision
			{
				find_collision(rigidbody const& p1, rigidbody const& p2):
					p1(&p1), p2(&p2) { }
				
				rigidbody const* p1;
				rigidbody const* p2;

				std::optional<collision_data> operator()(sphere_shape const& s1, sphere_shape const& s2) const
				{
					auto offset = p2->get_position() - p1->get_position();
					auto distance = glm::length(offset);

					if (distance == 0.f)
						return { };

					if (distance < s1.m_radius + s2.m_radius)
					{
						auto penetration = (s1.m_radius + s2.m_radius) - distance;
						auto normal = glm::normalize(offset);
						auto point = p1->get_position() + normal * (s1.m_radius - penetration);

						return collision_data{ point, normal, penetration };
					}

					return { };
				}

				std::optional<collision_data> operator()(inverse_sphere_shape const& s1, sphere_shape const& s2) const
				{
					auto offset = p2->get_position() - p1->get_position();
					auto distance = glm::length(offset);

					if (distance > s1.m_radius - s2.m_radius)
					{
						auto penetration = (distance + s2.m_radius) - s1.m_radius;
						auto normal = -glm::normalize(offset);
						auto point = p1->get_position() + -normal * (s1.m_radius + penetration);

						return collision_data{ point, normal, penetration };
					}

					return { };
				}

				std::optional<collision_data> operator()(sphere_shape const& s1, inverse_sphere_shape const& s2) const
				{
					return flip_collision(find_collision(*p2, *p1)(s2, s1));
				}

				std::optional<collision_data> operator()(inverse_sphere_shape const& s1, inverse_sphere_shape const& s2) const
				{
					(void)s1;
					(void)s2;
					return { }; // todo: implement me!
				}

				// std::optional<collision_data> operator()(sphere_shape const& s1, plane_shape const& s2) const
				// {
				// 	auto distance = glm::dot(p1.get_position(), s2.m_normal) + s2.m_distance;

				// 	if (distance < s1.m_radius)
				// 	{
				// 		auto penetration = s1.m_radius - distance;
				// 		auto normal = -s2.m_normal; // from s1 to s2
				// 		auto point = p1.get_position() - s2.m_normal * distance;
				// 		return collision_data{ point, normal, penetration };
				// 	}

				// 	return { };
				// }
			};
			
		} // unnamed

		std::optional<collision_data> dispatch_find_collision(rigidbody const& p1, collider const& c1, rigidbody const& p2, collider const& c2)
		{
			return std::visit(find_collision(p1, p2), c1.get_shape(), c2.get_shape());
		}

		void resolve_impulse(rigidbody& a, rigidbody& b, collision_data const& c, float e)
		{
			if (a.has_infinite_mass() && b.has_infinite_mass())
				return; // both objects have infinite mass: nothing to do!
			
			auto const ra = c.m_point - a.get_position();
			auto const rb = c.m_point - b.get_position();

			auto const vab = 
				(a.get_velocity() + glm::cross(a.get_angular_velocity(), ra)) -
				(b.get_velocity() + glm::cross(b.get_angular_velocity(), rb));

			if (glm::dot(vab, c.m_normal) < 0.f)
				return; // objects are moving away from each other
			
			auto const bottom =  (a.get_inverse_mass() + b.get_inverse_mass()) + 
				glm::dot(c.m_normal,
					glm::cross(a.get_inverse_inertia_tensor() * glm::cross(ra, c.m_normal), ra) +
					glm::cross(b.get_inverse_inertia_tensor() * glm::cross(rb, c.m_normal), rb));
			
			auto const top = -(1.f + e) * glm::dot(vab, c.m_normal);

			auto const j = (top / bottom);
			auto const nj = c.m_normal * j;

			a.add_impulse_at_point(nj, ra);
			b.add_impulse_at_point(-nj, rb);
		}

		void resolve_projection(rigidbody& a, rigidbody& b, collision_data const& c)
		{
			auto const slop = 1.01f;
			auto const distance = c.m_penetration * slop;

			auto total_inv_mass = a.get_inverse_mass() + b.get_inverse_mass();
			auto factor = glm::clamp(a.get_inverse_mass() / total_inv_mass, 0.f, 1.f);

			a.set_position(a.get_position() + -c.m_normal * distance * factor * a.get_linear_factor());
			b.set_position(b.get_position() +  c.m_normal * distance * (1.f - factor) * b.get_linear_factor());
		}

		collider::collider():
			m_shape(sphere_shape{ 1.f }),
			m_restitution(0.5f),
			m_layer(std::numeric_limits<std::uint32_t>::max()),
			m_layer_mask(std::numeric_limits<std::uint32_t>::max())
			{ }
		
	} // physics
	
} // bump