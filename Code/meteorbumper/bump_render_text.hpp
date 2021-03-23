#pragma once

#include "bump_font_asset.hpp"
#include "bump_font_ft_context.hpp"
#include "bump_gl_texture.hpp"

#include <string>

namespace bump
{
	
	struct text_texture
	{
		glm::i32vec2 m_pos;
		gl::texture_2d m_texture;
	};

	text_texture render_text_to_gl_texture(font::ft_context const& ft_context, font::font_asset const& font, std::string const& utf8_text);
	text_texture render_text_outline_to_gl_texture(font::ft_context const& ft_context, font::font_asset const& font, std::string const& utf8_text, double outline_width);
	
} // bump
