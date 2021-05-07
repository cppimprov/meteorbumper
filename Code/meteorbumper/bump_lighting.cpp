#include "bump_lighting.hpp"

#include "bump_camera.hpp"
#include "bump_narrow_cast.hpp"
#include "bump_mbp_model.hpp"

#include <Tracy.hpp>

namespace bump
{

	namespace lighting
	{
	
		gbuffers::gbuffers(glm::ivec2 screen_size)
		{
			recreate(screen_size);
		}

		void gbuffers::recreate(glm::ivec2 screen_size)
		{
			die_if(glm::any(glm::lessThan(screen_size, glm::ivec2(0))));

			m_buffer_1.destroy();
			m_buffer_2.destroy();
			m_buffer_3.destroy();

			m_depth_stencil.destroy();
			m_framebuffer.destroy();
			
			m_framebuffer = gl::framebuffer();

			m_buffer_1 = gl::texture_2d();
			m_buffer_1.set_data(screen_size, GL_RGBA8, gl::make_texture_data_source(GL_RGBA, GL_UNSIGNED_BYTE));
			m_buffer_1.set_min_filter(GL_NEAREST);
			m_buffer_1.set_mag_filter(GL_NEAREST);
			m_framebuffer.attach_texture(GL_COLOR_ATTACHMENT0, m_buffer_1);

			m_buffer_2 = gl::texture_2d();
			m_buffer_2.set_data(screen_size, GL_RGBA16F, gl::make_texture_data_source(GL_RGBA, GL_FLOAT));
			m_buffer_2.set_min_filter(GL_NEAREST);
			m_buffer_2.set_mag_filter(GL_NEAREST);
			m_framebuffer.attach_texture(GL_COLOR_ATTACHMENT1, m_buffer_2);

			m_buffer_3 = gl::texture_2d();
			m_buffer_3.set_data(screen_size, GL_RGBA8, gl::make_texture_data_source(GL_RGBA, GL_UNSIGNED_BYTE));
			m_buffer_3.set_min_filter(GL_NEAREST);
			m_buffer_3.set_mag_filter(GL_NEAREST);
			m_framebuffer.attach_texture(GL_COLOR_ATTACHMENT2, m_buffer_3);

			m_depth_stencil = gl::renderbuffer();
			m_depth_stencil.set_data(screen_size, GL_DEPTH24_STENCIL8);

			m_framebuffer.attach_renderbuffer(GL_DEPTH_STENCIL_ATTACHMENT, m_depth_stencil);

			m_framebuffer.set_draw_buffers({ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 });

			die_if(!m_framebuffer.is_complete());
		}

		shadow_rendertarget::shadow_rendertarget(glm::ivec2 size)
		{
			m_texture.set_data(size, GL_DEPTH_COMPONENT24, gl::make_texture_data_source(GL_DEPTH_COMPONENT, GL_FLOAT));
			m_texture.set_min_filter(GL_NEAREST);
			m_texture.set_mag_filter(GL_NEAREST);
			
			m_framebuffer.attach_texture(GL_DEPTH_ATTACHMENT, m_texture);
			m_framebuffer.set_draw_buffers({ GL_NONE });

			die_if(!m_framebuffer.is_complete());
		}

		lighting_rendertarget::lighting_rendertarget(glm::ivec2 screen_size, gl::renderbuffer const& depth_stencil_rt)
		{
			recreate(screen_size, depth_stencil_rt);
		}

		void lighting_rendertarget::recreate(glm::ivec2 screen_size, gl::renderbuffer const& depth_stencil_rt)
		{
			m_texture.destroy();
			m_framebuffer.destroy();
			
			m_framebuffer = gl::framebuffer();

			m_texture = gl::texture_2d();
			m_texture.set_data(screen_size, GL_RGBA16F, gl::make_texture_data_source(GL_RGBA, GL_FLOAT));
			m_texture.set_min_filter(GL_NEAREST);
			m_texture.set_mag_filter(GL_NEAREST);

			m_framebuffer.attach_texture(GL_COLOR_ATTACHMENT0, m_texture);
			m_framebuffer.attach_renderbuffer(GL_DEPTH_STENCIL_ATTACHMENT, depth_stencil_rt);
			
			m_framebuffer.set_draw_buffers({ GL_COLOR_ATTACHMENT0 });

			die_if(!m_framebuffer.is_complete());
		}

		tone_map_quad::tone_map_quad(gl::shader_program const& shader):
			m_shader(shader),
			m_position(0.f),
			m_size(1.f),
			m_in_VertexPosition(m_shader.get_attribute_location("in_VertexPosition")),
			m_u_MVP(m_shader.get_uniform_location("u_MVP")),
			m_u_Position(m_shader.get_uniform_location("u_Position")),
			m_u_Size(m_shader.get_uniform_location("u_Size")),
			m_u_Texture(m_shader.get_uniform_location("u_Texture"))
		{
			auto vertices = { 0.f, 0.f,  1.f, 0.f,  1.f, 1.f,  0.f, 0.f,  1.f, 1.f,  0.f, 1.f, };
			m_vertex_buffer.set_data(GL_ARRAY_BUFFER, vertices.begin(), 2, 6, GL_STATIC_DRAW);
			m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertex_buffer);
		}
		
		void tone_map_quad::render(gl::texture_2d const& texture, gl::renderer& renderer, camera_matrices const& matrices)
		{
			ZoneScopedN("textured_quad::render()");

			auto const mvp = matrices.model_view_projection_matrix(glm::mat4(1.f));

			renderer.set_depth_test(gl::renderer::depth_test::ALWAYS);

			renderer.set_program(m_shader);
			renderer.set_uniform_4x4f(m_u_MVP, mvp);
			renderer.set_uniform_2f(m_u_Position, m_position);
			renderer.set_uniform_2f(m_u_Size, m_size);
			renderer.set_uniform_1i(m_u_Texture, 0);
			
			renderer.set_texture_2d(0, texture);
			renderer.set_vertex_array(m_vertex_array);

			renderer.draw_arrays(GL_TRIANGLES, m_vertex_buffer.get_element_count());

			renderer.clear_vertex_array();
			renderer.clear_texture_2d(0);
			renderer.clear_program();

			renderer.set_depth_test(gl::renderer::depth_test::LESS);
		}

		lighting_system::lighting_system(entt::registry& registry, gl::shader_program const& directional_light_shader, gl::shader_program const& point_light_shader, mbp_model const& point_light_model, gl::shader_program const& emissive_shader):
			m_registry(registry),
			m_renderable_directional(registry, directional_light_shader),
			m_renderable_point(registry, point_light_shader, point_light_model),
			m_renderable_emissive(registry, emissive_shader) { }
		
		void lighting_system::render(gl::renderer& renderer, glm::vec2 screen_size, camera_matrices const& light_matrices, camera_matrices const& scene_matrices, camera_matrices const& ui_matrices, gbuffers const& gbuf, gl::texture_2d const& shadow_map)
		{
			ZoneScopedN("lighting_system::render()");

			renderer.set_depth_test(gl::renderer::depth_test::ALWAYS);
			renderer.set_depth_write(gl::renderer::depth_write::DISABLED);
			renderer.set_blending(gl::renderer::blending::ADD);

			m_renderable_directional.render(renderer, screen_size, light_matrices, scene_matrices, ui_matrices, gbuf, shadow_map);

			renderer.set_depth_test(gl::renderer::depth_test::GREATER_EQUAL);
			renderer.set_face_culling(gl::renderer::face_culling::COUNTER_CLOCKWISE);

			m_renderable_point.render(renderer, scene_matrices, gbuf);

			renderer.set_face_culling(gl::renderer::face_culling::CLOCKWISE);
			renderer.set_depth_test(gl::renderer::depth_test::ALWAYS);

			m_renderable_emissive.render(renderer, screen_size, ui_matrices, gbuf);

			renderer.set_blending(gl::renderer::blending::NONE);
			renderer.set_depth_write(gl::renderer::depth_write::ENABLED);
			renderer.set_depth_test(gl::renderer::depth_test::LESS);
		}

		lighting_system::directional_light_renderable::directional_light_renderable(entt::registry& registry, gl::shader_program const& shader):
			m_registry(registry),
			m_shader(shader),
			m_in_VertexPosition(m_shader.get_attribute_location("in_VertexPosition")),
			m_in_LightDirection(m_shader.get_attribute_location("in_LightDirection")),
			m_in_LightColor(m_shader.get_attribute_location("in_LightColor")),
			m_u_MVP(m_shader.get_uniform_location("u_MVP")),
			m_u_Size(m_shader.get_uniform_location("u_Size")),
			m_g_buffer_1(m_shader.get_uniform_location("g_buffer_1")),
			m_g_buffer_2(m_shader.get_uniform_location("g_buffer_2")),
			m_g_buffer_3(m_shader.get_uniform_location("g_buffer_3")),
			m_u_Shadows(m_shader.get_uniform_location("u_Shadows")),
			m_u_LightViewMatrix(m_shader.get_uniform_location("u_LightViewMatrix")),
			m_u_InvViewMatrix(m_shader.get_uniform_location("u_InvViewMatrix")),
			m_u_InvProjMatrix(m_shader.get_uniform_location("u_InvProjMatrix"))
		{
			auto vertices = { 0.f, 0.f,  1.f, 0.f,  1.f, 1.f,  0.f, 0.f,  1.f, 1.f,  0.f, 1.f, };
			m_buffer_vertices.set_data(GL_ARRAY_BUFFER, vertices.begin(), 2, 6, GL_STATIC_DRAW);
			m_vertex_array.set_array_buffer(m_in_VertexPosition, m_buffer_vertices);

			m_buffer_light_directions.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 3, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_LightDirection, m_buffer_light_directions, 1);
			m_buffer_light_colors.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 3, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_LightColor, m_buffer_light_colors, 1);
		}

		void lighting_system::directional_light_renderable::render(gl::renderer& renderer, glm::vec2 screen_size, camera_matrices const& light_matrices, camera_matrices const& scene_matrices, camera_matrices const& ui_matrices, gbuffers const& gbuf, gl::texture_2d const& shadow_map)
		{
			// get instance data
			{
				auto view = m_registry.view<directional_light>();
				if (view.empty()) return;

				m_frame_light_directions.reserve(view.size());
				m_frame_light_colors.reserve(view.size());

				for (auto id : view)
				{
					auto const& l = view.get<directional_light>(id);
					m_frame_light_directions.push_back(glm::vec3(scene_matrices.m_view * glm::vec4(l.m_direction, 0.f)));
					m_frame_light_colors.push_back(l.m_color);
				}

				m_buffer_light_directions.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_frame_light_directions.front()), 3, m_frame_light_directions.size(), GL_STREAM_DRAW);
				m_buffer_light_colors.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_frame_light_colors.front()), 3, m_frame_light_colors.size(), GL_STREAM_DRAW);

				m_frame_light_directions.clear();
				m_frame_light_colors.clear();
			}

			// render
			{
				auto mvp = ui_matrices.model_view_projection_matrix(glm::mat4(1.f));

				renderer.set_program(m_shader);
				renderer.set_uniform_4x4f(m_u_MVP, mvp);
				renderer.set_uniform_2f(m_u_Size, screen_size);
				renderer.set_uniform_1i(m_g_buffer_1, 0);
				renderer.set_uniform_1i(m_g_buffer_2, 1);
				renderer.set_uniform_1i(m_g_buffer_3, 2);
				renderer.set_uniform_1i(m_u_Shadows, 3);
				renderer.set_texture_2d(0, gbuf.m_buffer_1);
				renderer.set_texture_2d(1, gbuf.m_buffer_2);
				renderer.set_texture_2d(2, gbuf.m_buffer_3);
				renderer.set_texture_2d(3, shadow_map);
				renderer.set_uniform_4x4f(m_u_LightViewMatrix, light_matrices.m_view_projection);
				renderer.set_uniform_4x4f(m_u_InvViewMatrix, scene_matrices.m_inv_view);
				renderer.set_uniform_4x4f(m_u_InvProjMatrix, scene_matrices.m_inv_projection);
				renderer.set_vertex_array(m_vertex_array);

				renderer.draw_arrays(GL_TRIANGLES, m_buffer_vertices.get_element_count(), m_buffer_light_directions.get_element_count());

				renderer.clear_vertex_array();
				renderer.clear_texture_2d(3);
				renderer.clear_texture_2d(2);
				renderer.clear_texture_2d(1);
				renderer.clear_texture_2d(0);
				renderer.clear_program();
			}
		}

		lighting_system::point_light_renderable::point_light_renderable(entt::registry& registry, gl::shader_program const& shader, mbp_model const& model):
			m_registry(registry),
			m_shader(shader),
			m_in_VertexPosition(m_shader.get_attribute_location("in_VertexPosition")),
			m_in_LightPosition(m_shader.get_attribute_location("in_LightPosition")),
			m_in_LightPosition_vs(m_shader.get_attribute_location("in_LightPosition_vs")),
			m_in_LightColor(m_shader.get_attribute_location("in_LightColor")),
			m_in_LightRadius(m_shader.get_attribute_location("in_LightRadius")),
			m_u_MVP(m_shader.get_uniform_location("u_MVP")),
			m_g_buffer_1(m_shader.get_uniform_location("g_buffer_1")),
			m_g_buffer_2(m_shader.get_uniform_location("g_buffer_2")),
			m_g_buffer_3(m_shader.get_uniform_location("g_buffer_3")),
			m_u_InvProjMatrix(m_shader.get_uniform_location("u_InvProjMatrix"))
		{
			die_if(model.m_submeshes.size() != 1);
			auto const& mesh = model.m_submeshes.front().m_mesh;

			die_if(mesh.m_vertices.empty());
			m_buffer_vertices.set_data(GL_ARRAY_BUFFER, mesh.m_vertices.data(), 3, mesh.m_vertices.size() / 3, GL_STATIC_DRAW);
			m_vertex_array.set_array_buffer(m_in_VertexPosition, m_buffer_vertices);

			die_if(mesh.m_indices.empty());
			m_buffer_indices.set_data(GL_ARRAY_BUFFER, mesh.m_indices.data(), 1, mesh.m_indices.size(), GL_STATIC_DRAW);
			m_vertex_array.set_index_buffer(m_buffer_indices);

			m_buffer_light_positions.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 3, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_LightPosition, m_buffer_light_positions, 1);
			m_buffer_light_positions_vs.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 3, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_LightPosition_vs, m_buffer_light_positions_vs, 1);
			m_buffer_light_colors.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 3, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_LightColor, m_buffer_light_colors, 1);
			m_buffer_light_radii.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 1, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_LightRadius, m_buffer_light_radii, 1);
		}

		void lighting_system::point_light_renderable::render(gl::renderer& renderer, camera_matrices const& scene_matrices, gbuffers const& gbuf)
		{
			// get instance data
			{
				auto view = m_registry.view<point_light>();
				if (view.empty()) return;

				m_frame_light_positions.reserve(view.size());
				m_frame_light_positions_vs.reserve(view.size());
				m_frame_light_colors.reserve(view.size());
				m_frame_light_radii.reserve(view.size());

				for (auto id : view)
				{
					auto const& l = view.get<point_light>(id);
					m_frame_light_positions.push_back(l.m_position);
					m_frame_light_positions_vs.push_back(glm::vec3(scene_matrices.m_view * glm::vec4(l.m_position, 1.0)));
					m_frame_light_colors.push_back(l.m_color);
					m_frame_light_radii.push_back(l.m_radius);
				}

				m_buffer_light_positions.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_frame_light_positions.front()), 3, m_frame_light_positions.size(), GL_STREAM_DRAW);
				m_buffer_light_positions_vs.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_frame_light_positions_vs.front()), 3, m_frame_light_positions_vs.size(), GL_STREAM_DRAW);
				m_buffer_light_colors.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_frame_light_colors.front()), 3, m_frame_light_colors.size(), GL_STREAM_DRAW);
				m_buffer_light_radii.set_data(GL_ARRAY_BUFFER, m_frame_light_radii.data(), 1, m_frame_light_radii.size(), GL_STREAM_DRAW);

				m_frame_light_positions.clear();
				m_frame_light_positions_vs.clear();
				m_frame_light_colors.clear();
				m_frame_light_radii.clear();
			}

			// render
			{
				auto const& mvp = scene_matrices.m_view_projection; // note: not actually an mvp

				renderer.set_program(m_shader);
				renderer.set_uniform_4x4f(m_u_MVP, mvp);
				renderer.set_uniform_1i(m_g_buffer_1, 0);
				renderer.set_uniform_1i(m_g_buffer_2, 1);
				renderer.set_uniform_1i(m_g_buffer_3, 2);
				renderer.set_texture_2d(0, gbuf.m_buffer_1);
				renderer.set_texture_2d(1, gbuf.m_buffer_2);
				renderer.set_texture_2d(2, gbuf.m_buffer_3);
				renderer.set_uniform_4x4f(m_u_InvProjMatrix, scene_matrices.m_inv_projection);
				renderer.set_vertex_array(m_vertex_array);

				renderer.draw_indexed(GL_TRIANGLES, m_buffer_indices.get_element_count(), m_buffer_indices.get_component_type(), m_buffer_light_positions.get_element_count());

				renderer.clear_vertex_array();
				renderer.clear_program();
			}
		}

		lighting_system::emissive_renderable::emissive_renderable(entt::registry& registry, gl::shader_program const& shader):
			m_registry(registry),
			m_shader(shader),
			m_in_VertexPosition(m_shader.get_attribute_location("in_VertexPosition")),
			m_u_MVP(m_shader.get_uniform_location("u_MVP")),
			m_u_Size(m_shader.get_uniform_location("u_Size")),
			m_g_buffer_1(m_shader.get_uniform_location("g_buffer_1")),
			m_g_buffer_2(m_shader.get_uniform_location("g_buffer_2")),
			m_g_buffer_3(m_shader.get_uniform_location("g_buffer_3"))
		{
			auto vertices = { 0.f, 0.f,  1.f, 0.f,  1.f, 1.f,  0.f, 0.f,  1.f, 1.f,  0.f, 1.f, };
			m_buffer_vertices.set_data(GL_ARRAY_BUFFER, vertices.begin(), 2, 6, GL_STATIC_DRAW);
			m_vertex_array.set_array_buffer(m_in_VertexPosition, m_buffer_vertices);
		}
		
		void lighting_system::emissive_renderable::render(gl::renderer& renderer, glm::vec2 screen_size, camera_matrices const& ui_matrices, gbuffers const& gbuf)
		{
			auto mvp = ui_matrices.model_view_projection_matrix(glm::mat4(1.f));

			renderer.set_program(m_shader);
			renderer.set_uniform_4x4f(m_u_MVP, mvp);
			renderer.set_uniform_2f(m_u_Size, screen_size);
			renderer.set_uniform_1i(m_g_buffer_1, 0);
			renderer.set_uniform_1i(m_g_buffer_2, 1);
			renderer.set_uniform_1i(m_g_buffer_3, 2);
			renderer.set_texture_2d(0, gbuf.m_buffer_1);
			renderer.set_texture_2d(1, gbuf.m_buffer_2);
			renderer.set_texture_2d(2, gbuf.m_buffer_3);
			renderer.set_vertex_array(m_vertex_array);

			renderer.draw_arrays(GL_TRIANGLES, m_buffer_vertices.get_element_count());

			renderer.clear_vertex_array();
			renderer.clear_texture_2d(2);
			renderer.clear_texture_2d(1);
			renderer.clear_texture_2d(0);
			renderer.clear_program();
		}

	} // lighting

} // bump


// todo:

	// shadows

		// render scene depth
			// player
			// bouys
			// powerups
		
		// don't render color output! render depth instead...

	// add spotlights
		// use for engine lights
		// use for player searchlight(s)
		// add a fake volumetric effect by rendering a cone?
	
	// add shadows? (from main light only?)

	// make sure that directional lights ignore skybox pixels properly...?
	
	// make it simpler to use the lighting stuff (put everything in lighting_system class?)

	// add deferred lighting to start screen too!
