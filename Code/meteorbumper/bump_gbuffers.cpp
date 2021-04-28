#include "bump_gbuffers.hpp"

#include "bump_narrow_cast.hpp"

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

		m_framebuffer.destroy();
		m_buffers.clear();
		m_depth_stencil.destroy();
		
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

	textured_quad::textured_quad(gl::shader_program const& shader):
		m_shader(shader),
		m_position(0.f),
		m_size(1.f),
		m_in_VertexPosition(m_shader.get_attribute_location("in_VertexPosition")),
		m_u_MVP(m_shader.get_uniform_location("u_MVP")),
		m_u_Position(m_shader.get_uniform_location("u_Position")),
		m_u_Size(m_shader.get_uniform_location("u_Size")),
		m_u_Texture(m_shader.get_uniform_location("g_buffer_1")) // !!!
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

	// todo:

		// remove gbuffer stuff from particle rendering for now!
		// add lighting pass!

		// transparent rendering questions:
			// where to do transparent rendering?
			// should particles write to the depth buffer?
			// do we need a separate texture to render transparent stuff onto first?
		
		// add gbuffers to start screen too
	
	// then:
		// lighting pass + lights!

} // bump
