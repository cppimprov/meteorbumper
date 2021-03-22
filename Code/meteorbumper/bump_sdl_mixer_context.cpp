#include "bump_sdl_mixer_context.hpp"

#include "bump_die.hpp"
#include "bump_log.hpp"
#include "bump_sdl_mixer_chunk.hpp"
#include "bump_sdl_mixer_music.hpp"

#include <SDL_mixer.h>

#include <string>

namespace bump
{
	
	namespace sdl
	{
		
		mixer_context::mixer_context()
		{
			auto init_flags = 0;
			if (Mix_Init(init_flags) != init_flags)
			{
				log_error("Mix_Init() failed: " + std::string(Mix_GetError()));
				die();
			}

			if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) != 0)
			{
				log_error("Mix_OpenAudio() failed: " + std::string(Mix_GetError()));
				die();
			}
		}

		mixer_context::~mixer_context()
		{
			Mix_CloseAudio();
			Mix_Quit();
		}
		
		void mixer_context::play_once(mixer_chunk const& chunk) const
		{
			die_if(!chunk.is_valid());

			if (Mix_PlayChannel(-1, chunk.get_handle(), 0) == -1)
			{
				log_error("Mix_PlayChannel() failed: " + std::string(Mix_GetError()));
				// non-fatal error... for now!
			}
		}

		void mixer_context::play_music(mixer_music const& music, int fade_in_ms) const
		{
			die_if(!music.is_valid());
			die_if(fade_in_ms < 0);

			if (Mix_FadeInMusic(music.get_handle(), -1, fade_in_ms) == -1)
			{
				log_error("Mix_FadeInMusic() failed: " + std::string(Mix_GetError()));
				// non-fatal error... for now!
			}
		}

		void mixer_context::set_music_volume(int volume) const
		{
			die_if(volume < 0);
			die_if(volume > MIX_MAX_VOLUME);

			Mix_VolumeMusic(volume);
		}

		int mixer_context::get_music_volumne() const
		{
			return Mix_VolumeMusic(-1);
		}
		
		void mixer_context::pause_music() const
		{
			Mix_PauseMusic();
		}

		void mixer_context::unpause_music() const
		{
			Mix_ResumeMusic();
		}

		void mixer_context::stop_music(int fade_out_ms) const
		{
			die_if(fade_out_ms < 0);

			if (Mix_FadeOutMusic(fade_out_ms) != 1)
			{
				log_error("Mix_FadeOutMusic() failed: " + std::string(Mix_GetError()));
				// non-fatal error... for now!
			}
		}
		
	} // sdl
	
} // bump
