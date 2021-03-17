
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

	// player controls / physics:
		// add rotational drag
		// try denser air?
		// better inertia tensor
		// click to fire.
	
	// asteroids!
		// physics component for each asteroid!
		// randomly deform sphere for each asteroid
		// more "normal" colors

	// camera:
		// spring attaching camera to fixed offset from player
		// which direction should the spring be? (same as fixed offset, or just backwards?)
	
	
	// level:
		// cylindrical force-field to keep player in range -> collidable
		// force field to push player downwards? (might not need it with fixed max angle...)
	
	// gameplay demo:
		// load and experiment with tunnel mechanics.
		// rotation around axis?

	// lighting
	// reflection / refraction materials?

	// intro music!



// sometime:

	// ui::cell
		// origin
		// rel_pos, size
		// abs_pos
