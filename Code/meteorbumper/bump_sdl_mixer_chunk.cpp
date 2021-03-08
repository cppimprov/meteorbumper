#include "bump_sdl_mixer_chunk.hpp"

#include "bump_die.hpp"
#include "bump_log.hpp"

namespace bump
{
	
	namespace sdl
	{

		mixer_chunk::mixer_chunk(std::string const& file)
		{
			auto handle = Mix_LoadWAV(file.c_str());

			if (!handle)
			{
				log_error("Mix_LoadWAV() failed: " + std::string(Mix_GetError()));
				die();
			}

			reset(handle, [] (Mix_Chunk* c) { Mix_FreeChunk(c); });
		}

	} // sdl
	
} // bump
