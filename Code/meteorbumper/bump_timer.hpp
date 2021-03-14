#pragma once

#include "bump_time.hpp"

namespace bump
{

	template<class ClockT = high_res_clock_t>
	class timer
	{
	public:

		using clock_t = ClockT;

		timer():
			m_start(clock_t::now()) { }

		typename clock_t::duration get_elapsed_time() const
		{
			return clock_t::now() - m_start;
		}

	private:

		typename clock_t::time_point m_start;
	};

	template<class ClockT = high_res_clock_t>
	class frame_timer
	{
	public:

		using clock_t = ClockT;

		explicit frame_timer(typename clock_t::duration last_frame_time = std::chrono::duration_cast<typename clock_t::duration>(std::chrono::duration<float>{ 1.f / 60.f })):
			m_timer(),
			m_last_frame_time(last_frame_time) { }
		
		typename clock_t::duration get_last_frame_time() const
		{
			return m_last_frame_time;
		}

		void tick()
		{
			m_last_frame_time = m_timer.get_elapsed_time();
			m_timer = timer<clock_t>();
		}

	private:

		timer<clock_t> m_timer;
		typename clock_t::duration m_last_frame_time;
	};

} // bump
