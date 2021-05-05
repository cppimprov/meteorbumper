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
			m_in_VertexNormal(shader.get_attribute_location("in_VertexNormal")),
			m_u_MVP(shader.get_uniform_location("u_MVP")),
			m_u_NormalMatrix(shader.get_uniform_location("u_NormalMatrix")),
			m_u_Color(shader.get_uniform_location("u_Color")),
			m_u_Metallic(shader.get_uniform_location("u_Metallic")),
			m_u_Roughness(shader.get_uniform_location("u_Roughness")),
			m_u_Emissive(shader.get_uniform_location("u_Emissive")),
			m_transform(1.f)
		{
			for (auto const& submesh : model.m_submeshes)
			{
				auto s = submesh_data();

				s.m_metallic = submesh.m_material.m_metallic;
				s.m_roughness = submesh.m_material.m_roughness;
				
				if (submesh.m_material.m_emissive_color != glm::vec3(0.0))
				{
					s.m_color = submesh.m_material.m_emissive_color; // replace base color with emissive
					s.m_emissive = 1.f; // set flag
				}
				else
				{
					s.m_color = submesh.m_material.m_base_color; // use standard base color
					s.m_emissive = 0.f; // clear flag
				}

				die_if(submesh.m_mesh.m_vertices.empty());
				s.m_vertices.set_data(GL_ARRAY_BUFFER, submesh.m_mesh.m_vertices.data(), 3, submesh.m_mesh.m_vertices.size() / 3, GL_STATIC_DRAW);
				s.m_vertex_array.set_array_buffer(m_in_VertexPosition, s.m_vertices);

				die_if(submesh.m_mesh.m_normals.empty());
				s.m_normals.set_data(GL_ARRAY_BUFFER, submesh.m_mesh.m_normals.data(), 3, submesh.m_mesh.m_normals.size() / 3, GL_STATIC_DRAW);
				s.m_vertex_array.set_array_buffer(m_in_VertexNormal, s.m_normals);

				die_if(submesh.m_mesh.m_indices.empty());
				s.m_indices.set_data(GL_ELEMENT_ARRAY_BUFFER, submesh.m_mesh.m_indices.data(), 1, submesh.m_mesh.m_indices.size(), GL_STATIC_DRAW);
				s.m_vertex_array.set_index_buffer(s.m_indices);

				m_submeshes.push_back(std::move(s));
			}
		}

		void basic_renderable::render(gl::renderer& renderer, camera_matrices const& matrices)
		{
			ZoneScopedN("basic_renderable::render()");

			auto mvp = matrices.model_view_projection_matrix(m_transform);
			auto n = matrices.normal_matrix(m_transform);

			renderer.set_program(*m_shader);

			for (auto const& submesh : m_submeshes)
			{
				renderer.set_uniform_4x4f(m_u_MVP, mvp);
				renderer.set_uniform_3x3f(m_u_NormalMatrix, n);
				renderer.set_uniform_3f(m_u_Color, submesh.m_color);
				renderer.set_uniform_1f(m_u_Metallic, submesh.m_metallic);
				renderer.set_uniform_1f(m_u_Roughness, submesh.m_roughness);
				renderer.set_uniform_1f(m_u_Emissive, submesh.m_emissive);

				renderer.set_vertex_array(submesh.m_vertex_array);

				renderer.draw_indexed(GL_TRIANGLES, submesh.m_indices.get_element_count(), submesh.m_indices.get_component_type());

				renderer.clear_vertex_array();
			}

			renderer.clear_program();
		}
		
		
		basic_renderable_alpha::basic_renderable_alpha(gl::shader_program const& shader, mbp_model const& model):
			m_shader(&shader),
			m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
			m_in_VertexNormal(shader.get_attribute_location("in_VertexNormal")),
			m_u_MVP(shader.get_uniform_location("u_MVP")),
			m_u_NormalMatrix(shader.get_uniform_location("u_NormalMatrix")),
			m_u_Color(shader.get_uniform_location("u_Color")),
			m_u_Metallic(shader.get_uniform_location("u_Metallic")),
			m_u_Roughness(shader.get_uniform_location("u_Roughness")),
			m_u_Emissive(shader.get_uniform_location("u_Emissive")),
			m_u_Opacity(shader.get_uniform_location("u_Opacity")),
			m_transform(1.f)
		{
			for (auto const& submesh : model.m_submeshes)
			{
				auto s = submesh_data();

				s.m_metallic = submesh.m_material.m_metallic;
				s.m_roughness = submesh.m_material.m_roughness;
				s.m_opacity = submesh.m_material.m_alpha;
				
				if (submesh.m_material.m_emissive_color != glm::vec3(0.0))
				{
					s.m_color = submesh.m_material.m_emissive_color; // replace base color with emissive
					s.m_emissive = 1.f; // set flag
				}
				else
				{
					s.m_color = submesh.m_material.m_base_color; // use standard base color
					s.m_emissive = 0.f; // clear flag
				}

				die_if(submesh.m_mesh.m_vertices.empty());
				s.m_vertices.set_data(GL_ARRAY_BUFFER, submesh.m_mesh.m_vertices.data(), 3, submesh.m_mesh.m_vertices.size() / 3, GL_STATIC_DRAW);
				s.m_vertex_array.set_array_buffer(m_in_VertexPosition, s.m_vertices);

				die_if(submesh.m_mesh.m_normals.empty());
				s.m_normals.set_data(GL_ARRAY_BUFFER, submesh.m_mesh.m_normals.data(), 3, submesh.m_mesh.m_normals.size() / 3, GL_STATIC_DRAW);
				s.m_vertex_array.set_array_buffer(m_in_VertexNormal, s.m_normals);

				die_if(submesh.m_mesh.m_indices.empty());
				s.m_indices.set_data(GL_ELEMENT_ARRAY_BUFFER, submesh.m_mesh.m_indices.data(), 1, submesh.m_mesh.m_indices.size(), GL_STATIC_DRAW);
				s.m_vertex_array.set_index_buffer(s.m_indices);

				m_submeshes.push_back(std::move(s));
			}
		}

		void basic_renderable_alpha::render(gl::renderer& renderer, camera_matrices const& matrices)
		{
			ZoneScopedN("basic_renderable_alpha::render()");

			auto mvp = matrices.model_view_projection_matrix(m_transform);
			auto n = matrices.normal_matrix(m_transform);

			renderer.set_blending(gl::renderer::blending::BLEND);
			renderer.set_program(*m_shader);

			for (auto const& submesh : m_submeshes)
			{
				renderer.set_uniform_4x4f(m_u_MVP, mvp);
				renderer.set_uniform_3x3f(m_u_NormalMatrix, n);
				renderer.set_uniform_3f(m_u_Color, submesh.m_color);
				renderer.set_uniform_1f(m_u_Metallic, submesh.m_metallic);
				renderer.set_uniform_1f(m_u_Roughness, submesh.m_roughness);
				renderer.set_uniform_1f(m_u_Emissive, submesh.m_emissive);
				renderer.set_uniform_1f(m_u_Opacity, submesh.m_opacity);

				renderer.set_vertex_array(submesh.m_vertex_array);

				renderer.draw_indexed(GL_TRIANGLES, submesh.m_indices.get_element_count(), submesh.m_indices.get_component_type());

				renderer.clear_vertex_array();
			}

			renderer.clear_program();
			renderer.set_blending(gl::renderer::blending::NONE);
		}
		
	} // game
	
} // bump