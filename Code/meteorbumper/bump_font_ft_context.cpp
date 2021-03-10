#include "bump_font_ft_context.hpp"

#include "bump_die.hpp"
#include "bump_log.hpp"

namespace bump
{
	
	namespace font
	{
		
		ft_context::ft_context()
		{
			auto handle = FT_Library{ nullptr };

			if (auto err = FT_Init_FreeType(&handle))
			{
				log_error("FT_Init_FreeType() failed: " + std::string(FT_Error_String(err)));
				die();
			}

			auto deleter = [] (FT_Library l)
			{
				if (auto err = FT_Done_FreeType(l))
				{
					log_error("FT_Done_FreeType() failed: " + std::string(FT_Error_String(err)));
					die();
				}
			};

			reset(handle, std::move(deleter));
		}
		
	} // font
	
} // bump