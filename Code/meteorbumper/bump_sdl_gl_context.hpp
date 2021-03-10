#pragma once

#include "bump_ptr_handle.hpp"

namespace bump
{
	
	namespace sdl
	{

		class window;
		
		class gl_context : public ptr_handle<void>
		{
		public:

			explicit gl_context(window const& window);
		};
		
	} // sdl
	
} // bump