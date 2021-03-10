#include "bump_game_assets.hpp"

#include "bump_die.hpp"
#include "bump_game_app.hpp"
#include "bump_log.hpp"

namespace bump
{
	
	namespace game
	{
		
		assets load_assets(app& app, 
			std::vector<font::font_asset_key> const& fonts,
			std::vector<std::string> const& sounds)
		{
			auto out = assets();

			// load fonts:
			{
				for (auto const& key : fonts)
				{
					auto ft_font = font::ft_font(app.m_ft_context.get_handle(), "data/fonts/" + key.m_name + ".ttf");
					ft_font.set_pixel_size(key.m_size);

					auto hb_font = font::hb_font(ft_font.get_handle());

					if (!out.m_fonts.insert({ key, font::font_asset{ std::move(ft_font), std::move(hb_font) } }).second)
					{
						log_error("load_assets(): duplicate font id: { " + key.m_name + ", " + std::to_string(key.m_size) + " }");
						die();
					}
				}
			}

			// load sounds:
			{
				for (auto const& name : sounds)
				{
					auto chunk = sdl::mixer_chunk("data/sounds/" + name + ".wav");

					if (!out.m_sounds.insert({ name, std::move(chunk) }).second)
					{
						log_error("load_assets(): duplicate sound id: " + name);
						die(); 
					}
				}
			}

			return out;
		}
		
	} // game
	
} // bump