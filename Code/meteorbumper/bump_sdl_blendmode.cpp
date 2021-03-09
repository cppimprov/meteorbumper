#include "bump_sdl_blendmode.hpp"

#include "bump_die.hpp"

namespace bump
{
	
	namespace sdl
	{
		
		SDL_BlendMode get_sdl_blendmode(blend_mode mode)
		{
			if (mode == blend_mode::NONE) return SDL_BLENDMODE_NONE;
			if (mode == blend_mode::BLEND) return SDL_BLENDMODE_BLEND;
			if (mode == blend_mode::ADDITIVE) return SDL_BLENDMODE_ADD;
			if (mode == blend_mode::MODULATE) return SDL_BLENDMODE_MOD;

			die();
			return SDL_BLENDMODE_NONE;
		}
		
	} // sdl
	
} // bump