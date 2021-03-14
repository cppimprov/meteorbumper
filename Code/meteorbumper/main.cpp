
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

	// skybox!
		// use camera transform
		// set camera matrices once only
		// add debug camera controls
		// glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); ???
		// test cubemap mipmaps
	
	// player ship!
		// make class
		// load + render!

	// gameplay demo:
		// load and experiment with tunnel mechanics.
		// rotation around axis?

	// intro music!
	// add a skybox... distant stars / nebulae.



// sometime:

	// ui::cell
		// origin
		// rel_pos, size
		// abs_pos
