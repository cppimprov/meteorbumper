#include "bump_game_basic_renderable.hpp"

#include "bump_camera.hpp"
#include "bump_mbp_model.hpp"

#include <Tracy.hpp>

namespace bump
{
	
	namespace game
	{
		
		basic_renderable::basic_renderable(gl::shader_program const& shader, mbp_model const& model):
			m_shader(&shader),
			m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
			m_u_MVP(shader.get_uniform_location("u_MVP")),
			m_u_Color(shader.get_uniform_location("u_Color")),
			m_transform(1.f)
		{
			for (auto const& submesh : model.m_submeshes)
			{
				auto s = submesh_data();

				s.m_color = submesh.m_material.m_base_color;

				s.m_vertices.set_data(GL_ARRAY_BUFFER, submesh.m_mesh.m_vertices.data(), 3, submesh.m_mesh.m_vertices.size() / 3, GL_STATIC_DRAW);
				s.m_vertex_array.set_array_buffer(m_in_VertexPosition, s.m_vertices);

				s.m_indices.set_data(GL_ELEMENT_ARRAY_BUFFER, submesh.m_mesh.m_indices.data(), 1, submesh.m_mesh.m_indices.size(), GL_STATIC_DRAW);
				s.m_vertex_array.set_index_buffer(s.m_indices);

				m_submeshes.push_back(std::move(s));
			}
		}

		void basic_renderable::render(gl::renderer& renderer, camera_matrices const& matrices)
		{
			ZoneScopedN("basic_renderable::render()");

			auto mvp = matrices.model_view_projection_matrix(m_transform);

			renderer.set_program(*m_shader);

			for (auto const& submesh : m_submeshes)
			{
				renderer.set_uniform_4x4f(m_u_MVP, mvp);
				renderer.set_uniform_3f(m_u_Color, submesh.m_color);

				renderer.set_vertex_array(submesh.m_vertex_array);

				renderer.draw_indexed(GL_TRIANGLES, submesh.m_indices.get_element_count(), submesh.m_indices.get_component_type());

				renderer.clear_vertex_array();
			}

			renderer.clear_program();
		}
		
	} // game
	
} // bump