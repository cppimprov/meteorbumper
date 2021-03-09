#include "bump_gl_error.hpp"

#include "bump_die.hpp"
#include "bump_log.hpp"

#include <GL/glew.h>

#include <map>

namespace bump
{
	
	namespace gl
	{
		
		void die_if_error()
		{
			auto code = glGetError();

			if (code == GL_NO_ERROR)
				return;

			auto const error_strings = std::map<GLenum, char const*>
			{
				{ GL_INVALID_ENUM, "GL_INVALID_ENUM" },
				{ GL_INVALID_VALUE, "GL_INVALID_VALUE" },
				{ GL_INVALID_OPERATION, "GL_INVALID_OPERATION" },
				{ GL_INVALID_FRAMEBUFFER_OPERATION, "GL_INVALID_FRAMEBUFFER_OPERATION" },
				{ GL_OUT_OF_MEMORY, "GL_OUT_OF_MEMORY" },
				{ GL_STACK_UNDERFLOW, "GL_STACK_UNDERFLOW" },
				{ GL_STACK_OVERFLOW, "GL_STACK_OVERFLOW" },
			};

			auto entry = error_strings.find(code);
			auto str = (entry != error_strings.end() ? entry->second : "[unknown value!]");

			log_error("glGetError() returned " + std::string(str));
			die();
		}
		
	} // gl
	
} // bump