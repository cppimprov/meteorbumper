#include "bump_sdl_mixer_music.hpp"

#include "bump_die.hpp"
#include "bump_log.hpp"

namespace bump
{
	
	namespace sdl
	{
		
		mixer_music::mixer_music(std::string const& file)
		{
			auto handle = Mix_LoadMUS(file.c_str());

			if (!handle)
			{
				log_error("Mix_LoadMUS() failed: " + std::string(Mix_GetError()));
				die();
			}

			reset(handle, [] (Mix_Music* m) { Mix_FreeMusic(m); });
		}
		
	} // sdl
	
} // bump