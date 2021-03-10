#pragma once

#include "bump_ptr_handle.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace bump
{
	
	namespace font
	{
		
		class ft_context : public ptr_handle<FT_LibraryRec_>
		{
		public:

			ft_context();
		};
		
	} // font
	
} // bump