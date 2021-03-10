#include "bump_game_app.hpp"

namespace bump
{

	namespace game
	{

		namespace
		{

			auto const fonts = std::vector<font_metadata>
			{
				{ "press_start", "Bungee-Regular.ttf", 64 },
			};

			auto const sounds = std::vector<sound_metadata>{ };

			auto const shaders = std::vector<shader_metadata>
			{
				{ "press_start", { "text_quad.vert", "text_quad.frag" } },
			};

		} // unnamed

		app::app():
			m_sdl_context(),
			m_mixer_context(),
			m_ft_context(),
			m_window({ 1280, 720 }, "bump!", sdl::window::display_mode::WINDOWED),
			m_gl_context(m_window),
			m_glew_context(),
			m_renderer(),
			m_assets(load_assets(*this, fonts, sounds, shaders))
		{
			m_window.set_min_size({ 640, 360 });
		}
	
	} // game

} // bump
