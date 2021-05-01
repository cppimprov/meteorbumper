#include "bump_gl_renderbuffer.hpp"

#include "bump_die.hpp"
#include "bump_gl_error.hpp"

namespace bump
{
	
	namespace gl
	{
		
		renderbuffer::renderbuffer()
		{
			auto id = GLuint{ 0 };
			glGenRenderbuffers(1, &id);
			die_if(!id);

			reset(id, [] (GLuint id) { glDeleteRenderbuffers(1, &id); });
		}


		void renderbuffer::set_data(glm::ivec2 size, GLenum format)
		{
			die_if(!is_valid());

			glBindRenderbuffer(GL_RENDERBUFFER, get_id());
			glRenderbufferStorage(GL_RENDERBUFFER, format, size.x, size.y);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			die_if_error();
		}

		glm::ivec2 renderbuffer::get_size() const
		{
			die_if(!is_valid());

			auto size = glm::ivec2();

			glBindRenderbuffer(GL_RENDERBUFFER, get_id());
			glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &size.x);
			glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &size.y);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			
			die_if_error();

			return size;
		}

		GLenum renderbuffer::get_format() const
		{
			die_if(!is_valid());

			auto format = GLint{ 0 };

			glBindRenderbuffer(GL_RENDERBUFFER, get_id());
			glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT, &format);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			
			die_if_error();

			return static_cast<GLenum>(format);
		}
		
	} // gl
	
} // bump