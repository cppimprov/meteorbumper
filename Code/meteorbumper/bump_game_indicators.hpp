#pragma once

#include "bump_gl.hpp"

#include <entt.hpp>

namespace bump
{

	class camera_matrices;
	
	namespace game
	{
		
		class indicators
		{
		public:

			explicit indicators(entt::registry& registry, gl::shader_program const& shader);

			void set_player_position(glm::vec3 position) { m_player_position = position; }

			void render(gl::renderer& renderer, glm::vec2 window_size, camera_matrices const& screen_matrices, camera_matrices const& ui_matrices);

		private:

			entt::registry& m_registry;

			gl::shader_program const& m_shader;
			GLint m_in_VertexPosition;
			GLint m_in_Direction;
			GLint m_in_Color;
			GLint m_u_MVP;
			GLint m_u_Size;

			gl::buffer m_buffer_positions;
			gl::buffer m_buffer_directions;
			gl::buffer m_buffer_colors;
			gl::vertex_array m_vertex_array;

			std::vector<glm::vec2> m_frame_positions;
			std::vector<glm::vec2> m_frame_directions;
			std::vector<glm::vec3> m_frame_colors;

			glm::vec3 m_player_position;
		};
		
	} // game
	
} // bump