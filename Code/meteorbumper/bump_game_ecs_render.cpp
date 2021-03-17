#include "bump_game_ecs_render.hpp"

#include "bump_camera.hpp"
#include "bump_game_ecs_physics.hpp"
#include "bump_mbp_model.hpp"

namespace bump
{
	
	namespace game::ecs
	{
		
		void render_system::render(entt::registry& registry, gl::renderer& renderer, perspective_camera const& scene_camera, camera_matrices const& matrices)
		{
			// update transforms for physics objects
			{
				auto view = registry.view<physics_component, basic_renderable>();

				for (auto id : view)
					view.get<basic_renderable>(id).set_transform(view.get<physics_component>(id).get_transform());
			}

			// render skybox
			{
				auto view = registry.view<skybox_renderable>();
				die_if(view.size() != 1);

				for (auto id : view)
					view.get<skybox_renderable>(id).render(renderer, scene_camera, matrices);
			}

			// render
			{
				auto view = registry.view<basic_renderable>();

				for (auto id : view)
					view.get<basic_renderable>(id).render(renderer, matrices);
			}
		}

		skybox_renderable::skybox_renderable(mbp_model const& model, gl::shader_program const& shader, gl::texture_cubemap const& texture):
			m_shader(&shader),
			m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
			m_u_MVP(shader.get_uniform_location("u_MVP")),
			m_u_Scale(shader.get_uniform_location("u_Scale")),
			m_u_CubemapTexture(shader.get_uniform_location("u_CubemapTexture")),
			m_texture(&texture)
		{
			die_if(model.m_submeshes.size() != 1);

			auto const& mesh = model.m_submeshes.front();

			m_vertices.set_data(GL_ARRAY_BUFFER, mesh.m_mesh.m_vertices.data(), 3, mesh.m_mesh.m_vertices.size() / 3, GL_STATIC_DRAW);
			m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertices);

			m_indices.set_data(GL_ELEMENT_ARRAY_BUFFER, mesh.m_mesh.m_indices.data(), 1, mesh.m_mesh.m_indices.size(), GL_STATIC_DRAW);
			m_vertex_array.set_index_buffer(m_indices);
		}

		void skybox_renderable::render(gl::renderer& renderer, perspective_camera const& scene_camera, camera_matrices const& matrices)
		{
			auto model = glm::translate(glm::mat4(1.f), get_position(scene_camera.m_transform));
			auto mvp = matrices.model_view_projection_matrix(model);

			renderer.set_depth_write(gl::renderer::depth_write::DISABLED);

			renderer.set_program(*m_shader);

			renderer.set_uniform_4x4f(m_u_MVP, mvp);
			renderer.set_uniform_1f(m_u_Scale, 5.f); // note: ensure we render beyond the camera's near plane!
			renderer.set_uniform_1i(m_u_CubemapTexture, 0);

			renderer.set_texture_cubemap(0, *m_texture);
			renderer.set_seamless_cubemaps(gl::renderer::seamless_cubemaps::ENABLED);
			
			renderer.set_vertex_array(m_vertex_array);

			renderer.draw_indexed(GL_TRIANGLES, m_indices.get_element_count(), m_indices.get_component_type());

			renderer.clear_vertex_array();
			renderer.set_seamless_cubemaps(gl::renderer::seamless_cubemaps::DISABLED);
			renderer.clear_texture_cubemap(0);
			renderer.clear_program();
			renderer.set_depth_write(gl::renderer::depth_write::ENABLED);
		}
		
		basic_renderable::basic_renderable(mbp_model const& model, gl::shader_program const& shader):
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
		
	} // game::ecs
	
} // bump