#include "bump_physics_system.hpp"

#include "bump_physics_collision.hpp"
#include "bump_physics_component.hpp"

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
		
	} // physics
	
} // bump