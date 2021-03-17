#pragma once

#include "bump_time.hpp"

#include <glm/glm.hpp>

namespace bump
{
	
	namespace game
	{
		
		class debug_camera_controls
		{
		public:

			debug_camera_controls();

			void update(high_res_duration_t dt);

			bool m_move_fast;
			bool m_move_forwards, m_move_backwards;
			bool m_move_left, m_move_right;
			bool m_move_up, m_move_down;
			glm::vec2 m_rotate;

			glm::mat4 m_transform;
		};
		
		
	} // game
	
} // bump