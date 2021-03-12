
#include "bump_game.hpp"
#include "bump_game_app.hpp"
#include "bump_log.hpp"

#include <SDL.h>
#include <cstdlib>

int main(int , char* [])
{
	using namespace bump;

	{
		auto app = game::app();
		game::run_state({ game::do_start }, app);
	}

	log_info("done!");

	return EXIT_SUCCESS;
}

// todo next:

	// add set_depth_test to renderer { LESS, LESS_EQUAL, GREATER, GREATER_EQUAL, EQUAL, NOT_EQUAL, ALWAYS, NEVER }
	// use depth mask to prevent depth write

	// renderer constructor should set it to always (?)

	// load main ship model + render!

	// intro music!
	// add a skybox... distant stars / nebulae.



// sometime:

	// ui::cell
		// origin
		// rel_pos, size
		// abs_pos
