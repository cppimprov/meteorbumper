#include "bump_game_skybox.hpp"

#include "bump_camera.hpp"
#include "bump_mbp_model.hpp"
#include "bump_transform.hpp"

#include <Tracy.hpp>

namespace bump
{
	
	namespace game
	{
		
		skybox::skybox(mbp_model const& model, gl::shader_program const& shader, gl::texture_cubemap const& texture):
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

		void skybox::render_scene(gl::renderer& renderer, perspective_camera const& scene_camera, camera_matrices const& matrices)
		{
			ZoneScopedN("skybox::render_scene()");

			auto model = glm::translate(glm::mat4(1.f), get_position(scene_camera.m_transform));
			auto mvp = matrices.model_view_projection_matrix(model);

			renderer.set_depth_test(gl::renderer::depth_test::LESS_EQUAL);
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
			renderer.set_depth_test(gl::renderer::depth_test::LESS);
		}
		
	} // game
	
} // bump