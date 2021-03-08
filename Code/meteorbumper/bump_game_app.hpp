#pragma once

#include "bump_sdl_context.hpp"
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


			sdl::context m_context;
			sdl::mixer_context m_mixer_context;

			sdl::window m_window;
		};

	} // game

} // bump
