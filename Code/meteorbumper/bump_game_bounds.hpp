#pragma once

#include <entt.hpp>

namespace bump
{
	
	namespace game
	{

		struct bounds_tag { };
		
		class bounds
		{
		public:

			explicit bounds(entt::registry& registry, float radius);

			bounds(bounds const&) = delete;
			bounds& operator=(bounds const&) = delete;
			bounds(bounds&&) = delete;
			bounds& operator=(bounds&&) = delete;

			~bounds();

		private:

			entt::registry& m_registry;
			entt::entity m_id;
		};
		
	} // game
	
} // bump