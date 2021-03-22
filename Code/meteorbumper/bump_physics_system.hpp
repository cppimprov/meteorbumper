#pragma once

#include "bump_time.hpp"

#include <entt.hpp>

namespace bump
{
	
	namespace physics
	{
		
		class physics_component;
	
		class physics_system
		{
		public:

			physics_system(high_res_duration_t update_time = 
				std::chrono::duration_cast<high_res_duration_t>(std::chrono::duration<float>(1.f / 120.f)));

			void update(entt::registry& registry, high_res_duration_t dt);

		private:

			high_res_duration_t m_update_time;
			high_res_duration_t m_accumulator;

			// todo: collision pairs / data
			// todo: force generators
		};

	} // physics
	
} // bump