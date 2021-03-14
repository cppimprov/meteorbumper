#include "bump_sdl_window.hpp"

#include "bump_die.hpp"
#include "bump_log.hpp"

namespace bump
{
	
	namespace sdl
	{
		
		window::window(glm::i32vec2 size, std::string const& title, display_mode mode)
		{
			auto handle = SDL_CreateWindow(
				title.c_str(),
				SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
				size.x, size.y,
				get_sdl_window_flags(mode) | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
			
			if (!handle)
			{
				log_error("SDL_CreateWindow() failed: " + std::string(SDL_GetError()));
				die();
			}

			reset(handle, [] (SDL_Window* w) { SDL_DestroyWindow(w); });
		}

		void window::set_title(std::string const& title)
		{
			die_if(!is_valid());

			SDL_SetWindowTitle(get_handle(), title.c_str());
		}

		void window::set_size(glm::i32vec2 size)
		{
			die_if(!is_valid());
			die_if(size.x <= 0 || size.y <= 0);

			SDL_SetWindowSize(get_handle(), size.x, size.y);
		}

		void window::set_min_size(glm::i32vec2 min_size)
		{
			die_if(!is_valid());
			die_if(min_size.x <= 0 || min_size.y <= 0);

			SDL_SetWindowMinimumSize(get_handle(), min_size.x, min_size.y);
		}

		void window::set_display_mode(display_mode mode)
		{
			die_if(!is_valid());

			SDL_SetWindowFullscreen(get_handle(), get_sdl_window_flags(mode));
		}

		void window::set_cursor_mode(cursor_mode mode)
		{
			die_if(!is_valid());

			if (mode == cursor_mode::FREE)
			{
				SDL_SetRelativeMouseMode(SDL_FALSE);
				SDL_SetWindowGrab(get_handle(), SDL_FALSE);
			}
			else if (mode == cursor_mode::GRABBED)
			{
				SDL_SetRelativeMouseMode(SDL_FALSE);
				SDL_SetWindowGrab(get_handle(), SDL_TRUE);
			}
			else if (mode == cursor_mode::RELATIVE)
			{
				SDL_SetRelativeMouseMode(SDL_TRUE);
				SDL_SetWindowGrab(get_handle(), SDL_FALSE);
			}
		}

		glm::i32vec2 window::get_size() const
		{
			die_if(!is_valid());

			auto size = glm::i32vec2();
			SDL_GetWindowSize(get_handle(), &size.x, &size.y);
			return size;
		}

		glm::i32vec2 window::get_min_size() const
		{
			die_if(!is_valid());

			auto size = glm::i32vec2();
			SDL_GetWindowMinimumSize(get_handle(), &size.x, &size.y);
			return size;
		}

		window::display_mode window::get_display_mode() const
		{
			die_if(!is_valid());

			auto flags = SDL_GetWindowFlags(get_handle());
			
			return 
				(flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP ? display_mode::BORDERLESS_WINDOWED :
				(flags & SDL_WINDOW_FULLSCREEN) == SDL_WINDOW_FULLSCREEN_DESKTOP ? display_mode::FULLSCREEN :
				display_mode::WINDOWED;
		}

		window::cursor_mode window::get_cursor_mode() const
		{
			die_if(!is_valid());

			// note:
			// SDL lets us use relative and grabbed mode at the same time...
			// relative mode is more important, so we use that value.

			return 
				SDL_GetRelativeMouseMode() ? cursor_mode::RELATIVE :
				SDL_GetWindowGrab(get_handle()) ? cursor_mode::GRABBED :
				cursor_mode::FREE;
		}

		void window::swap_buffers() const
		{
			die_if(!is_valid());

			SDL_GL_SwapWindow(get_handle());
		}

		Uint32 window::get_sdl_window_flags(display_mode mode)
		{
			return (mode == display_mode::WINDOWED) ? 0 : (mode == display_mode::BORDERLESS_WINDOWED) ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN;
		}
		
	} // sdl
	
} // bump
