#pragma once

#include "bump_gl_object_handle.hpp"

#include <GL/glew.h>

namespace bump
{
	
	namespace gl
	{
		
		class framebuffer : public object_handle<>
		{
		public:

			framebuffer();

			template<class TextureT>
			void attach(GLenum location, TextureT const& texture) { attach(location, texture.get_id()); }
			void detach(GLenum location);

			GLenum get_status() const;
			bool is_complete() const;

		private:

			void attach(GLenum location, GLuint texture_id);
		};

	} // gl
	
} // bump