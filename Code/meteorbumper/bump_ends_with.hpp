#pragma once

#include <algorithm>
#include <string>

namespace bump
{
	
	inline bool ends_with(std::string const& source, std::string const& match)
	{
		if (match.empty())
			return false;

		if (source.length() < match.length())
			return false;

		return std::equal(source.rbegin(), source.rbegin() + match.length(), match.rbegin());
	}
	
} // bump
