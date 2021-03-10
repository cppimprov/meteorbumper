#pragma once

#include "bump_font_ft_font.hpp"
#include "bump_font_hb_font.hpp"
#include "bump_hash.hpp"

#include <cstdint>
#include <string>

namespace bump
{
	
	namespace font
	{
		
		class font_asset
		{
		public:

			font::ft_font m_ft_font;
			font::hb_font m_hb_font;
		};
		
	} // font
	
} // bump