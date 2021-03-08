#include "bump_die.hpp"

namespace bump
{
	
	void die()
	{
		__debugbreak();
	}

	void die_if(bool condition)
	{
		if (condition)
			die();
	}
	
} // bump
