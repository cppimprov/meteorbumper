#pragma once

namespace bump
{
	
	namespace sdl
	{
		
		class mixer_chunk;
		class mixer_music;

		class mixer_context
		{
		public:
		
			mixer_context();
			~mixer_context();

			mixer_context(mixer_context const&) = delete;
			mixer_context& operator=(mixer_context const&) = delete;

			mixer_context(mixer_context&&) = delete;
			mixer_context& operator=(mixer_context&&) = delete;

			void play_once(mixer_chunk const& chunk) const;

			void play_music(mixer_music const& music, int fade_in_ms = 0) const;
			void set_music_volume(int volume) const;
			int get_music_volumne() const;
			void pause_music() const;
			void unpause_music() const;
			void stop_music(int fade_out_ms = 0) const;
		};
		
	} // sdl
	
} // bump
