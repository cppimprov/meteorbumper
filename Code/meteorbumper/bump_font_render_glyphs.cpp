#include "bump_font_render_glyphs.hpp"

#include "bump_font_conversions.hpp"
#include "bump_font_ft_context.hpp"
#include "bump_font_ft_font.hpp"
#include "bump_font_hb_font.hpp"
#include "bump_font_hb_shaper.hpp"
#include "bump_log.hpp"
#include "bump_narrow_cast.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_ERRORS_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_BBOX_H
#include FT_STROKER_H

#include <glm/glm.hpp>

#include <algorithm>
#include <cmath>

namespace bump
{
	
	namespace font
	{
		
		image<std::uint8_t> ft_bitmap_to_image(FT_Bitmap const& bitmap)
		{
			die_if(bitmap.pixel_mode != FT_PIXEL_MODE_GRAY);

			auto out = image<std::uint8_t>(1, { bitmap.width, bitmap.rows });

			auto const src = bitmap.buffer;
			auto const dst = out.data();

			for (auto y = std::size_t{ 0 }; y != bitmap.rows; ++y)
			{
				for (auto x = std::size_t{ 0 }; x != bitmap.width; ++x)
				{
					auto const src_index = ((bitmap.rows - 1) - y) * bitmap.pitch + x; // check: pitch can be negative, does this still work?
					auto const dst_index = y * out.size().x + x;
					dst[dst_index] = src[src_index];
				}
			}

			return out;
		}

		void blit_image(image<std::uint8_t>& dst, glm::size2 dst_pos, image<std::uint8_t> const& src)
		{
			die_if(dst.channels() != src.channels());
			die_if(glm::any(glm::greaterThan(dst_pos + src.size(), dst.size())));

			auto const src_pixels = src.data();
			auto const dst_pixels = dst.data();

			for (auto y = std::size_t{ 0 }; y != src.size().y; ++y)
			{
				for (auto x = std::size_t{ 0 }; x != src.size().x; ++x)
				{
					auto const src_index = (y * src.size().x + x) * src.channels();
					auto const dst_index = ((dst_pos.y + y) * dst.size().x + (dst_pos.x + x)) * dst.channels();

					for (auto c = std::size_t{ 0 }; c != src.channels(); ++c)
						dst_pixels[dst_index + c] = src_pixels[src_index + c];
				}
			}
		}

		glyph_image blit_glyphs(std::vector<glyph_image> const& glyphs)
		{
			if (glyphs.empty())
				return { { 0, 0 }, image<std::uint8_t>() };

			auto min = glm::i32vec2(std::numeric_limits<std::int32_t>::max());
			auto max = glm::i32vec2(std::numeric_limits<std::int32_t>::lowest());

			for (auto const& g : glyphs)
			{
				min = glm::min(min, g.m_pos);
				max = glm::max(max, g.m_pos + narrow_cast<glm::i32vec2>(g.m_image.size()));
			}

			auto image_size = narrow_cast<glm::size2>(max - min);
			auto out = glyph_image{ min, image<std::uint8_t>(1, image_size) };

			for (auto const& g : glyphs)
				blit_image(out.m_image, narrow_cast<glm::size2>(g.m_pos - out.m_pos), g.m_image);

			return out;
		}

		std::vector<glyph_image> render_glyphs(ft_context const& ft_context, ft_font const& ft_font, hb_font const& , hb_shaper const& hb_shaper, std::optional<double> stroke_width)
		{
			auto const glyph_info = hb_shaper.get_glyph_info();
			auto const glyph_positions = hb_shaper.get_glyph_positions();
			auto const ft_face = ft_font.get_handle();

			die_if(glyph_info.size() != glyph_positions.size());

			if (glyph_info.empty()) // nothing to do!
				return {};

			auto out = std::vector<glyph_image>();
			out.reserve(glyph_info.size());
			
			auto const is_horizontal = HB_DIRECTION_IS_HORIZONTAL(hb_shaper.get_direction());
			auto const font_max_descender = font_units_to_pixels(ft_face->bbox.yMin, ft_face->units_per_EM, ft_face->size->metrics.y_ppem);
			auto const font_max_width =  font_units_to_pixels(ft_face->bbox.xMax - ft_face->bbox.xMin, ft_face->units_per_EM, ft_face->size->metrics.x_ppem);

			auto const baseline = glm::f64vec2
			{
				is_horizontal ? 0.0 : std::ceil(0.5 * font_max_width),  // center-line
				is_horizontal ? std::ceil(-(font_max_descender)) : 0.0 // baseline
			};

			auto pen = glm::f64vec2(0.0);

			auto stroker = (FT_Stroker)nullptr;

			if (stroke_width)
			{
				if (auto err = FT_Stroker_New(ft_context.get_handle(), &stroker))
				{
					log_error("FT_Stroker_New() failed: " + std::string(FT_Error_String(err)));
					die();
				}

				FT_Stroker_Set(stroker, (FT_Fixed)(stroke_width.value() * 64.0), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
			}

			for (auto i = std::size_t{ 0 }; i != glyph_info.size(); ++i)
			{
				auto const glyph_index = glyph_info.at(i).codepoint;
				auto const& glyph_position = glyph_positions.at(i);

				auto const offset = glm::f64vec2{ glyph_position.x_offset, glyph_position.y_offset } / 64.0;
				auto const advance = glm::f64vec2{ glyph_position.x_advance, glyph_position.y_advance } / 64.0;

				if (auto err = FT_Load_Glyph(ft_face, glyph_index, FT_LOAD_DEFAULT))
				{
					log_error("FT_Load_Glyph() failed: " + std::string(FT_Error_String(err)));
					die();
				}

				auto glyph = (FT_Glyph)nullptr;

				if (auto err = FT_Get_Glyph(ft_face->glyph, &glyph))
				{
					log_error("FT_Get_Glyph() failed: " + std::string(FT_Error_String(err)));
					die();
				}

				if (stroke_width)
				{
					if (auto err = FT_Glyph_Stroke(&glyph, stroker, true))
					{
						log_error("FT_Glyph_Stroke() failed: " + std::string(FT_Error_String(err)));
						die();
					}
				}

				auto const p = (baseline + pen + offset) * 64.0;
				auto origin = FT_Vector{ (FT_Pos)p.x, (FT_Pos)p.y };

				if (auto err = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_LIGHT, &origin, true))
				{
					log_error("FT_Glyph_To_Bitmap() failed: " + std::string(FT_Error_String(err)));
					die();
				}

				auto bitmap = (FT_BitmapGlyph)glyph;

				if (bitmap->bitmap.width != 0 && bitmap->bitmap.rows != 0)
				{
					auto const left = bitmap->left;
					auto const bottom = (bitmap->top - 1) - (narrow_cast<std::int32_t>(bitmap->bitmap.rows) - 1);
					auto image = ft_bitmap_to_image(bitmap->bitmap);
					
					out.push_back({ { left, bottom }, std::move(image) });
				}
				
				FT_Done_Glyph(glyph);

				pen += advance;
			}

			if (stroke_width)
				FT_Stroker_Done(stroker);

			return out;
		}
		
	} // font
	
} // bump