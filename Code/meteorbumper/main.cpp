
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

	// blender exporter
		// 

	// load the cube and render!

	// load ship model + render!

	// intro music!
	// add a skybox... distant stars / nebulae.



// sometime:

	// ui::cell
		// origin
		// rel_pos, size
		// abs_pos
