#include "bump_physics_system.hpp"

#include "bump_physics_collider.hpp"
#include "bump_physics_rigidbody.hpp"

namespace bump
{
	
	namespace physics
	{
		
		physics_system::physics_system(entt::registry& registry, high_res_duration_t update_time):
			m_registry(registry),
			m_update_time(update_time),
			m_accumulator(0) { }

		void physics_system::update(high_res_duration_t dt)
		{
			m_accumulator += dt;

			while (m_accumulator >= m_update_time)
			{
				// check for collisions
				auto colliders = m_registry.view<rigidbody, collider>();

				if (!colliders.empty())
				{
					// broad phase - find collision candidate pairs
					auto candidate_pairs = std::vector<std::pair<entt::entity, entt::entity>>();
					candidate_pairs.reserve(colliders.size() * colliders.size());

					{
						auto const end_1 = std::prev(colliders.end());
						auto const end_2 = colliders.end();

						for (auto first = colliders.begin(); first != end_1; ++first)
							for (auto second = std::next(first); second != end_2; ++second)
								candidate_pairs.push_back({ *first, *second });
					}
					
					// narrow phase - get collision data
					struct hit_data
					{
						entt::entity a, b;
						collision_data c;
						float rv; // relative velocity
					};

					auto collisions = std::vector<hit_data>();

					for (auto const& pair : candidate_pairs)
					{
						auto const& p1 = colliders.get<rigidbody>(pair.first);
						auto const& p2 = colliders.get<rigidbody>(pair.second);
						auto const& c1 = colliders.get<collider>(pair.first);
						auto const& c2 = colliders.get<collider>(pair.second);

						// check collision layers
						if ((c1.get_collision_mask() & c2.get_collision_layer()) == 0) continue;
						if ((c2.get_collision_mask() & c1.get_collision_layer()) == 0) continue;
						
						auto hit = dispatch_find_collision(p1, c1, p2, c2);

						if (hit)
							collisions.push_back({ pair.first, pair.second, hit.value(), glm::length(p1.get_velocity() - p2.get_velocity()) });
					}

					// resolve collision
					for (auto const& hit : collisions)
					{
						auto& a = colliders.get<rigidbody>(hit.a);
						auto& b = colliders.get<rigidbody>(hit.b);
						auto const& c = hit.c;
						
						auto e = glm::min(
							colliders.get<collider>(hit.a).get_restitution(), 
							colliders.get<collider>(hit.b).get_restitution());

						resolve_impulse(a, b, c, e);
						resolve_projection(a, b, c);
					}

					// notify colliders of collision
					for (auto const& hit : collisions)
					{
						colliders.get<collider>(hit.a).call_callback(hit.b, hit.c, hit.rv);
						colliders.get<collider>(hit.b).call_callback(hit.a, hit.c, hit.rv);
					}
				}

				// update physics components
				auto view = m_registry.view<rigidbody>();
				
				for (auto id : view)
					view.get<rigidbody>(id).update(m_update_time);

				m_accumulator -= m_update_time;
			}

			// clear forces
			auto view = m_registry.view<rigidbody>();

			for (auto id : view)
			{
				auto& c = view.get<rigidbody>(id);
				c.clear_force();
				c.clear_torque();
			}
		}
		
	} // physics
	
} // bump