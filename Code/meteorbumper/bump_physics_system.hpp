#pragma once

#include "bump_time.hpp"

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

			// todo: collision pairs / data
			// todo: force generators
		};

	} // physics
	
} // bump