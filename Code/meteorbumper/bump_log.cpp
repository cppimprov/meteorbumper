#include "bump_log.hpp"

#include <iostream>

namespace bump
{
	
	void log_info(std::string const& message)
	{
		std::cerr << "INFO: " << message << std::endl;
	}

	void log_error(std::string const& message)
	{
		std::cerr << "ERROR: " << message << std::endl;
	}

} // bump
