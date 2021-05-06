#pragma once

#include "bump_gl.hpp"
#include "bump_game_basic_renderable.hpp"

#include <entt.hpp>
#include <glm/glm.hpp>

#include <vector>

namespace bump
{

	struct mbp_model;
	class camera_matrices;
	
	namespace game
	{

		struct bounds_tag { };
		
		class bounds
		{
		public:

			explicit bounds(entt::registry& registry, float radius, gl::shader_program const& bouy_shader, mbp_model const& bouy_model);

			void render_scene(gl::renderer& renderer, camera_matrices const& matrices);

			~bounds();

		private:

			entt::registry& m_registry;
			entt::entity m_id;

			struct bouy_light_tag {}; // tag type for bouy light entites
			
			basic_renderable_instanced m_bouy;
			std::vector<glm::mat4> m_bouy_transforms;
		};
		
	} // game
	
} // bump