#include "bump_glew_context.hpp"

#include "bump_die.hpp"
#include "bump_log.hpp"

#include <GL/glew.h>

namespace bump
{
	
	glew_context::glew_context()
	{
		glewExperimental = GL_TRUE;
		auto status = glewInit();

		if (status != GLEW_OK)
		{
			log_error("glewInit() failed: " + std::string(reinterpret_cast<char const*>(glewGetErrorString(status))));
			die();
		}
		
		glGetError(); // clear any OpenGL errors in glewInit();
	}

} // bump