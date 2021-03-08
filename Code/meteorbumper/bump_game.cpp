#include "bump_game.hpp"

#include "bump_die.hpp"
#include "bump_game_app.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <chrono>
#include <deque>
#include <iterator>
#include <random>
#include <string>

namespace bump
{
	
	namespace game
	{
		
		gamestate do_start(app& )
		{
			while (true)
			{
				// input
				{
					SDL_Event e;

					while (SDL_PollEvent(&e))
					{
						if (e.type == SDL_QUIT)
							return { };

						if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)
							return { };
					}
				}

				// update
				{
					// ...
				}

				// render
				{
					// ...
				}
			}

			return { };
		}

	} // game
	
} // bump
