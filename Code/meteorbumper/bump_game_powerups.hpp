#pragma once

#include "bump_gl.hpp"
#include "bump_game_basic_renderable.hpp"
#include "bump_time.hpp"

#include <entt.hpp>
#include <glm/glm.hpp>

namespace bump
{
	
	class camera_matrices;
	struct mbp_model;

	namespace game
	{
		
		class powerups
		{
		public:

			enum class powerup_type { RESET_SHIELDS, RESET_ARMOR, UPGRADE_LASERS };

			explicit powerups(
				entt::registry& registry,
				gl::shader_program const& shader,
				mbp_model const& shield_model,
				mbp_model const& armor_model,
				mbp_model const& lasers_model);

			void spawn(glm::vec3 position, powerup_type type);

			void update(high_res_duration_t dt);
			void render(gl::renderer& renderer, camera_matrices const& matrices);
			
			struct powerup_data
			{
				powerup_type m_type;
				high_res_duration_t m_lifetime;
				bool m_collected;
			};

		private:

			entt::registry& m_registry;

			basic_renderable m_shield_renderable;
			basic_renderable m_armor_renderable;
			basic_renderable m_lasers_renderable;

			high_res_duration_t m_max_lifetime;
			std::vector<entt::entity> m_entities;
		};
		
	} // game
	
} // bump