#include "bump_gbuffers.hpp"

#include "bump_camera.hpp"
#include "bump_narrow_cast.hpp"
#include "bump_mbp_model.hpp"

#include <Tracy.hpp>

namespace bump
{
	
	gbuffers::gbuffers(std::size_t buffer_count, glm::ivec2 screen_size)
	{
		recreate(buffer_count, screen_size);
	}

	void gbuffers::recreate(std::size_t buffer_count, glm::ivec2 screen_size)
	{
		die_if(glm::any(glm::lessThan(screen_size, glm::ivec2(0))));

		m_buffers.clear();
		m_depth_stencil.destroy();
		m_framebuffer.destroy();
		
		m_framebuffer = gl::framebuffer();

		auto draw_buffers = std::vector<GLenum>();
		draw_buffers.reserve(buffer_count);

		for (auto i = std::size_t{ 0 }; i != buffer_count; ++i)
		{
			auto texture = gl::texture_2d();
			texture.set_data(screen_size, GL_RGBA8, gl::make_texture_data_source(GL_RGBA, GL_UNSIGNED_BYTE));
			texture.set_min_filter(GL_NEAREST);
			texture.set_mag_filter(GL_NEAREST);

			auto const buffer_id = GL_COLOR_ATTACHMENT0 + narrow_cast<GLenum>(i);
			m_framebuffer.attach(buffer_id, texture);
			
			draw_buffers.push_back(buffer_id);
			m_buffers.push_back(std::move(texture));
		}

		m_depth_stencil = gl::texture_2d();
		m_depth_stencil.set_data(screen_size, GL_DEPTH24_STENCIL8, gl::make_texture_data_source(GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8));
		m_depth_stencil.set_min_filter(GL_NEAREST);
		m_depth_stencil.set_mag_filter(GL_NEAREST);

		m_framebuffer.attach(GL_DEPTH_ATTACHMENT, m_depth_stencil);
		m_framebuffer.attach(GL_STENCIL_ATTACHMENT, m_depth_stencil);

		m_framebuffer.set_draw_buffers(draw_buffers);

		die_if(!m_framebuffer.is_complete());
	}

	lighting_rendertarget::lighting_rendertarget(glm::ivec2 screen_size)
	{
		recreate(screen_size);
	}

	void lighting_rendertarget::recreate(glm::ivec2 screen_size)
	{
		m_texture.destroy();
		m_framebuffer.destroy();
		
		m_framebuffer = gl::framebuffer();

		m_texture = gl::texture_2d();
		m_texture.set_data(screen_size, GL_RGBA8, gl::make_texture_data_source(GL_RGBA, GL_UNSIGNED_BYTE));
		m_texture.set_min_filter(GL_NEAREST);
		m_texture.set_mag_filter(GL_NEAREST);

		m_framebuffer.attach(GL_COLOR_ATTACHMENT0, m_texture);
		
		m_framebuffer.set_draw_buffers({ GL_COLOR_ATTACHMENT0 });

		die_if(!m_framebuffer.is_complete());
	}

	textured_quad::textured_quad(gl::shader_program const& shader):
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
	
	void textured_quad::render(gl::texture_2d const& texture, gl::renderer& renderer, camera_matrices const& matrices)
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

	lighting_system::lighting_system(entt::registry& registry, gl::shader_program const& directional_light_shader, gl::shader_program const& point_light_shader, mbp_model const& point_light_model):
		m_registry(registry),
		m_renderable_directional(registry, directional_light_shader),
		m_renderable_point(registry, point_light_shader, point_light_model) { }
	
	void lighting_system::render(gl::renderer& renderer, glm::vec2 screen_size, camera_matrices const& scene_matrices, camera_matrices const& ui_matrices, gbuffers const& gbuf)
	{
		renderer.set_depth_test(gl::renderer::depth_test::ALWAYS);
		renderer.set_depth_write(gl::renderer::depth_write::DISABLED);
		renderer.set_blending(gl::renderer::blending::ADD);

		m_renderable_directional.render(renderer, screen_size, scene_matrices, ui_matrices, gbuf);

		renderer.set_depth_test(gl::renderer::depth_test::GREATER_EQUAL);
		renderer.set_face_culling(gl::renderer::face_culling::COUNTER_CLOCKWISE);

		//m_renderable_point.render(renderer, scene_matrices, gbuf);

		renderer.set_face_culling(gl::renderer::face_culling::CLOCKWISE);
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
		m_g_buffer_depth(m_shader.get_uniform_location("g_buffer_depth")),
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

	void lighting_system::directional_light_renderable::render(gl::renderer& renderer, glm::vec2 screen_size, camera_matrices const& scene_matrices, camera_matrices const& ui_matrices, gbuffers const& gbuf)
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

			renderer.set_depth_test(gl::renderer::depth_test::ALWAYS);
			renderer.set_program(m_shader);
			renderer.set_uniform_4x4f(m_u_MVP, mvp);
			renderer.set_uniform_2f(m_u_Size, screen_size);
			renderer.set_uniform_1i(m_g_buffer_1, 0);
			renderer.set_uniform_1i(m_g_buffer_2, 1);
			renderer.set_uniform_1i(m_g_buffer_depth, 2);
			renderer.set_texture_2d(0, gbuf.m_buffers[0]);
			renderer.set_texture_2d(1, gbuf.m_buffers[1]);
			renderer.set_texture_2d(2, gbuf.m_depth_stencil);
			renderer.set_uniform_4x4f(m_u_InvProjMatrix, scene_matrices.m_inv_projection);
			renderer.set_vertex_array(m_vertex_array);

			renderer.draw_arrays(GL_TRIANGLES, m_buffer_vertices.get_element_count(), m_buffer_light_directions.get_element_count());

			renderer.clear_vertex_array();
			renderer.clear_texture_2d(2);
			renderer.clear_texture_2d(1);
			renderer.clear_texture_2d(0);
			renderer.clear_program();
			renderer.set_depth_test(gl::renderer::depth_test::LESS);
		}
	}

	lighting_system::point_light_renderable::point_light_renderable(entt::registry& registry, gl::shader_program const& shader, mbp_model const& model):
		m_registry(registry),
		m_shader(shader),
		m_in_VertexPosition(m_shader.get_attribute_location("in_VertexPosition")),
		m_in_LightPosition(m_shader.get_attribute_location("in_LightPosition")),
		m_in_LightColor(m_shader.get_attribute_location("in_LightColor")),
		m_in_LightRadius(m_shader.get_attribute_location("in_LightRadius")),
		m_u_MVP(m_shader.get_uniform_location("u_MVP")),
		m_g_buffer_1(m_shader.get_uniform_location("g_buffer_1")),
		m_g_buffer_2(m_shader.get_uniform_location("g_buffer_2")),
		m_g_buffer_depth(m_shader.get_uniform_location("g_buffer_depth")),
		m_u_InvProjMatrix(m_shader.get_uniform_location("u_InvProjMatrix"))
	{
		die_if(model.m_submeshes.size() != 1);

		// TODO: get vertex data!!! <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

		m_buffer_light_positions.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 3, 0, GL_STREAM_DRAW);
		m_vertex_array.set_array_buffer(m_in_LightPosition, m_buffer_light_positions, 1);
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
			m_frame_light_colors.reserve(view.size());
			m_frame_light_radii.reserve(view.size());

			for (auto id : view)
			{
				auto const& l = view.get<point_light>(id);
				m_frame_light_positions.push_back(glm::vec3(scene_matrices.m_view * glm::vec4(l.m_position, 1.0)));
				m_frame_light_colors.push_back(l.m_color);
				m_frame_light_radii.push_back(l.m_radius);
			}

			m_buffer_light_positions.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_frame_light_positions.front()), 3, m_frame_light_positions.size(), GL_STREAM_DRAW);
			m_buffer_light_colors.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_frame_light_colors.front()), 3, m_frame_light_colors.size(), GL_STREAM_DRAW);
			m_buffer_light_radii.set_data(GL_ARRAY_BUFFER, m_frame_light_radii.data(), 1, m_frame_light_radii.size(), GL_STREAM_DRAW);

			m_frame_light_positions.clear();
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
			renderer.set_uniform_1i(m_g_buffer_depth, 2);
			renderer.set_texture_2d(0, gbuf.m_buffers[0]);
			renderer.set_texture_2d(1, gbuf.m_buffers[1]);
			renderer.set_texture_2d(2, gbuf.m_depth_stencil);
			renderer.set_uniform_4x4f(m_u_InvProjMatrix, scene_matrices.m_inv_projection);
			renderer.set_vertex_array(m_vertex_array);

			renderer.draw_indexed(GL_TRIANGLES, m_buffer_indices.get_element_count(), m_buffer_indices.get_component_type(), m_buffer_light_positions.get_element_count());

			renderer.clear_vertex_array();
			renderer.clear_program();
		}
	}

	// todo:

		// blit skybox somewhere...
			// either to the main framebuffer after lighting
			// do we want the lighting target to ONLY be lighting? or what?

		// rename gbuffers.hpp/cpp to lighting.hpp/cpp put stuff in a lighting namespace
		
		// test spherical normal conversion vs storing normal directly. is it actually better?

		// write material parameters to gbuffers and use in lighting calculations

		// transparent rendering questions:
			// where to do transparent rendering?
			// should particles write to the depth buffer?
			// do we need a separate texture to render transparent stuff onto first?
		
		// add gbuffers to start screen too

} // bump
