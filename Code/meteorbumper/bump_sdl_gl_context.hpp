#pragma once

#include "bump_sdl_object_handle.hpp"

namespace bump
{
	
	namespace sdl
	{

		class window;
		
		class gl_context : public object_handle<void>
		{
		public:

			explicit gl_context(window const& window);
		};
		
	} // sdl
	
} // bump