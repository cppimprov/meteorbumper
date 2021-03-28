#pragma once

#include "bump_camera.hpp"
#include "bump_font.hpp"
#include "bump_gl.hpp"
#include "bump_render_text.hpp"
#include "bump_time.hpp"

#include <glm/glm.hpp>

namespace bump
{
	
	namespace game
	{
		
		class fps_counter
		{
		public:

			explicit fps_counter(font::ft_context const& ft_context, font::font_asset const& font, gl::shader_program const& shader);

			void set_position(glm::vec2 position) { m_position = position; }
			glm::vec2 get_position() const { return m_position; }

			void set_color(glm::vec3 color) { m_color = color; }
			glm::vec3 get_color() const { return m_color; }

			glm::vec2 get_size() const;

			void update(high_res_duration_t dt);
			void render(gl::renderer& renderer, camera_matrices const& matrices);

		private:

			gl::shader_program const& m_shader;
			GLint m_in_VertexPosition;
			GLint m_u_MVP;
			GLint m_u_Position;
			GLint m_u_Size;
			GLint m_u_Color;
			GLint m_u_CharOrigin;
			GLint m_u_CharSize;
			GLint m_u_CharTexture;

			gl::buffer m_vertices;
			gl::vertex_array m_vertex_array;

			glm::vec2 m_position;
			glm::vec3 m_color;

			charmap_texture m_charmap_texture;
			std::string m_fps_string;
		};
		
	} // game
	
} // bump