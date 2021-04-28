#pragma once

#include "bump_time.hpp"
#include "bump_physics_bucket_grid.hpp"
#include "bump_physics_collider.hpp"

#include <entt.hpp>

namespace bump
{
	
	namespace physics
	{
		
		class rigidbody;
	
		class physics_system
		{
		public:

			explicit physics_system(entt::registry& registry, high_res_duration_t update_time = high_res_duration_from_seconds(1.f / 120.f));

			void update(high_res_duration_t dt);

		private:

			entt::registry& m_registry;

			high_res_duration_t m_update_time;
			high_res_duration_t m_accumulator;

			bucket_grid m_bucket_grid_asteroids;
			bucket_grid m_bucket_grid_particles;

			struct hit_data
			{
				entt::entity a, b;
				collision_data c;
				float rv; // relative velocity
			};

			std::vector<std::pair<entt::entity, entt::entity>> m_frame_candidate_pairs;
			std::vector<hit_data> m_frame_collisions;
		};

	} // physics
	
} // bump