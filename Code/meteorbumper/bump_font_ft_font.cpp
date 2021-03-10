#include "bump_font_ft_font.hpp"

#include "bump_die.hpp"
#include "bump_log.hpp"
#include "bump_narrow_cast.hpp"

namespace bump
{
	
	namespace font
	{
		
		ft_font::ft_font(FT_Library library, std::string const &filename, std::size_t face_index)
		{
			auto handle = FT_Face{ nullptr };

			if (auto err = FT_New_Face(library, filename.c_str(), narrow_cast<FT_Long>(face_index), &handle))
			{
				log_error("FT_New_Face() failed: " + std::string(FT_Error_String(err)));
				die();
			}

			auto deleter = [] (FT_Face f)
			{
				if (auto err = FT_Done_Face(f))
				{
					log_error("FT_Done_Face() failed: " + std::string(FT_Error_String(err)));
					die();
				}
			};

			reset(handle, std::move(deleter));
		}
		
		void ft_font::set_pixel_size(std::uint32_t pixels_per_em)
		{
			die_if(!is_valid());

			if (auto err = FT_Set_Pixel_Sizes(get_handle(), pixels_per_em, pixels_per_em))
			{
				log_error("FT_Set_Pixel_Sizes() failed: " + std::string(FT_Error_String(err)));
				die();
			}
		}
		
		std::int32_t ft_font::get_line_height() const
		{
			die_if(!is_valid());

			return (get_handle()->ascender - get_handle()->descender) / 64;
		}
		
	} // font
	
} // bump