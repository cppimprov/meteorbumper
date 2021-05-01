#include "bump_sdl_gl_context.hpp"

#include "bump_die.hpp"
#include "bump_log.hpp"
#include "bump_sdl_window.hpp"

namespace bump
{
	
	namespace sdl
	{
		
		gl_context::gl_context(window const& window)
		{
			SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
			SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);

			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

			auto context = SDL_GL_CreateContext(window.get_handle());

			if (!context)
			{
				log_error("SDL_GL_CreateContext() failed: " + std::string(SDL_GetError()));
				die();
			}

			reset(context, [] (SDL_GLContext c) { SDL_GL_DeleteContext(c); });
		}
		
	} // sdl
	
} // bump