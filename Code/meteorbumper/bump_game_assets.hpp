#pragma once

#include "bump_font_asset.hpp"
#include "bump_sdl_mixer_chunk.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace bump
{
	
	namespace game
	{

		class app;

		class assets
		{
		public:

			std::unordered_map<font::font_asset_key, font::font_asset, font::font_asset_key_hash> m_fonts;
			std::unordered_map<std::string, sdl::mixer_chunk> m_sounds;
		};

		assets load_assets(app& app, 
			std::vector<font::font_asset_key> const& fonts,
			std::vector<std::string> const& sounds
			// ...
		);
		
	} // game
	
} // bump