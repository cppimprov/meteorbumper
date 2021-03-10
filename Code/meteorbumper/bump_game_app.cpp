#include "bump_game_app.hpp"

namespace bump
{

	namespace game
	{

		namespace
		{

			const auto font_assets = std::vector<font::font_asset_key>{ { "BungeeShade-Regular", 32 } };
			const auto sound_assets = std::vector<std::string>{ };

		} // unnamed

		app::app():
			m_sdl_context(),
			m_mixer_context(),
			m_ft_context(),
			m_window({ 1280, 720 }, "bump!", sdl::window::display_mode::WINDOWED),
			m_gl_context(m_window),
			m_glew_context(),
			m_renderer(),
			m_assets(load_assets(*this, font_assets, sound_assets))
		{
			m_window.set_min_size({ 640, 360 });
		}
	
	} // game

} // bump
