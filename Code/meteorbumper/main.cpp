
#include "bump_game.hpp"
#include "bump_game_app.hpp"
#include "bump_log.hpp"

#include <SDL.h>
#include <SDL_Mixer.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <json.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>

#include <GL/glew.h>

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

// todo next: bring over sdl framework code...
	// make window resizeable, able to switch to fullscreen...
