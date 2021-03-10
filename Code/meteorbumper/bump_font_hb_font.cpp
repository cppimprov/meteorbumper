#include "bump_font_hb_font.hpp"

#include "bump_die.hpp"
#include "bump_log.hpp"

#include <hb-ft.h>

namespace bump
{
	
	namespace font
	{
		
		hb_font::hb_font(FT_Face ft_face)
		{
			auto handle = hb_ft_font_create(ft_face, nullptr);

			if (!handle)
			{
				log_error("hb_ft_font_create() failed.");
				die();
			}

			reset(handle, [] (hb_font_t* f) { hb_font_destroy(f); });
		}

		hb_font_extents_t hb_font::get_extents(hb_direction_t direction) const
		{
			die_if(!is_valid());

			auto extents = hb_font_extents_t();
			hb_font_get_extents_for_direction(get_handle(), direction, &extents);

			return extents;
		}
		
	} // font
	
} // bump