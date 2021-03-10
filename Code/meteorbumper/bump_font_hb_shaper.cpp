#include "bump_font_hb_shaper.hpp"

#include "bump_die.hpp"
#include "bump_log.hpp"
#include "bump_narrow_cast.hpp"

namespace bump
{
	
	namespace font
	{
		
		hb_shaper::hb_shaper(hb_direction_t direction, hb_script_t script, hb_language_t language)
		{
			auto handle = hb_buffer_create();

			if (!handle)
			{
				log_error("hb_buffer_create() failed");
				die();
			}

			reset(handle, [] (hb_buffer_t* b) { hb_buffer_destroy(b); });
			
			set_direction(direction);
			set_script(script);
			set_language(language);
		}
		
		void hb_shaper::set_direction(hb_direction_t direction)
		{
			die_if(!is_valid());
			hb_buffer_set_direction(get_handle(), direction);
		}

		hb_direction_t hb_shaper::get_direction() const
		{
			die_if(!is_valid());
			return hb_buffer_get_direction(get_handle());
		}

		void hb_shaper::set_script(hb_script_t script)
		{
			die_if(!is_valid());
			hb_buffer_set_script(get_handle(), script);
		}

		hb_script_t hb_shaper::get_script() const
		{
			die_if(!is_valid());
			return hb_buffer_get_script(get_handle());
		}
		
		void hb_shaper::set_language(hb_language_t language)
		{
			die_if(!is_valid());
			hb_buffer_set_language(get_handle(), language);
		}
		
		hb_language_t hb_shaper::get_language() const
		{
			die_if(!is_valid());
			return hb_buffer_get_language(get_handle());
		}

		void hb_shaper::shape(hb_font_t* harfbuzz_font, std::string const& utf8_str, span<hb_feature_t> features)
		{
			hb_buffer_add_utf8(get_handle(), utf8_str.c_str(), narrow_cast<int>(utf8_str.size()), 0, narrow_cast<int>(utf8_str.size()));

			hb_shape(harfbuzz_font, get_handle(), features.data(), narrow_cast<unsigned int>(features.size()));
		}

		span<hb_glyph_info_t> hb_shaper::get_glyph_info() const
		{
			die_if(!is_valid());

			auto length = 0u;
			auto data = hb_buffer_get_glyph_infos(get_handle(), &length);

			return { data, length };
		}

		span<hb_glyph_position_t> hb_shaper::get_glyph_positions() const
		{
			die_if(!is_valid());

			auto length = 0u;
			auto data = hb_buffer_get_glyph_positions(get_handle(), &length);

			return { data, length };
		}
		
	} // font
	
} // bump