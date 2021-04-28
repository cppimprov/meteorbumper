#include "bump_log.hpp"

#include <iostream>

namespace bump
{
	
	void log_info(std::string const& message)
	{
		std::cerr << "INFO: " << message << "\n";
	}

	void log_error(std::string const& message)
	{
		std::cerr << "ERROR: " << message << "\n";
	}

} // bump
