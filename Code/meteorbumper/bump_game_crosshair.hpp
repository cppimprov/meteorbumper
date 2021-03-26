#pragma once

#include "bump_camera.hpp"
#include "bump_gl.hpp"

#include <glm/glm.hpp>

namespace bump
{
	
	namespace game
	{
		
		class crosshair
		{
		public:

			explicit crosshair(gl::shader_program const& shader);

			glm::vec2 m_position;
			glm::vec2 m_size;
			glm::vec4 m_color;

			void render(gl::renderer& renderer, camera_matrices const& matrices);

		private:

			gl::shader_program const& m_shader;
			GLint m_in_VertexPosition;
			GLint m_u_MVP;
			GLint m_u_Position;
			GLint m_u_Size;
			GLint m_u_Color;

			gl::buffer m_vertex_buffer;
			gl::vertex_array m_vertex_array;
		};

	} // game
	
} // bump