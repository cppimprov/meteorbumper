#include "bump_game_crosshair.hpp"

#include <Tracy.hpp>

namespace bump
{
	
	namespace game
	{
		
		crosshair::crosshair(gl::shader_program const& shader):
			m_position(0.f),
			m_size(30.f),
			m_color{ 1.f, 0.f, 0.f, 1.f },
			m_shader(shader),
			m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
			m_u_MVP(shader.get_uniform_location("u_MVP")),
			m_u_Position(shader.get_uniform_location("u_Position")),
			m_u_Size(shader.get_uniform_location("u_Size")),
			m_u_Color(shader.get_uniform_location("u_Color"))
		{
			auto vertices = { 0.f, 0.f,  1.f, 0.f,  1.f, 1.f,  0.f, 0.f,  1.f, 1.f,  0.f, 1.f, };
			m_vertex_buffer.set_data(GL_ARRAY_BUFFER, vertices.begin(), 2, 6, GL_STATIC_DRAW);
			m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertex_buffer);
		}

		void crosshair::render(gl::renderer& renderer, camera_matrices const& matrices)
		{
			ZoneScopedN("crosshair::render()");

			auto const mvp = matrices.model_view_projection_matrix(glm::mat4(1.f));

			renderer.set_blending(gl::renderer::blending::BLEND);
			renderer.set_depth_test(gl::renderer::depth_test::ALWAYS);

			renderer.set_program(m_shader);
			renderer.set_uniform_4x4f(m_u_MVP, mvp);
			renderer.set_uniform_2f(m_u_Position, m_position);
			renderer.set_uniform_2f(m_u_Size, m_size);
			renderer.set_uniform_4f(m_u_Color, m_color);
			renderer.set_vertex_array(m_vertex_array);

			renderer.draw_arrays(GL_TRIANGLES, m_vertex_buffer.get_element_count());

			renderer.clear_vertex_array();
			renderer.clear_program();
			renderer.set_depth_test(gl::renderer::depth_test::LESS);
			renderer.set_blending(gl::renderer::blending::NONE);
		}
		
	} // game
	
} // bump