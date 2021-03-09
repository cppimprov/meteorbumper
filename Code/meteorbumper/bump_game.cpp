#include "bump_game.hpp"

#include "bump_die.hpp"
#include "bump_game_app.hpp"

#include <glm/glm.hpp>

#include <GL/glew.h>

#include <algorithm>
#include <chrono>
#include <deque>
#include <iterator>
#include <random>
#include <string>

namespace bump
{
	
	namespace game
	{
		
		gamestate do_start(app& app)
		{
			while (true)
			{
				// input
				{
					SDL_Event e;

					while (SDL_PollEvent(&e))
					{
						if (e.type == SDL_QUIT)
							return { };

						if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)
							return { };

						if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN && (e.key.keysym.mod & KMOD_LALT) != 0)
						{
							auto mode = app.m_window.get_display_mode();
							using display_mode = sdl::window::display_mode;
							
							if (mode != display_mode::FULLSCREEN)
								app.m_window.set_display_mode(mode == display_mode::BORDERLESS_WINDOWED ? display_mode::WINDOWED : display_mode::BORDERLESS_WINDOWED);
						}
					}
				}

				// update
				{
					// ...
				}

				// render
				{
					app.m_renderer.clear_color_buffers({ 1.f, 0.f, 0.f, 1.f });
					app.m_renderer.clear_depth_buffers();

					// ...

					app.m_window.swap_buffers();
				}
			}

			return { };
		}

	} // game
	
} // bump
