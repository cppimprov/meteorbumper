#pragma once

#include "bump_ptr_handle.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <cstdint>
#include <string>

namespace bump
{
	
	namespace font
	{

		class ft_font : public ptr_handle<FT_FaceRec_>
		{
		public:

			ft_font() = default;
			explicit ft_font(FT_Library library, std::string const &filename, std::size_t face_index = 0);

			void set_pixel_size(std::uint32_t pixels_per_em);

			// todo: other font metrics?
			std::int32_t get_line_height() const;
		};

	} // font
	
} // bump