#include "bump_gbuffers.hpp"

#include "bump_narrow_cast.hpp"

namespace bump
{
	
	gbuffers::gbuffers(std::size_t buffer_count, glm::vec<2, GLsizei> screen_size)
	{
		recreate(buffer_count, screen_size);
	}

	void gbuffers::recreate(std::size_t buffer_count, glm::vec<2, GLsizei> screen_size)
	{
		m_framebuffer.destroy();
		m_buffers.clear();
		m_depth_stencil.destroy();
		
		m_framebuffer = gl::framebuffer();

		for (auto i = std::size_t{ 0 }; i != buffer_count; ++i)
		{
			auto texture = gl::texture_2d();
			texture.set_data(screen_size, GL_RGBA8, gl::make_texture_data_source(GL_RGBA, GL_UNSIGNED_BYTE));
			texture.set_min_filter(GL_NEAREST);
			texture.set_mag_filter(GL_NEAREST);

			m_framebuffer.attach(GL_COLOR_ATTACHMENT0 + narrow_cast<GLenum>(i), texture);
			m_buffers.push_back(std::move(texture));
		}

		m_depth_stencil = gl::texture_2d();
		m_depth_stencil.set_data(screen_size, GL_DEPTH24_STENCIL8, gl::make_texture_data_source(GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8));
		m_depth_stencil.set_min_filter(GL_NEAREST);
		m_depth_stencil.set_mag_filter(GL_NEAREST);

		m_framebuffer.attach(GL_DEPTH_ATTACHMENT, m_depth_stencil);
		m_framebuffer.attach(GL_STENCIL_ATTACHMENT, m_depth_stencil);

		die_if(!m_framebuffer.is_complete());
	}

	// todo:
		// copy over gbuffer shaders
		// change current shaders to render to gbuffers
		// add a blit pass to take the color data and put in on screen (temp to make sure things work)!
		// add gbuffers to start screen too
	
	// then:
		// lighting pass + lights!

} // bump
