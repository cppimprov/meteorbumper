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
		
		basic_renderable_instanced::basic_renderable_instanced(gl::shader_program const& shader, mbp_model const& model):
			m_shader(&shader),
			m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
			m_in_VertexNormal(shader.get_attribute_location("in_VertexNormal")),
			m_in_MVP(shader.get_attribute_location("in_MVP")),
			m_in_NormalMatrix(shader.get_attribute_location("in_NormalMatrix")),
			m_u_Color(shader.get_uniform_location("u_Color")),
			m_u_Metallic(shader.get_uniform_location("u_Metallic")),
			m_u_Roughness(shader.get_uniform_location("u_Roughness")),
			m_u_Emissive(shader.get_uniform_location("u_Emissive"))
		{
			m_buffer_mvp_matrices.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 16, 0, GL_STREAM_DRAW);
			m_buffer_normal_matrices.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 9, 0, GL_STREAM_DRAW);

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

				s.m_vertex_array.set_array_buffer(m_in_MVP, m_buffer_mvp_matrices, 4, 4, 1);
				s.m_vertex_array.set_array_buffer(m_in_NormalMatrix, m_buffer_normal_matrices, 3, 3, 1);

				m_submeshes.push_back(std::move(s));
			}
		}

		void basic_renderable_instanced::render(gl::renderer& renderer, camera_matrices const& matrices, std::vector<glm::mat4> const& transforms)
		{
			ZoneScopedN("basic_renderable_instanced::render()");

			auto const instance_count = transforms.size();
			if (instance_count == 0) return;

			m_frame_mvp_matrices.reserve(instance_count);
			m_frame_normal_matrices.reserve(instance_count);

			for (auto const& t : transforms)
			{
				m_frame_mvp_matrices.push_back(matrices.model_view_projection_matrix(t));
				m_frame_normal_matrices.push_back(matrices.normal_matrix(t));
			}
			
			m_buffer_mvp_matrices.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_frame_mvp_matrices.front()), 16, instance_count, GL_STREAM_DRAW);
			m_buffer_normal_matrices.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_frame_normal_matrices.front()), 9, instance_count, GL_STREAM_DRAW);

			m_frame_mvp_matrices.clear();
			m_frame_normal_matrices.clear();

			renderer.set_program(*m_shader);

			for (auto const& submesh : m_submeshes)
			{
				renderer.set_uniform_3f(m_u_Color, submesh.m_color);
				renderer.set_uniform_1f(m_u_Metallic, submesh.m_metallic);
				renderer.set_uniform_1f(m_u_Roughness, submesh.m_roughness);
				renderer.set_uniform_1f(m_u_Emissive, submesh.m_emissive);

				renderer.set_vertex_array(submesh.m_vertex_array);

				renderer.draw_indexed(GL_TRIANGLES, submesh.m_indices.get_element_count(), submesh.m_indices.get_component_type(), instance_count);

				renderer.clear_vertex_array();
			}

			renderer.clear_program();
		}
		
	} // game
	
} // bump