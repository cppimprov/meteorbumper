#include "bump_game.hpp"

#include "bump_die.hpp"
#include "bump_game_app.hpp"
#include "bump_font.hpp"
#include "bump_log.hpp"
#include "bump_narrow_cast.hpp"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <stb_image.h>
#include <stb_image_write.h>

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
		
		// temp!
		void write_png(std::string const& filename, image<std::uint8_t> const& image)
		{
			die_if(image.channels() <= 0 || image.channels() > 4);

			auto const stride = sizeof(std::uint8_t) * image.channels() * image.size().x;
			if (!stbi_write_png(filename.c_str(), narrow_cast<int>(image.size().x), narrow_cast<int>(image.size().y), narrow_cast<int>(image.channels()), image.data(), narrow_cast<int>(stride)))
			{
				log_error("stbi_write_png() failed: " + std::string(stbi_failure_reason()));
				die();
			}
		}
		
		gamestate do_start(app& app)
		{
			auto ft_font = font::ft_font(app.m_ft_context.get_handle(), "data/fonts/BungeeShade-Regular.ttf");
			ft_font.set_pixel_size(32);

			auto hb_font = font::hb_font(ft_font.get_handle());

			auto text = std::string("\u1EB2test\u222B 1 2 \u1EB2 \ngy \u0604 3.4 ~@ q`¬|' Vo Te Av fi iiiiiiiiiii");

			auto hb_shaper = font::hb_shaper(HB_DIRECTION_LTR, HB_SCRIPT_LATIN, hb_language_from_string("en", -1));
			hb_shaper.shape(hb_font.get_handle(), text);

			auto glyphs = render_glyphs(ft_font, hb_font, hb_shaper);
			auto image = blit_glyphs(glyphs);

			write_png("test.png", image.m_image);


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
