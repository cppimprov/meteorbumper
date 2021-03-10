#pragma once

#include "bump_image.hpp"

#include <cstdint>
#include <vector>

namespace bump
{
	
	namespace font
	{

		class ft_font;
		class hb_font;
		class hb_shaper;
		
		struct glyph_image
		{
			glm::i32vec2 m_pos = { 0, 0 }; // bottom left
			image<std::uint8_t> m_image; // note: the first pixel in the vector is the bottom left
		};
		
		std::vector<glyph_image> render_glyphs(ft_font const& ft_font, hb_font const& hb_font, hb_shaper const& hb_shaper);
		glyph_image blit_glyphs(std::vector<glyph_image> const& glyphs);

	} // font
	
} // bump
