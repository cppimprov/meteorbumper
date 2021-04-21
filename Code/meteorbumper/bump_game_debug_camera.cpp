#include "bump_game_debug_camera.hpp"

#include "bump_transform.hpp"

namespace bump
{
	
	namespace game
	{
		
		debug_camera_controls::debug_camera_controls():
			m_move_fast(false),
			m_move_forwards(false), m_move_backwards(false),
			m_move_left(false), m_move_right(false),
			m_move_up(false), m_move_down(false),
			m_rotate(0.f),
			m_transform(1.f) { }
			
		void debug_camera_controls::update(high_res_duration_t dt)
		{
			auto const dt_seconds = high_res_duration_to_seconds(dt);
			auto const movement_speed = 5.f; // metres per second
			auto const rotation_speed = 100.f; // degrees per mouse input unit per second (?)
			auto const speed_modifier = m_move_fast ? 5.f : 1.f;

			if (m_rotate.y != 0.f)
			{
				auto const amount = rotation_speed * m_rotate.y * dt_seconds;
				rotate_around_local_axis(m_transform, { 1.f, 0.f, 0.f }, glm::radians(amount));
			}

			if (m_rotate.x != 0.f)
			{
				auto const amount = rotation_speed * m_rotate.x * dt_seconds;
				rotate_around_world_axis(m_transform, { 0.f, 1.f, 0.f }, glm::radians(amount));
			}

			m_rotate = glm::vec2(0.f);

			if (m_move_forwards)
			{
				auto const amount = movement_speed * speed_modifier * dt_seconds;
				translate_in_local(m_transform, glm::vec3{ 0.f, 0.f, -1.f } * amount);
			}

			if (m_move_backwards)
			{
				auto const amount = movement_speed * speed_modifier * dt_seconds;
				translate_in_local(m_transform, glm::vec3{ 0.f, 0.f, 1.f } * amount);
			}

			if (m_move_left)
			{
				auto const amount = movement_speed * speed_modifier * dt_seconds;
				translate_in_local(m_transform, glm::vec3{ -1.f, 0.f, 0.f } * amount);
			}

			if (m_move_right)
			{
				auto const amount = movement_speed * speed_modifier * dt_seconds;
				translate_in_local(m_transform, glm::vec3{ 1.f, 0.f, 0.f } * amount);
			}

			if (m_move_up)
			{
				auto const amount = movement_speed * speed_modifier * dt_seconds;
				translate_in_local(m_transform, glm::vec3{ 0.f, 1.f, 0.f } * amount);
			}

			if (m_move_down)
			{
				auto const amount = movement_speed * speed_modifier * dt_seconds;
				translate_in_local(m_transform, glm::vec3{ 0.f, -1.f, 0.f } * amount);
			}
		}
		
	} // game
	
} // bump