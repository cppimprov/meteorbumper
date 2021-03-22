#pragma once

#include "bump_ptr_handle.hpp"

#include <SDL_mixer.h>

#include <string>

namespace bump
{
	
	namespace sdl
	{
		
		class mixer_music : public ptr_handle<Mix_Music>
		{
		public:

			mixer_music() = default;
			explicit mixer_music(std::string const& file);
		};
		
	} // sdl
	
} // bump
