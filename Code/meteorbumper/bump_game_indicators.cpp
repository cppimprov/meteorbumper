#include "bump_game_indicators.hpp"

#include "bump_camera.hpp"
#include "bump_game_asteroids.hpp"
#include "bump_game_powerups.hpp"
#include "bump_physics.hpp"

#include <glm/gtx/string_cast.hpp>
#include <glm/glm.hpp>

#include <iostream>

namespace bump
{
	
	namespace game
	{
		
		indicators::indicators(entt::registry& registry, gl::shader_program const& shader):
			m_registry(registry),
			m_shader(shader),
			m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
			m_in_Direction(shader.get_attribute_location("in_Direction")),
			m_in_Color(shader.get_attribute_location("in_Color")),
			m_u_MVP(shader.get_uniform_location("u_MVP")),
			m_u_Size(shader.get_uniform_location("u_Size"))
		{
			m_buffer_positions.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 2, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_VertexPosition, m_buffer_positions, 1);
			m_buffer_directions.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 2, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_Direction, m_buffer_directions, 1);
			m_buffer_colors.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 3, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_Color, m_buffer_colors, 1);
		}

		void indicators::render(gl::renderer& renderer, glm::vec2 window_size, camera_matrices const& screen_matrices, camera_matrices const& ui_matrices)
		{
			auto scene_mvp = screen_matrices.model_view_projection_matrix(glm::mat4(1.f));

			struct id_distance
			{
				entt::entity m_id;
				float m_distance;
			};

			auto const asteroid_color = glm::vec3(0.8f);
			auto const powerup_colors = std::map<powerups::powerup_type, glm::vec3>
			{
				{ powerups::powerup_type::RESET_SHIELDS, glm::vec3(0.080f, 0.636f, 0.674f) },
				{ powerups::powerup_type::RESET_ARMOR, glm::vec3(0.828f, 0.801f, 0.044f) },
				{ powerups::powerup_type::UPGRADE_LASERS, glm::vec3(0.055f, 0.814f, 0.091f) },
			};

			// find closest asteroids
			{
				auto view = m_registry.view<asteroid_field::asteroid_data, physics::rigidbody>();

				auto distances = std::vector<id_distance>();
				distances.reserve(view.size());

				for (auto id : view)
				{
					auto const& rb = view.get<physics::rigidbody>(id);
					auto const distance = glm::distance(m_player_position, rb.get_position());
					distances.push_back({ id, distance });
				}

				// find closest 5, discard the rest
				auto const num = std::min(distances.size(), std::size_t{ 5 });
				std::partial_sort(distances.begin(), distances.begin() + num, distances.end(),
					[] (id_distance const& a, id_distance const& b) { return a.m_distance < b.m_distance; });
				distances.resize(num);

				// add asteroids to the indicator list
				for (auto id : distances)
				{
					auto const& rb = view.get<physics::rigidbody>(id.m_id);

					// get world space direction
					auto direction = rb.get_position() - m_player_position;
					if (glm::length(direction) == 0.f) continue;
					direction = glm::normalize(direction);

					// get screen space direction
					auto dir_world = glm::vec4{ direction.x, direction.y, direction.z, 0.0 };
					auto dir_screen = screen_matrices.m_viewport * scene_mvp * dir_world;
					auto dir_final = glm::normalize(glm::vec2{ dir_screen.x, dir_screen.y });

					auto const radius = 75.f;
					auto const pos = (window_size / 2.f) + dir_final * radius;

					m_frame_positions.push_back(pos);
					m_frame_directions.push_back(dir_final);
					m_frame_colors.push_back(asteroid_color);
				}
			}

			// find closest powerups
			{
				auto view = m_registry.view<powerups::powerup_data, physics::rigidbody>();

				auto distances = std::vector<id_distance>();
				distances.reserve(view.size());

				for (auto id : view)
				{
					auto const& rb = view.get<physics::rigidbody>(id);
					auto const distance = glm::distance(m_player_position, rb.get_position());
					distances.push_back({ id, distance });
				}

				// find closest 5, discard the rest
				auto const num = std::min(distances.size(), std::size_t{ 5 });
				std::partial_sort(distances.begin(), distances.begin() + num, distances.end(),
					[] (id_distance const& a, id_distance const& b) { return a.m_distance < b.m_distance; });
				distances.resize(num);

				// add powerups to the indicator list
				for (auto id : distances)
				{
					auto [pu, rb] = view.get<powerups::powerup_data, physics::rigidbody>(id.m_id);

					// get world space direction
					auto direction = rb.get_position() - m_player_position;
					if (glm::length(direction) == 0.f) continue;
					direction = glm::normalize(direction);

					// get screen space direction
					auto dir_world = glm::vec4{ direction.x, direction.y, direction.z, 0.0 };
					auto dir_screen = screen_matrices.m_viewport * scene_mvp * dir_world;
					auto dir_final = glm::normalize(glm::vec2{ dir_screen.x, dir_screen.y });

					auto const radius = 75.f;
					auto const pos = (window_size / 2.f) + dir_final * radius;

					m_frame_positions.push_back(pos);
					m_frame_directions.push_back(dir_final);
					m_frame_colors.push_back(powerup_colors.at(pu.m_type));
				}
			}

			if (m_frame_positions.empty())
				return; // nothing to do!

			auto instance_count = m_frame_positions.size();

			static_assert(sizeof(glm::vec2) == sizeof(float) * 2, "Unexpected vector size.");
			m_buffer_positions.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_frame_positions.front()), 2, m_frame_positions.size(), GL_STREAM_DRAW);
			m_buffer_directions.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_frame_directions.front()), 2, m_frame_directions.size(), GL_STREAM_DRAW);
			static_assert(sizeof(glm::vec3) == sizeof(float) * 3, "Unexpected vector size.");
			m_buffer_colors.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_frame_colors.front()), 3, m_frame_colors.size(), GL_STREAM_DRAW);

			auto mvp = ui_matrices.model_view_projection_matrix(glm::mat4(1.f));
			
			renderer.set_program(m_shader);
			renderer.set_uniform_4x4f(m_u_MVP, mvp);
			renderer.set_uniform_1f(m_u_Size, 3.f);
			renderer.set_vertex_array(m_vertex_array);

			renderer.draw_arrays(GL_POINTS, 1, instance_count);

			renderer.clear_vertex_array();
			renderer.clear_program();

			m_frame_positions.clear();
			m_frame_directions.clear();
			m_frame_colors.clear();
		}
		
	} // game
	
} // bump