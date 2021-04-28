#include "bump_game_fps_counter.hpp"

#include <Tracy.hpp>

namespace bump
{
	
	namespace game
	{
		
		fps_counter::fps_counter(font::ft_context const& ft_context, font::font_asset const& font, gl::shader_program const& shader):
			m_shader(shader),
			m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
			m_u_MVP(shader.get_uniform_location("u_MVP")),
			m_u_Position(shader.get_uniform_location("u_Position")),
			m_u_Size(shader.get_uniform_location("u_Size")),
			m_u_Color(shader.get_uniform_location("u_Color")),
			m_u_CharOrigin(shader.get_uniform_location("u_CharOrigin")),
			m_u_CharSize(shader.get_uniform_location("u_CharSize")),
			m_u_CharTexture(shader.get_uniform_location("u_CharTexture")),
			m_position(0.f),
			m_color(1.f),
			m_charmap_texture(render_charmap(ft_context, font, "0123456789.")),
			m_fps_string()
		{
			auto vertices = { 0.f, 0.f,  1.f, 0.f,  1.f, 1.f,  0.f, 0.f,  1.f, 1.f,  0.f, 1.f };
			m_vertices.set_data(GL_ARRAY_BUFFER, vertices.begin(), 2, 6, GL_STATIC_DRAW);
			m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertices);
		}

		glm::vec2 fps_counter::get_size() const
		{
			auto pen_x = m_charmap_texture.m_pos.x;

			for (auto ch : m_fps_string)
				pen_x += m_charmap_texture.m_char_positions.at(ch).m_size.x;
			
			auto pen_y = m_charmap_texture.m_pos.y + m_charmap_texture.m_texture.get_size().y;
			
			return { pen_x, pen_y };
		}

		void fps_counter::update(high_res_duration_t dt)
		{
			auto const frame_time = high_res_duration_to_seconds(dt);
			auto const fps = (frame_time == 0.f ? 0.f : 1.f / frame_time);

			m_fps_string = std::to_string(static_cast<int>(fps));
		}

		void fps_counter::render(gl::renderer& renderer, camera_matrices const& matrices)
		{
			ZoneScopedN("fps_counter::render()");

			auto mvp = matrices.model_view_projection_matrix(glm::mat4(1.f));

			renderer.set_blending(gl::renderer::blending::BLEND);
			renderer.set_program(m_shader);

			renderer.set_uniform_4x4f(m_u_MVP, mvp);
			renderer.set_uniform_3f(m_u_Color, m_color);
			renderer.set_uniform_1i(m_u_CharTexture, 0);
			renderer.set_texture_2d(0, m_charmap_texture.m_texture);

			renderer.set_vertex_array(m_vertex_array);

			auto pen = m_charmap_texture.m_pos;

			for (auto ch : m_fps_string)
			{
				auto [origin, size] = m_charmap_texture.m_char_positions.at(ch);

				renderer.set_uniform_2f(m_u_Position, m_position + glm::vec2(pen));
				renderer.set_uniform_2f(m_u_Size, glm::vec2(size));
				renderer.set_uniform_2f(m_u_CharOrigin, glm::vec2(origin));
				renderer.set_uniform_2f(m_u_CharSize, glm::vec2(size));

				renderer.draw_arrays(GL_TRIANGLES, m_vertices.get_element_count());

				pen.x += size.x;
			}
			
			renderer.clear_texture_2d(0);
			renderer.clear_vertex_array();
			renderer.clear_program();
			renderer.set_blending(gl::renderer::blending::NONE);
		}
		
	} // game
	
} // bump
