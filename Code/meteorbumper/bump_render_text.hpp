#pragma once

#include "bump_font_asset.hpp"
#include "bump_gl_texture.hpp"

#include <string>

namespace bump
{
	
	struct text_texture
	{
		glm::i32vec2 m_pos;
		gl::texture_2d m_texture;
	};

	text_texture render_text_to_gl_texture(font::font_asset const& font, std::string const& utf8_text);
	
} // bump
