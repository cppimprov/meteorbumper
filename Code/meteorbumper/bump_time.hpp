#pragma once

#include <chrono>

namespace bump
{
	
	using high_res_clock_t = std::chrono::high_resolution_clock;
	using high_res_duration_t = high_res_clock_t::duration;
	using high_res_time_point_t = high_res_clock_t::time_point;

	inline high_res_duration_t high_res_duration_from_seconds(float seconds)
	{
		return std::chrono::duration_cast<high_res_duration_t>(std::chrono::duration<float>(seconds));
	}

	inline float high_res_duration_to_seconds(high_res_duration_t duration)
	{
		return std::chrono::duration_cast<std::chrono::duration<float>>(duration).count();
	}
	
} // bump
