#pragma once

#include "bump_sdl_object_handle.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace bump
{
	
	namespace font
	{
		
		class ft_context : public sdl::object_handle<FT_LibraryRec_>
		{
		public:

			ft_context();
		};
		
	} // font
	
} // bump