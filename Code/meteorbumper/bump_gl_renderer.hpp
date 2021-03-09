#pragma once

#include <glm/glm.hpp>

namespace bump
{
	
	namespace gl
	{
		
		class renderer
		{
		public:

			void clear_color_buffers(glm::f32vec4 color = { 0.f, 0.f, 0.f, 0.f });
			void clear_depth_buffers(float depth = 1.f);
		};
		
	} // gl
	
} // bump
