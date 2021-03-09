#include "bump_gl_framebuffer.hpp"

#include "bump_die.hpp"
#include "bump_gl_error.hpp"

namespace bump
{
	
	namespace gl
	{
		
		framebuffer::framebuffer()
		{
			auto id = GLuint{ 0 };
			glGenFramebuffers(1, &id);
			die_if(!id);

			reset(id, [] (GLuint id) { glDeleteFramebuffers(1, &id); });
		}

		void framebuffer::attach(GLenum location, GLuint texture_id)
		{
			die_if(!is_valid());

			// note: doesn't work for cubemap textures (they need glFramebufferTexture2D with a special target)

			glBindFramebuffer(GL_FRAMEBUFFER, get_id());
			glFramebufferTexture(GL_FRAMEBUFFER, location, texture_id, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			die_if_error();
		}

		void framebuffer::detach(GLenum location)
		{
			die_if(!is_valid());

			glBindFramebuffer(GL_FRAMEBUFFER, get_id());
			glFramebufferTexture(GL_FRAMEBUFFER, location, 0, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			
			die_if_error();
		}
		
		GLenum framebuffer::get_status() const
		{
			die_if(!is_valid());

			glBindFramebuffer(GL_FRAMEBUFFER, get_id());
			auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			
			die_if_error();
			return status;
		}

		bool framebuffer::is_complete() const
		{
			die_if(!is_valid());
			
			return (get_status() == GL_FRAMEBUFFER_COMPLETE);
		}
		
	} // gl
	
} // bump
