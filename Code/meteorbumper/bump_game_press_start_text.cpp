#include "bump_game_press_start_text.hpp"

namespace bump
{
	
	namespace game
	{
		
		press_start_text::press_start_text(font::ft_context const& ft_context, font::font_asset const& font, gl::shader_program const& outline_shader, gl::shader_program const& shader):
			m_outline(outline_shader, render_text_outline_to_gl_texture(ft_context, font, "Press any key to start!", 3.0)),
			m_text(shader, render_text_to_gl_texture(ft_context, font, "Press any key to start!"))
			{ }

		void press_start_text::render(gl::renderer& renderer, camera_matrices const& matrices, glm::vec2 window_size, high_res_duration_t dt)
		{
			renderer.set_blending(gl::renderer::blending::BLEND);
			renderer.set_depth_test(gl::renderer::depth_test::ALWAYS);

			{
				auto size = glm::vec2(m_outline.m_text.m_texture.get_size());

				auto outline_offset = m_outline.m_text.m_pos - m_text.m_text.m_pos;

				auto pos = glm::round(glm::vec2{
					(window_size.x - m_text.m_text.m_texture.get_size().x) / 2 + outline_offset.x, // center
					window_size.y / 8.f + outline_offset.y, // offset from bottom
				});

				m_outline.render(renderer, matrices, pos, dt);
			}
			
			{
				auto size = glm::vec2(m_text.m_text.m_texture.get_size());

				auto pos = glm::round(glm::vec2{
					(window_size.x - size.x) / 2, // center
					window_size.y / 8.f, // offset from bottom
				});

				m_text.render(renderer, matrices, pos, dt);
			}

			renderer.set_depth_test(gl::renderer::depth_test::LESS);
			renderer.set_blending(gl::renderer::blending::NONE);
		}

		press_start_text::graphics_data::graphics_data(gl::shader_program const& shader, text_texture text):
			m_shader(shader),
			m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
			m_u_MVP(shader.get_uniform_location("u_MVP")),
			m_u_Position(shader.get_uniform_location("u_Position")),
			m_u_Size(shader.get_uniform_location("u_Size")),
			m_u_Time(shader.get_uniform_location("u_Time")),
			m_u_Color(shader.get_uniform_location("u_Color")),
			m_u_TextTexture(shader.get_uniform_location("u_TextTexture")),
			m_text(std::move(text)),
			m_time(0)
		{
			auto vertices = { 0.f, 0.f,  1.f, 0.f,  1.f, 1.f,  0.f, 0.f,  1.f, 1.f,  0.f, 1.f, };
			m_vertex_buffer.set_data(GL_ARRAY_BUFFER, vertices.begin(), 2, 6, GL_STATIC_DRAW);

			m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertex_buffer);
		}

		void press_start_text::graphics_data::render(gl::renderer& renderer, camera_matrices const& matrices, glm::vec2 position, high_res_duration_t dt)
		{
			m_time += dt;

			auto mvp = matrices.model_view_projection_matrix(glm::mat4(1.f));

			renderer.set_program(m_shader);

			renderer.set_uniform_4x4f(m_u_MVP, mvp);
			renderer.set_uniform_1i(m_u_TextTexture, 0);

			renderer.set_vertex_array(m_vertex_array);
		
			renderer.set_uniform_2f(m_u_Position, position);
			renderer.set_uniform_2f(m_u_Size, glm::vec2(m_text.m_texture.get_size()));
			renderer.set_uniform_1f(m_u_Time, std::chrono::duration_cast<std::chrono::duration<float>>(m_time).count());
			renderer.set_uniform_3f(m_u_Color, glm::vec3{ 0.2f, 0.2f, 0.2f });
			renderer.set_texture_2d(0, m_text.m_texture);

			renderer.draw_arrays(GL_TRIANGLES, m_vertex_buffer.get_element_count());

			renderer.clear_vertex_array();
			renderer.clear_texture_2d(0);
			renderer.clear_program();
		}
		
	} // game
	
} // bump