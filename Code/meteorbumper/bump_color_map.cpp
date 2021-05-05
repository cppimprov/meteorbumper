#include "bump_color_map.hpp"

#include "bump_die.hpp"

#include <iterator>

namespace bump
{
	
	glm::vec4 catmull_rom(glm::vec4 p0, glm::vec4 p1, glm::vec4 p2, glm::vec4 p3, float t)
	{
		return 0.5f * (
			(2.0f * p1) +
			(-p0 + p2) * t +
			(2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t * t +
			(-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t * t * t);
	}
	
	glm::vec4 get_color_from_map(std::map<float, glm::vec4> const& color_map, float a)
	{
		if (color_map.empty())
			return glm::vec4(1.f);
		
		if (color_map.size() == 1)
			return color_map.begin()->second;
		
		auto upper = color_map.upper_bound(a);
		
		if (upper == color_map.end())
			return std::prev(color_map.end())->second;
		
		auto p2 = upper;
		auto p3 = (std::next(p2) == color_map.end() ? p2 : std::next(p2));
		
		if (upper == color_map.begin())
			return upper->second;
		
		auto p1 = std::prev(upper);
		auto p0 = (p1 == color_map.begin() ? p1 : std::prev(p1));
		
		die_if(a < p1->first || a >= p2->first);
		auto t = (a - p1->first) / (p2->first - p1->first);
		
		return catmull_rom(p0->second, p1->second, p2->second, p3->second, t);
	}
	
	float catmull_rom(float p0, float p1, float p2, float p3, float t)
	{
		return 0.5f * (
			(2.0f * p1) +
			(-p0 + p2) * t +
			(2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t * t +
			(-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t * t * t);
	}
	
	float get_size_from_map(std::map<float, float> const& size_map, float a)
	{
		if (size_map.empty())
			return 1.f;
		
		if (size_map.size() == 1)
			return size_map.begin()->second;
		
		auto upper = size_map.upper_bound(a);
		
		if (upper == size_map.end())
			return std::prev(size_map.end())->second;
		
		auto p2 = upper;
		auto p3 = (std::next(p2) == size_map.end() ? p2 : std::next(p2));
		
		if (upper == size_map.begin())
			return upper->second;
		
		auto p1 = std::prev(upper);
		auto p0 = (p1 == size_map.begin() ? p1 : std::prev(p1));
		
		die_if(a < p1->first || a >= p2->first);
		auto t = (a - p1->first) / (p2->first - p1->first);
		
		return catmull_rom(p0->second, p1->second, p2->second, p3->second, t);
	}
	
	
} // bump
