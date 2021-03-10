#pragma once

#include "bump_ptr_handle.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>

namespace bump
{
	
	namespace font
	{
		
		class hb_font : public ptr_handle<hb_font_t>
		{
		public:

			hb_font() = default;
			explicit hb_font(FT_Face ft_face);

			hb_font_extents_t get_extents(hb_direction_t direction) const;
		};
		
	} // font
	
} // bump