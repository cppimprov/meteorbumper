#pragma once

#include "bump_ptr_handle.hpp"

#include <SDL_mixer.h>

#include <string>

namespace bump
{
	
	namespace sdl
	{
		
		class mixer_chunk : public ptr_handle<Mix_Chunk>
		{
		public:

			mixer_chunk() = default;
			explicit mixer_chunk(std::string const& file);
		};
		
	} // sdl
	
} // bump