#include "bump_physics_system.hpp"

#include "bump_game_asteroids.hpp"
#include "bump_game_bounds.hpp"
#include "bump_game_particle_effect.hpp"
#include "bump_game_player.hpp"
#include "bump_game_powerups.hpp"
#include "bump_log.hpp"
#include "bump_physics_collider.hpp"
#include "bump_physics_rigidbody.hpp"

#include <entt.hpp>

#include <glm/glm.hpp>

#include <Tracy.hpp>

#include <array>
#include <iostream>
#include <vector>

namespace bump
{
	
	namespace physics
	{

		physics_system::physics_system(entt::registry& registry, high_res_duration_t update_time):
			m_registry(registry),
			m_update_time(update_time),
			m_accumulator(0),
			m_bucket_grid_asteroids(glm::vec3(60.f), glm::size3{ 10, 1, 10 }),
			m_bucket_grid_particles(glm::vec3(2.f), glm::size3{ 50, 4, 50 })
		{ }

		void physics_system::update(high_res_duration_t dt)
		{
			ZoneScopedN("physics_system::update()");

			m_accumulator += dt;

			while (m_accumulator >= m_update_time)
			{

				// check for collisions
				auto colliders = m_registry.view<rigidbody, collider>();

				if (!colliders.empty())
				{
					// broad phase - find collision candidate pairs
					{
						ZoneScopedN("physics_system::update() - broad phase");

						auto bounds_view = m_registry.view<rigidbody, collider, game::bounds_tag>();
						auto player_view = m_registry.view<rigidbody, collider, game::player_tag>();
						auto lasers_view = m_registry.view<rigidbody, collider, game::player_lasers::beam_segment>();
						auto asteroids_view = m_registry.view<rigidbody, collider, game::asteroid_field::asteroid_data>();
						auto particles_view = m_registry.view<rigidbody, collider, game::particle_effect::particle_data>();
						auto powerups_view = m_registry.view<rigidbody, collider, game::powerups::powerup_data>();

						m_bucket_grid_asteroids.create(asteroids_view);
						m_bucket_grid_particles.create(particles_view);

						// player -> bounds, powerups, asteroids
						m_frame_candidate_pairs.emplace_back(bounds_view.front(), player_view.front());
						for (auto id : powerups_view) m_frame_candidate_pairs.emplace_back(player_view.front(), id);
						m_bucket_grid_asteroids.get_collision_pairs(player_view, std::back_inserter(m_frame_candidate_pairs));

						// lasers -> asteroids
						m_bucket_grid_asteroids.get_collision_pairs(lasers_view, std::back_inserter(m_frame_candidate_pairs));

						// asteroids -> bounds, asteroids
						for (auto id : asteroids_view) m_frame_candidate_pairs.emplace_back(bounds_view.front(), id);
						m_bucket_grid_asteroids.get_collision_pairs(asteroids_view, std::back_inserter(m_frame_candidate_pairs));
						
						// particles -> player, asteroids, powerups
						m_bucket_grid_particles.get_collision_pairs(player_view, std::back_inserter(m_frame_candidate_pairs));
						m_bucket_grid_particles.get_collision_pairs(asteroids_view, std::back_inserter(m_frame_candidate_pairs));
						m_bucket_grid_particles.get_collision_pairs(powerups_view, std::back_inserter(m_frame_candidate_pairs));
					}

					{
						ZoneScopedN("physics_system::update() - narrow phase");

						// narrow phase - get collision data
						for (auto const& pair : m_frame_candidate_pairs)
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
								m_frame_collisions.push_back({ pair.first, pair.second, hit.value(), glm::length(p1.get_velocity() - p2.get_velocity()) });
						}
						
						m_frame_candidate_pairs.clear();
					}

					{
						ZoneScopedN("physics_system::update() - resolve");

						// resolve collision
						for (auto const& hit : m_frame_collisions)
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
					}

					{
						ZoneScopedN("physics_system::update() - notify");

						// notify colliders of collision
						for (auto const& hit : m_frame_collisions)
						{
							colliders.get<collider>(hit.a).call_callback(hit.b, hit.c, hit.rv);
							colliders.get<collider>(hit.b).call_callback(hit.a, hit.c, hit.rv);
						}

						m_frame_collisions.clear();
					}
				}

				{
					ZoneScopedN("physics_system::update() - update rigidbodies");

					// update physics components
					auto view = m_registry.view<rigidbody>();
					
					for (auto id : view)
						view.get<rigidbody>(id).update(m_update_time);
				}

				m_accumulator -= m_update_time;
			}

			{
				ZoneScopedN("physics_system::update() - clear forces");

				// clear forces
				auto view = m_registry.view<rigidbody>();

				for (auto id : view)
				{
					auto& c = view.get<rigidbody>(id);
					c.clear_force();
					c.clear_torque();
				}
			}
		}
		
	} // physics
	
} // bump