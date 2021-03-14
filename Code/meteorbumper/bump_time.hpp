#pragma once

#include <chrono>

namespace bump
{
	
	using high_res_clock_t = std::chrono::high_resolution_clock;
	using high_res_duration_t = high_res_clock_t::duration;
	using high_res_time_point_t = high_res_clock_t::time_point;
	
} // bump
