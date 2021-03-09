#include "bump_gl_renderer.hpp"

#include "bump_die.hpp"

#include <GL/glew.h>

namespace bump
{
	
	namespace gl
	{
		
		void renderer::clear_color_buffers(glm::f32vec4 color)
		{
			glClearColor(color.x, color.y, color.z, color.w);
			glClear(GL_COLOR_BUFFER_BIT);
			
			die_if(glGetError() != GL_NO_ERROR);
		}

		void renderer::clear_depth_buffers(float depth)
		{
			glClearDepth(depth);
			glClear(GL_DEPTH_BUFFER_BIT);

			die_if(glGetError() != GL_NO_ERROR);
		}
		
	} // gl
	
} // bump