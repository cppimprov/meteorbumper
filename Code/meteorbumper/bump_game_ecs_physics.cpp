#include "bump_game_ecs_physics.hpp"

#include "bump_die.hpp"

#include <optional>
#include <variant>

namespace bump
{	

	namespace game::ecs
	{

		namespace
		{

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

					return {};
				}

				std::optional<collision_data> operator()(cuboid_shape const& s1, cuboid_shape const& s2) const
				{
					(void)s1;
					(void)s2;
					return {};
				}

				std::optional<collision_data> operator()(cuboid_shape const& s1, sphere_shape const& s2) const
				{
					(void)s1;
					(void)s2;
					return {};
				}
				
				std::optional<collision_data> operator()(sphere_shape const& s1, cuboid_shape const& s2) const
				{
					(void)s1;
					(void)s2;
					return {};
				}

			};

			std::optional<collision_data> dispatch_find_collision(physics_component const& p1, collision_component const& c1, physics_component const& p2, collision_component const& c2)
			{
				return std::visit(find_collision(p1, p2), c1.get_shape(), c2.get_shape());
			}

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

			// todo: tidy this up, split up terms better (one struct for each object)
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

				if (!glm::epsilonEqual(i.ima, 0.f, glm::epsilon<float>()))
					dva.m_angular_velocity += i.iia * glm::cross(i.ra, nj);
				
				dvb.m_linear_velocity += i.imb * nj;

				if (!glm::epsilonEqual(i.imb, 0.f, glm::epsilon<float>()))
					dvb.m_angular_velocity += i.iib * glm::cross(i.rb, nj);
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

				// todo: tangent impulse!

				// update velocity
				p1.set_velocity(i.va + dva.m_linear_velocity);
				p1.set_angular_velocity(i.ava + dva.m_angular_velocity);

				p2.set_velocity(i.vb + dvb.m_linear_velocity);
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

		} // unnamed

		physics_system::physics_system(high_res_duration_t update_time):
			m_update_time(update_time),
			m_accumulator(0) { }

		void physics_system::update(entt::registry& registry, high_res_duration_t dt)
		{
			m_accumulator += dt;

			// note: if we start destroying physics objects inside the physics update loop (e.g. on collision)
			// we will need to update this view inside the accumulator!
			auto view = registry.view<physics_component>();

			while (m_accumulator >= m_update_time)
			{
				auto colliders = registry.view<physics_component, collision_component>();

				if (!colliders.empty())
				{
					// broad phase - find collision candidate pairs
					auto candidate_pairs = std::vector<std::pair<entt::entity, entt::entity>>();

					for (auto first = colliders.begin(); first != std::prev(colliders.end()); ++first)
						for (auto second = std::next(first); second != colliders.end(); ++second)
							candidate_pairs.push_back({ *first, *second });

					// narrow phase - get collision data
					auto collisions = std::vector<std::pair<std::pair<entt::entity, entt::entity>, collision_data>>();

					for (auto const& pair : candidate_pairs)
					{
						auto hit = dispatch_find_collision(
							colliders.get<physics_component>(pair.first),
							colliders.get<collision_component>(pair.first),
							colliders.get<physics_component>(pair.second),
							colliders.get<collision_component>(pair.second));

						if (hit)
							collisions.push_back({ pair, hit.value() });
					}

					// resolve collision
					for (auto const& c : collisions)
					{
						auto& p1 = colliders.get<physics_component>(c.first.first);
						auto& c1 = colliders.get<collision_component>(c.first.first);
						auto& p2 = colliders.get<physics_component>(c.first.second);
						auto& c2 = colliders.get<collision_component>(c.first.second);
						auto const& data = c.second;

						resolve_impulse(p1, p2, c1, c2, data);
						resolve_projection(p1, p2, data);
					}
				}

				for (auto id : view)
					view.get<physics_component>(id).update(m_update_time);

				m_accumulator -= m_update_time;
			}

			for (auto id : view)
			{
				auto& c = view.get<physics_component>(id);
				c.clear_force();
				c.clear_torque();
			}
		}
		
		physics_component::physics_component():
			// mass
			m_inverse_mass(1.f),
			m_local_inertia_tensor(make_sphere_inertia_tensor(1.f, 1.f)),
			// movement
			m_position(0.f),
			m_orientation(),
			m_velocity(0.f),
			m_angular_velocity(0.f),
			m_linear_damping(0.99999f),
			m_angular_damping(0.99999f),
			// forces
			m_force(0.f),
			m_torque(0.f)
			{ }
		
		void physics_component::update(high_res_duration_t dt)
		{
			auto dt_s = std::chrono::duration_cast<std::chrono::duration<float>>(dt).count();

			// integrate for position
			{
				auto acceleration = m_inverse_mass * m_force;

				// semi-implicit euler
				m_velocity += acceleration * dt_s;
				m_position += m_velocity * dt_s;
				m_velocity *= m_linear_damping;
			}

			// integrate for orientation
			{
				auto acceleration = get_inverse_inertia_tensor() * m_torque;

				m_angular_velocity += acceleration * dt_s;
				m_orientation += m_orientation * glm::quat(0.f, m_angular_velocity) * (dt_s / 2.f);
				m_orientation = glm::normalize(m_orientation);
				m_angular_velocity *= m_angular_damping;
			}
		}

		collision_component::collision_component():
			m_shape(sphere_shape{ 1.f }),
			m_restitution(0.5f)
			{ }
		
	} // game::ecs
	
} // bump
