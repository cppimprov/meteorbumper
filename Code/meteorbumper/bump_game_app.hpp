#pragma once

#include "bump_gl_renderer.hpp"
#include "bump_glew_context.hpp"
#include "bump_sdl_context.hpp"
#include "bump_sdl_gl_context.hpp"
#include "bump_sdl_mixer_context.hpp"
#include "bump_sdl_window.hpp"

namespace bump
{

	namespace game
	{

		class app
		{
		public:
			
			app();

			app(app const&) = delete;
			app& operator=(app const&) = delete;
			
			app(app&&) = delete;
			app& operator=(app&&) = delete;


			sdl::context m_sdl_context;
			sdl::mixer_context m_mixer_context;

			sdl::window m_window;
			sdl::gl_context m_gl_context;
			glew_context m_glew_context;

			gl::renderer m_renderer;
		};

	} // game

} // bump
