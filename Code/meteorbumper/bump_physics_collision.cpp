#include "bump_physics_collision.hpp"

#include "bump_physics_component.hpp"

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
				find_collision(physics_component const& p1, physics_component const& p2):
					p1(p1), p2(p2) { }
				
				physics_component const& p1;
				physics_component const& p2;

				std::optional<collision_data> operator()(sphere_shape const& s1, sphere_shape const& s2) const
				{
					auto offset = p2.get_position() - p1.get_position();
					auto distance = glm::length(offset);

					if (distance == 0.f)
						return {};

					if (distance < s1.m_radius + s2.m_radius)
					{
						auto penetration = (s1.m_radius + s2.m_radius) - distance;
						auto normal = glm::normalize(offset);
						auto point = p1.get_position() + normal * (s1.m_radius - penetration);

						return collision_data{ point, normal, penetration };
					}

					return { };
				}

				std::optional<collision_data> operator()(cuboid_shape const& s1, cuboid_shape const& s2) const
				{
					(void)s1;
					(void)s2;
					return {}; // todo: implement me!
				}

				std::optional<collision_data> operator()(cuboid_shape const& s1, sphere_shape const& s2) const
				{
					(void)s1;
					(void)s2;
					return {}; // todo: implement me!
				}
				
				std::optional<collision_data> operator()(sphere_shape const& s1, cuboid_shape const& s2) const
				{
					(void)s1;
					(void)s2;
					return {}; // todo: implement me!
				}

				std::optional<collision_data> operator()(plane_shape const&, plane_shape const&) const
				{
					return { }; // no collision between planes!
				}

				std::optional<collision_data> operator()(plane_shape const& s1, sphere_shape const& s2) const
				{
					return flip_collision(operator()(s2, s1));
				}
				
				std::optional<collision_data> operator()(sphere_shape const& s1, plane_shape const& s2) const
				{
					auto distance = glm::dot(p1.get_position(), s2.m_normal) + s2.m_distance;

					if (distance < s1.m_radius)
					{
						auto penetration = s1.m_radius - distance;
						auto normal = -s2.m_normal; // from s1 to s2
						auto point = p1.get_position() - s2.m_normal * distance;
						return collision_data{ point, normal, penetration };
					}

					return { };
				}

				std::optional<collision_data> operator()(plane_shape const& s1, cuboid_shape const& s2) const
				{
					(void)s1;
					(void)s2;
					return { }; // todo: implement me!
				}
				
				std::optional<collision_data> operator()(cuboid_shape const& s1, plane_shape const& s2) const
				{
					(void)s1;
					(void)s2;
					return { }; // todo: implement me!
				}
			};
			
			struct impulse_terms
			{
				// collision
				glm::vec3 p, n;

				// rigid bodies
				glm::vec3 pa, pb, va, vb, ava, avb;
				float ima, imb;
				glm::mat3 iia, iib;
				
				// variables
				glm::vec3 ra, rb, vpa, vpb;
				glm::vec3 vab;
				float e;
			};

			struct delta_velocity
			{
				glm::vec3 m_linear_velocity = glm::vec3(0.f);
				glm::vec3 m_angular_velocity = glm::vec3(0.f);
			};

			void calculate_impulse(impulse_terms const& i, delta_velocity& dva, delta_velocity& dvb)
			{
				auto const bottom =  (i.ima + i.imb) + 
					glm::dot(i.n,
						glm::cross(i.iia * glm::cross(i.ra, i.n), i.ra) +
							glm::cross(i.iib * glm::cross(i.rb, i.n), i.rb));
				
				auto const top = -(1.f + i.e) * glm::dot(i.vab, i.n);

				auto const j = (top / bottom);
				auto const nj = i.n * j;

				dva.m_linear_velocity += i.ima * nj;
				dvb.m_linear_velocity -= i.imb * nj;

				if (!glm::epsilonEqual(i.ima, 0.f, glm::epsilon<float>()))
					dva.m_angular_velocity += i.iia * glm::cross(i.ra, nj);
				
				if (!glm::epsilonEqual(i.imb, 0.f, glm::epsilon<float>()))
					dvb.m_angular_velocity -= i.iib * glm::cross(i.rb, nj);
			}

		} // unnamed

		std::optional<collision_data> dispatch_find_collision(physics_component const& p1, collision_component const& c1, physics_component const& p2, collision_component const& c2)
		{
			return std::visit(find_collision(p1, p2), c1.get_shape(), c2.get_shape());
		}

		void resolve_impulse(physics_component& p1, physics_component& p2, collision_component const& c1, collision_component const& c2, collision_data const& data)
		{
			if (p1.has_infinite_mass() && p2.has_infinite_mass())
				return; // both objects have infinite mass: nothing to do!
			
			auto i = impulse_terms();
			i.p = data.m_point;
			i.n = data.m_normal;
			i.pa = p1.get_position();
			i.pb = p2.get_position();
			i.va = p1.get_velocity();
			i.vb = p2.get_velocity();
			i.ava = p1.get_angular_velocity();
			i.avb = p2.get_angular_velocity();
			i.ima = p1.get_inverse_mass();
			i.imb = p2.get_inverse_mass();
			i.iia = p1.get_inverse_inertia_tensor();
			i.iib = p2.get_inverse_inertia_tensor();
			i.ra = i.p - i.pa;
			i.rb = i.p - i.pb;
			i.vpa = i.va + glm::cross(i.ava, i.ra);
			i.vpb = i.vb + glm::cross(i.avb, i.rb);
			i.vab = i.vpa - i.vpb;
			i.e = glm::min(c1.get_restitution(), c2.get_restitution());

			if (glm::dot(i.vab, i.n) < 0.f)
				return; // objects are moving away from each other

			auto dva = delta_velocity();
			auto dvb = delta_velocity();
			
			calculate_impulse(i, dva, dvb); // calculate normal impulse

			// todo: can we turn tangent impulse off for specific objects?
			// todo: friction parameters?

			// auto t = i.vab - glm::dot(i.vab, i.n) * i.n;
			// if (!glm::epsilonEqual(glm::length(t), 0.f, glm::epsilon<float>()))
			// {
			// 	i.n = glm::normalize(t);
			// 	i.e = 0.f;

			// 	calculate_impulse(i, dva, dvb); // calculate tangent impulse 
			// }

			// update velocity
			p1.set_velocity(i.va + dva.m_linear_velocity);
			p2.set_velocity(i.vb + dvb.m_linear_velocity);
			p1.set_angular_velocity(i.ava + dva.m_angular_velocity);
			p2.set_angular_velocity(i.avb + dvb.m_angular_velocity);
		}

		void resolve_projection(physics_component& p1, physics_component& p2, collision_data const& data)
		{
			auto const slop = 1.01f;
			auto const distance = data.m_penetration * slop;

			auto total_inv_mass = p1.get_inverse_mass() + p2.get_inverse_mass();
			auto factor = glm::clamp(p1.get_inverse_mass() / total_inv_mass, 0.f, 1.f);

			p1.set_position(p1.get_position() + -data.m_normal * distance * factor);
			p2.set_position(p2.get_position() +  data.m_normal * distance * (1.f - factor));
		}

		collision_component::collision_component():
			m_shape(sphere_shape{ 1.f }),
			m_restitution(0.5f)
			{ }
		
	} // physics
	
} // bump