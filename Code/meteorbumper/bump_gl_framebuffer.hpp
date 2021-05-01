#pragma once

#include "bump_die.hpp"

#include "bump_gl_object_handle.hpp"
#include "bump_gl_renderbuffer.hpp"

#include <GL/glew.h>

namespace bump
{
	
	namespace gl
	{
		
		class framebuffer : public object_handle
		{
		public:

			framebuffer();

			template<class TextureT>
			void attach_texture(GLenum location, TextureT const& texture) { die_if(!texture.is_valid()); attach_texture(location, texture.get_id()); }
			void attach_renderbuffer(GLenum location, renderbuffer const& rb) { die_if(!rb.is_valid()); attach_renderbuffer(location, rb.get_id()); }

			void detach_texture(GLenum location);
			void detach_renderbuffer(GLenum location);

			void set_draw_buffers(std::vector<GLenum> const& buffers);

			GLenum get_status() const;
			bool is_complete() const;

		private:

			void attach_texture(GLenum location, GLuint texture_id);
			void attach_renderbuffer(GLenum location, GLuint renderbuffer_id);
		};

	} // gl
	
} // bump
