#include "bump_physics_system.hpp"

#include "bump_physics_collider.hpp"
#include "bump_physics_rigidbody.hpp"

namespace bump
{
	
	namespace physics
	{
		
		physics_system::physics_system(high_res_duration_t update_time):
			m_update_time(update_time),
			m_accumulator(0) { }

		void physics_system::update(entt::registry& registry, high_res_duration_t dt)
		{
			m_accumulator += dt;

			while (m_accumulator >= m_update_time)
			{
				// check for collisions
				auto colliders = registry.view<rigidbody, collider>();

				if (!colliders.empty())
				{
					// broad phase - find collision candidate pairs
					auto candidate_pairs = std::vector<std::pair<entt::entity, entt::entity>>();

					for (auto first = colliders.begin(); first != std::prev(colliders.end()); ++first)
						for (auto second = std::next(first); second != colliders.end(); ++second)
							candidate_pairs.push_back({ *first, *second });
					
					// check that objects have at least one layer in common

					// narrow phase - get collision data
					auto collisions = std::vector<std::pair<std::pair<entt::entity, entt::entity>, collision_data>>();

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
							collisions.push_back({ pair, hit.value() });
					}

					// resolve collision
					for (auto const& collision : collisions)
					{
						auto& a = colliders.get<rigidbody>(collision.first.first);
						auto& b = colliders.get<rigidbody>(collision.first.second);
						auto const& c = collision.second;
						
						auto e = glm::min(
							colliders.get<collider>(collision.first.first).get_restitution(), 
							colliders.get<collider>(collision.first.second).get_restitution());

						resolve_impulse(a, b, c, e);
						resolve_projection(a, b, c);
					}

					// notify colliders of collision
					// todo: pass more information (what? where? when? how fast? etc.)
					for (auto const& c : collisions)
					{
						colliders.get<collider>(c.first.first).call_callback();
						colliders.get<collider>(c.first.second).call_callback();
					}
				}

				// update physics components
				auto view = registry.view<rigidbody>();
				
				for (auto id : view)
					view.get<rigidbody>(id).update(m_update_time);

				m_accumulator -= m_update_time;
			}

			// clear forces
			auto view = registry.view<rigidbody>();

			for (auto id : view)
			{
				auto& c = view.get<rigidbody>(id);
				c.clear_force();
				c.clear_torque();
			}
		}
		
	} // physics
	
} // bump