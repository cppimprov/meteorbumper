#pragma once

#include "bump_game_gamestate.hpp"

namespace bump
{
	
	namespace game
	{

		class app;
		
		gamestate do_game(app& app);
		gamestate do_start(app& app);
		
	} // game
	
} // bump
