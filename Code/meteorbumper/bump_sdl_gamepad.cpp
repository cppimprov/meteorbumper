#include "bump_sdl_gamepad.hpp"

#include "bump_die.hpp"
#include "bump_log.hpp"

namespace bump
{
	
	namespace sdl
	{
		
		gamepad::gamepad(int joystick_index):
			m_joystick(nullptr)
		{
			auto handle = SDL_GameControllerOpen(joystick_index);

			if (!handle)
			{
				log_error("SDL_GameControllerOpen() failed: " + std::string(SDL_GetError()));
				die();
			}

			m_joystick = SDL_GameControllerGetJoystick(handle);

			if (!m_joystick)
			{
				log_error("SDL_GameControllerGetJoystick() failed: " + std::string(SDL_GetError()));
				die();
			}

			// note: joystick mustn't be freed
			reset(handle, [&] (SDL_GameController* c) { SDL_GameControllerClose(c); m_joystick = nullptr; });
		}
		
	} // sdl
	
} // bump