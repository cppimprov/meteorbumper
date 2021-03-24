#pragma once

#include "bump_camera.hpp"
#include "bump_font.hpp"
#include "bump_gl.hpp"
#include "bump_time.hpp"
#include "bump_render_text.hpp"

#include <glm/glm.hpp>

namespace bump
{
	
	namespace game
	{
		
		class press_start_text
		{
		public:

			explicit press_start_text(font::ft_context const& ft_context, font::font_asset const& font, gl::shader_program const& outline_shader, gl::shader_program const& shader);

			void render(gl::renderer& renderer, camera_matrices const& matrices, glm::vec2 window_size, high_res_duration_t dt);

		private:

			struct graphics_data
			{
				graphics_data(gl::shader_program const& shader, text_texture text);

				void render(gl::renderer& renderer, camera_matrices const& matrices, glm::vec2 position, high_res_duration_t dt);

				gl::shader_program const& m_shader;
				GLint m_in_VertexPosition;
				GLint m_u_MVP;
				GLint m_u_Position;
				GLint m_u_Size;
				GLint m_u_Time;
				GLint m_u_Color;
				GLint m_u_TextTexture;

				text_texture m_text;
				
				gl::buffer m_vertex_buffer;
				gl::vertex_array m_vertex_array;

				high_res_duration_t m_time;
			};

			graphics_data m_outline;
			graphics_data m_text;
		};
		
	} // game
	
} // bump