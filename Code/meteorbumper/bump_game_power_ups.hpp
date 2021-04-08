#pragma once

#include "bump_gl.hpp"
#include "bump_time.hpp"

#include <entt.hpp>
#include <glm/glm.hpp>

namespace bump
{
	
	class camera_matrices;

	namespace game
	{
		
		class power_ups
		{
		public:

			enum class power_up_type { RESET_SHIELDS, RESET_ARMOR, UPGRADE_LASERS };

			explicit power_ups(entt::registry& registry);

			void spawn(glm::vec3 position, power_up_type type);

			void update(high_res_duration_t dt);
			void render(gl::renderer& renderer, camera_matrices const& matrices);
			
		private:

			entt::registry& m_registry;

			// todo: graphics stuff!

			struct power_up_data
			{
				power_up_type m_type;
				high_res_duration_t m_lifetime;
				bool m_collected;
			};

			high_res_duration_t m_max_lifetime;
			std::vector<entt::entity> m_entities;
		};
		
	} // game
	
} // bump