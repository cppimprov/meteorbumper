#pragma once

#include "bump_sdl_object_handle.hpp"

#include <glm/glm.hpp>
#include <SDL.h>

#include <string>

namespace bump
{
	
	namespace sdl
	{
		
		
		class window : public object_handle<SDL_Window>
		{
		public:

			enum class display_mode { WINDOWED, BORDERLESS_WINDOWED, FULLSCREEN };

			window() = default;
			explicit window(glm::i32vec2 size, std::string const& title, display_mode mode);

			void set_title(std::string const& title);
			void set_size(glm::i32vec2 size);
			void set_min_size(glm::i32vec2 min_size);
			void set_display_mode(display_mode mode);

			glm::i32vec2 get_size() const;
			glm::i32vec2 get_min_size() const;
			display_mode get_display_mode() const;

			void swap_buffers() const;

		private:

			static Uint32 get_sdl_window_flags(display_mode mode);
		};
		
	} // sdl
	
} // bump