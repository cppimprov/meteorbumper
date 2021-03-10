
#include "bump_die.hpp"
#include "bump_game.hpp"
#include "bump_game_app.hpp"
#include "bump_gl_vertex_array.hpp"
#include "bump_gl_buffer.hpp"
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

#include <algorithm>
#include <functional>

namespace bump
{
	
	namespace gl
	{


	} // gl
	
} // bump


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

	// move sdl::object_handle out of sdl:: and rename to ptr_handle (?)
	// remove template deleter from gl::object_handle (not needed)
