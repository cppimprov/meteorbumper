#include "bump_sdl_context.hpp"

#include "bump_die.hpp"
#include "bump_log.hpp"

#include <SDL.h>

#include <string>

namespace bump
{
	
	namespace sdl
	{
		
		context::context()
		{
			if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
			{
				log_error("SDL_Init() failed: " + std::string(SDL_GetError()));
				die();
			}
		}

		context::~context()
		{
			SDL_Quit();
		}
		
	} // sdl
	
} // bump