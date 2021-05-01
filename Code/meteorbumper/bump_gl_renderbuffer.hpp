#pragma once

#include "bump_gl_object_handle.hpp"

#include <GL/glew.h>

#include <glm/glm.hpp>

namespace bump
{
	
	namespace gl
	{
		
		class renderbuffer : public object_handle
		{
		public:

			renderbuffer();

			void set_data(glm::ivec2 size, GLenum format);

			glm::ivec2 get_size() const;
			GLenum get_format() const;
		};
		
	} // gl
	
} // bump