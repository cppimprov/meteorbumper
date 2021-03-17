#pragma once

#include "bump_die.hpp"
#include "bump_ptr_handle.hpp"

#include <SDL.h>

namespace bump
{
	
	namespace sdl
	{
		
		class gamepad : public ptr_handle<SDL_GameController>
		{
		public:

			explicit gamepad(int joystick_index);

			SDL_JoystickID get_joystick_id() const { die_if(!is_valid()); return SDL_JoystickInstanceID(m_joystick); }

		private:

			SDL_Joystick* m_joystick;
		};
		
	} // sdl
	
} // bump