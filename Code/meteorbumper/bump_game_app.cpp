#include "bump_game_app.hpp"

namespace bump
{

	namespace game
	{

		app::app():
			m_context(),
			m_mixer_context(),
			m_window({ 1280, 720 }, "bump!", sdl::window::display_mode::WINDOWED)
		{

		}
	
	} // game

} // bump
