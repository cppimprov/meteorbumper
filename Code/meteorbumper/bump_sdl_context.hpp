#pragma once

namespace bump
{
	
	namespace sdl
	{
		
		class context
		{
		public:
			
			context();
			~context();

			context(context const&) = delete;
			context& operator=(context const&) = delete;

			context(context&&) = delete;
			context& operator=(context&&) = delete;
		};

	} // sdl
	
} // bump