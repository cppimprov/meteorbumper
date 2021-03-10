
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

	// move some stuff into the renderer!
		// blend mode (similar to sdl)
		// use_shader();
		// clear_shader();
		// set_texture();
		// clear_texture();
		// set_uniform();

		// draw call?




	// ui::cell
		// origin
		// rel_pos, size
		// abs_pos
