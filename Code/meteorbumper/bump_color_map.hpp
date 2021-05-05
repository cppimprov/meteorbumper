#pragma once

#include <glm/glm.hpp>

#include <map>

namespace bump
{

	// TODO: use template functions and generalize to a "spline_map" or something!!!

	glm::vec4 catmull_rom(glm::vec4 p0, glm::vec4 p1, glm::vec4 p2, glm::vec4 p3, float t);
	glm::vec4 get_color_from_map(std::map<float, glm::vec4> const& color_map, float a);

	float catmull_rom(float p0, float p1, float p2, float p3, float t);
	float get_size_from_map(std::map<float, float> const& size_map, float a);
	
} // bump
