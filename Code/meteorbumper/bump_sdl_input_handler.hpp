#pragma once

#include "bump_input.hpp"
#include "bump_sdl_gamepad.hpp"

#include <SDL.h>

#include <vector>

namespace bump
{
	
	namespace sdl
	{

		class window;
		
		class input_handler
		{
		public:

			explicit input_handler(window& window);

			void poll_input(input::input_callbacks const& callbacks);

		private:

			window& m_window;
			std::vector<sdl::gamepad> m_gamepads;
		};
		
	} // sdl
	
} // bump