#include "bump_game_app.hpp"

namespace bump
{

	namespace game
	{

		app::app():
			m_sdl_context(),
			m_mixer_context(),
			m_window({ 1280, 720 }, "bump!", sdl::window::display_mode::WINDOWED),
			m_gl_context(m_window),
			m_glew_context(),
			m_renderer()
		{
			m_window.set_min_size({ 640, 360 });
		}
	
	} // game

} // bump
