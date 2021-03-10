#include "bump_render_text.hpp"

#include "bump_font.hpp"
#include "bump_gl_error.hpp"
#include "bump_narrow_cast.hpp"

#include <GL/glew.h>

namespace bump
{

	namespace
	{

		gl::texture_2d text_image_to_gl_texture(image<std::uint8_t> const& image)
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // :(

			auto texture = gl::texture_2d();
			texture.set_data(narrow_cast<glm::i32vec2>(image.size()), GL_R8, gl::make_texture_data_source(GL_RED, image.data()));
			texture.set_wrap_mode(GL_CLAMP_TO_EDGE);

			gl::die_if_error();
			return texture;
		}

	} // unnamed
	
	text_texture render_text_to_gl_texture(font::font_asset const& font, std::string const& utf8_text)
	{
		auto hb_shaper = font::hb_shaper(HB_DIRECTION_LTR, HB_SCRIPT_LATIN, hb_language_from_string("en", -1));
		hb_shaper.shape(font.m_hb_font.get_handle(), utf8_text);

		auto glyphs = render_glyphs(font.m_ft_font, font.m_hb_font, hb_shaper);
		auto image = blit_glyphs(glyphs);
		
		return { image.m_pos, text_image_to_gl_texture(image.m_image) };
	}
	
} // bump
