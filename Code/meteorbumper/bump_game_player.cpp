#include "bump_game_player.hpp"

#include "bump_game_assets.hpp"
#include "bump_game_asteroids.hpp"
#include "bump_game_crosshair.hpp"
#include "bump_game_powerups.hpp"
#include "bump_physics.hpp"

#include <iostream>

namespace bump
{
	
	namespace game
	{
		
		void player_controls::apply(physics::rigidbody& physics, crosshair& crosshair, glm::vec2 window_size, camera_matrices const& matrices)
		{
			auto const player_transform = physics.get_transform();

			// apply maneuvering forces
			{
				auto const move_force_N = 1000.f;

				auto const up = glm::vec3{ 0.f, 0.f, -1.f };
				auto const down = -up;
				auto const left = glm::vec3{ -1.f, 0.f, 0.f };
				auto const right = -left;
				
				physics.add_force(up * move_force_N * m_vertical_axis);
				physics.add_force(right * move_force_N * m_horizontal_axis);
			}

			// apply boost force
			// todo: judder? (smooth, judder at mid speeds, smooth at high speeds).
			{
				auto const boost_dir = forwards(player_transform);
				auto const boost_force_N = 2500.f;

				physics.add_force(boost_dir * boost_force_N * m_boost_axis);
			}

			// simple linear drag
			{
				auto const k1 = 2.f;
				auto const k2 = 0.2f;
				auto const v = physics.get_velocity();
				auto const l = glm::length(v);

				if (!glm::epsilonEqual(l, 0.f, glm::epsilon<float>()))
				{
					auto drag_factor = k1 * l + k2 * l * l;
					auto drag = -glm::normalize(v) * drag_factor;
					physics.add_force(drag);
				}
			}

			// lower damping for lower velocity
			{
				auto const min_damping = 0.99999f;
				auto const max_damping = 0.998f;
				auto const min_damping_speed = 5.f;
				auto const max_damping_speed = 50.f;
				auto const speed = glm::length(physics.get_velocity());
				auto const speed_factor = glm::clamp((speed - min_damping_speed) / (max_damping_speed - min_damping_speed), 0.f, 1.f);
				auto const damping = glm::mix(min_damping, max_damping, speed_factor);

				//std::cout << glm::length(physics.get_velocity()) << " " << damping << std::endl;
				physics.set_linear_damping(damping);
			}

			// crosshair
			{
				if (m_mouse_update)
				{
					auto const move = m_mouse_motion * window_size;
					crosshair.m_position += move;
					crosshair.m_position = glm::clamp(crosshair.m_position, glm::vec2(0.f), window_size);

					m_mouse_motion = glm::vec2(0.f);
					m_mouse_update = false;
				}

				if (m_controller_update)
				{
					auto vector = m_controller_position;

					// ignore inputs in deadzone
					auto const deadzone = 0.1f;
					vector = glm::length(vector) > deadzone ? vector : glm::vec2(0.f);
					// clamp input to a circle (looks neater)
					vector = glm::length(vector) <= 1.f ? vector : glm::normalize(vector);

					crosshair.m_position = (window_size * 0.5f) + vector * glm::min(window_size.x, window_size.y) * 0.5f;
					crosshair.m_position = glm::clamp(crosshair.m_position, glm::vec2(0.f), window_size);

					m_controller_update = false;
				}
			}

			// rotate to face crosshair
			{
				// unproject from screen space to world space (point on the near plane of the camera)
				auto cross_screen = glm::vec4{ crosshair.m_position.x, crosshair.m_position.y, 1.f, 1.f };
				auto cross_clip = matrices.m_inv_viewport * cross_screen;
				auto cross_world = matrices.m_inv_view_projection * cross_clip;
				cross_world.x /= cross_world.w;
				cross_world.y /= cross_world.w;
				cross_world.z /= cross_world.w;
				cross_world.w = 1.f;

				// project ray from camera position through the near-plane point
				auto ray_origin = get_position(matrices.m_inv_view);
				auto ray_dir = glm::normalize(glm::vec3(cross_world) - ray_origin);
				
				// intersect ray with y = 0 plane (find target aim point on plane)
				auto plane_point = glm::vec3(0.f);
				auto plane_normal = glm::vec3{ 0.f, 1.f, 0.f };
				auto p1 = glm::dot(ray_origin - plane_point, plane_normal);
				auto p2 = glm::dot(ray_dir, plane_normal);
				auto target_pos = ray_origin - (ray_dir * (p1 / p2));

				// get direction from player to target
				auto target_dir = target_pos - physics.get_position();

				if (glm::length(target_dir) > 0.1f) // avoid div by zero and also controller jitter
				{
					target_dir = glm::normalize(target_dir);

					// set player orientation to point in target_dir
					auto up = glm::vec3{ 0.f, 0.f, -1.f };
					auto target_angle = glm::orientedAngle(up, target_dir, glm::vec3{ 0.f, 1.f, 0.f });
					auto target_orientation = glm::angleAxis(target_angle, glm::vec3{ 0.f, 1.f, 0.f });

					physics.set_orientation(target_orientation); // todo: move towards it over time?
				}
			}
		}

		namespace
		{

			auto const g_low_damage_color = glm::vec3{ 0.f, 1.f, 0.f };
			auto const g_medium_damage_color = glm::vec3{ 1.f, 0.984f, 0.196f };
			auto const g_high_damage_color = glm::vec3{ 1.f, 0.169f, 0.929f };

			auto const g_low_damage = 10.f;
			auto const g_medium_damage = 20.f;
			auto const g_high_damage = 30.f;

		} // unnamed

		player_lasers::player_lasers(entt::registry& registry, gl::shader_program const& shader):
			m_registry(registry),
			m_shader(shader),
			m_in_Color(shader.get_attribute_location("in_Color")),
			m_in_Position(shader.get_attribute_location("in_Position")),
			m_in_Direction(shader.get_attribute_location("in_Direction")),
			m_in_BeamLength(shader.get_attribute_location("in_BeamLength")),
			m_u_MVP(shader.get_uniform_location("u_MVP")),
			m_firing_period(std::chrono::duration_cast<high_res_duration_t>(std::chrono::duration<float>(0.125f))),
			m_time_since_firing(m_firing_period),
			m_beam_speed_m_per_s(100.f),
			m_beam_length_factor(0.5f),
			m_upgrade_level(0)
		{
			// set up instance buffers
			m_instance_color.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 3, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_Color, m_instance_color, 1);
			m_instance_position.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 3, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_Position, m_instance_position, 1);
			m_instance_direction.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 3, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_Direction, m_instance_direction, 1);
			m_instance_beam_length.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 1, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_BeamLength, m_instance_beam_length, 1);
			
			// set up two emitters to start with (todo: proper configurations based on upgrade level...)
			auto left_emitter = emitter
			{
				g_low_damage,
				g_low_damage_color,
				{ -2.f, -0.1f, -1.f }, // origin
				std::chrono::duration_cast<high_res_duration_t>(std::chrono::duration<float>(1.f)),
				{},
			};

			m_emitters.push_back(std::move(left_emitter));
			
			auto right_emitter = emitter
			{
				g_low_damage,
				g_low_damage_color,
				{ 2.f, -0.1f, -1.f }, // origin
				std::chrono::duration_cast<high_res_duration_t>(std::chrono::duration<float>(1.f)),
				{},
			};

			m_emitters.push_back(std::move(right_emitter));
		}

		player_lasers::~player_lasers()
		{
			for (auto const& emitter : m_emitters)
				for (auto beam : emitter.m_beams)
					m_registry.destroy(beam);
		}

		void player_lasers::upgrade()
		{
			if (m_upgrade_level == 4)
				return;

			++m_upgrade_level;

			if (m_upgrade_level == 1)
			{
				// increase damage
				{
					m_emitters[0].m_damage = g_medium_damage;
					m_emitters[0].m_color = g_medium_damage_color;
					
					m_emitters[1].m_damage = g_medium_damage;
					m_emitters[1].m_color = g_medium_damage_color;
				}

				// add two more emitters
				{
					auto left_emitter = emitter
					{
						g_low_damage,
						g_low_damage_color,
						{ -3.f, -0.1f, -0.4f }, // origin
						std::chrono::duration_cast<high_res_duration_t>(std::chrono::duration<float>(1.f)),
						{},
					};

					m_emitters.push_back(std::move(left_emitter));

					auto right_emitter = emitter
					{
						g_low_damage,
						g_low_damage_color,
						{ 3.f, -0.1f, -0.4f }, // origin
						std::chrono::duration_cast<high_res_duration_t>(std::chrono::duration<float>(1.f)),
						{},
					};
					
					m_emitters.push_back(std::move(right_emitter));
				}
			}
			else if (m_upgrade_level == 2)
			{
				// increase damage
				{
					m_emitters[0].m_damage = g_high_damage;
					m_emitters[0].m_color = g_high_damage_color;
					
					m_emitters[1].m_damage = g_high_damage;
					m_emitters[1].m_color = g_high_damage_color;
					
					m_emitters[2].m_damage = g_medium_damage;
					m_emitters[2].m_color = g_medium_damage_color;
					
					m_emitters[3].m_damage = g_medium_damage;
					m_emitters[3].m_color = g_medium_damage_color;
				}

				// add two more emitters
				{
					auto left_emitter = emitter
					{
						g_low_damage,
						g_low_damage_color,
						{ -4.f, -0.1f, 0.3f }, // origin
						std::chrono::duration_cast<high_res_duration_t>(std::chrono::duration<float>(1.f)),
						{},
					};

					m_emitters.push_back(std::move(left_emitter));

					auto right_emitter = emitter
					{
						g_low_damage,
						g_low_damage_color,
						{ 4.f, -0.1f, 0.3f }, // origin
						std::chrono::duration_cast<high_res_duration_t>(std::chrono::duration<float>(1.f)),
						{},
					};

					m_emitters.push_back(std::move(right_emitter));
				}
			}
			else if (m_upgrade_level == 3)
			{
				// increase damage
				{
					m_emitters[2].m_damage = g_high_damage;
					m_emitters[2].m_color = g_high_damage_color;
					
					m_emitters[3].m_damage = g_high_damage;
					m_emitters[3].m_color = g_high_damage_color;
					
					m_emitters[4].m_damage = g_medium_damage;
					m_emitters[4].m_color = g_medium_damage_color;
					
					m_emitters[5].m_damage = g_medium_damage;
					m_emitters[5].m_color = g_medium_damage_color;
				}
			}
			else if (m_upgrade_level == 4)
			{
				// increase damage
				{
					m_emitters[4].m_damage = g_high_damage;
					m_emitters[4].m_color = g_high_damage_color;
					
					m_emitters[5].m_damage = g_high_damage;
					m_emitters[5].m_color = g_high_damage_color;
				}
			}
		}

		void player_lasers::update(bool fire, glm::mat4 const& player_transform, high_res_duration_t dt)
		{
			m_time_since_firing += dt;

			if (fire && m_time_since_firing >= m_firing_period)
			{
				// add a new beam to each emitter
				for (auto& emitter : m_emitters)
				{
					auto beam_entity = m_registry.create();
					
					auto& beam_physics = m_registry.emplace<physics::rigidbody>(beam_entity);
					beam_physics.set_mass(0.1f);
					beam_physics.set_local_inertia_tensor(physics::make_sphere_inertia_tensor(0.1f, 0.1f));
					beam_physics.set_position(transform_point_to_world(player_transform, emitter.m_origin));
					beam_physics.set_velocity(forwards(player_transform) * m_beam_speed_m_per_s);

					auto& beam_collision = m_registry.emplace<physics::collider>(beam_entity);
					beam_collision.set_shape({ physics::sphere_shape{ 0.1f } }); // todo: line!
					beam_collision.set_collision_layer(physics::collision_layers::PLAYER_WEAPONS);
					beam_collision.set_collision_mask(~physics::collision_layers::PLAYER);

					auto deleter = [=] (entt::entity, physics::collision_data const&, float)
					{
						m_registry.get<beam_segment>(beam_entity).m_collided = true;
					};

					beam_collision.set_callback(std::move(deleter));

					auto& segment = m_registry.emplace<beam_segment>(beam_entity);
					segment.m_color = emitter.m_color;
					segment.m_beam_length = 0.f;
					segment.m_lifetime = high_res_duration_t{ 0 };
					segment.m_collided = false;

					auto& damage = m_registry.emplace<player_weapon_damage>(beam_entity);
					damage.m_damage = emitter.m_damage;

					emitter.m_beams.push_back(beam_entity);
				}

				// reset firing timer
				m_time_since_firing = high_res_duration_t{ 0 };
			}

			auto const dt_s = std::chrono::duration_cast<std::chrono::duration<float>>(dt).count();
			auto const firing_period_s = std::chrono::duration_cast<std::chrono::duration<float>>(m_firing_period).count();
			auto const max_beam_length = m_beam_speed_m_per_s * firing_period_s * m_beam_length_factor;

			auto view = m_registry.view<beam_segment>();

			for (auto& emitter : m_emitters)
			{
				// update the length of the most recently fired beam
				if (!emitter.m_beams.empty())
				{
					auto& first = view.get<beam_segment>(emitter.m_beams.back());

					if (first.m_beam_length < max_beam_length)
					{
						first.m_beam_length += m_beam_speed_m_per_s * dt_s;
						first.m_beam_length = std::min(first.m_beam_length, max_beam_length);
					}
				}

				// update beam lifetimes
				for (auto beam : emitter.m_beams)
					view.get<beam_segment>(beam).m_lifetime += dt;
				
				// find and remove dead beams
				auto first_dead_beam = std::remove_if(emitter.m_beams.begin(), emitter.m_beams.end(),
					[&] (entt::entity b)
					{
						auto const& segment = view.get<beam_segment>(b);
						auto result = (segment.m_lifetime > emitter.m_max_lifetime) || segment.m_collided;

						if (result)
							m_registry.destroy(b);

						return result;
					});
				
				emitter.m_beams.erase(first_dead_beam, emitter.m_beams.end());
			}
		}

		void player_lasers::render(gl::renderer& renderer, camera_matrices const& matrices)
		{
			// get beam instance data for this frame
			auto view = m_registry.view<beam_segment, physics::rigidbody>();

			for (auto id : view)
			{
				auto const& physics = view.get<physics::rigidbody>(id);

				auto beam_direction = physics.get_velocity();
				beam_direction = glm::length(beam_direction) == 0.f ? glm::vec3(0.f) : glm::normalize(beam_direction);

				auto const& segment = view.get<beam_segment>(id);

				m_frame_instance_colors.push_back(segment.m_color);
				m_frame_instance_positions.push_back(physics.get_position());
				m_frame_instance_directions.push_back(beam_direction);
				m_frame_instance_beam_lengths.push_back(segment.m_beam_length);
			}

			// upload beam instance data to gpu buffers
			auto const instance_count = m_frame_instance_colors.size();

			if (instance_count == 0)
				return; // nothing to render

			m_instance_color.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_frame_instance_colors.front()), 3, instance_count, GL_STREAM_DRAW);
			m_instance_position.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_frame_instance_positions.front()), 3, instance_count, GL_STREAM_DRAW);
			m_instance_direction.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_frame_instance_directions.front()), 3, instance_count, GL_STREAM_DRAW);
			m_instance_beam_length.set_data(GL_ARRAY_BUFFER, m_frame_instance_beam_lengths.data(), 1, instance_count, GL_STREAM_DRAW);

			// render beams
			auto mvp = matrices.model_view_projection_matrix(glm::mat4(1.f));
			
			renderer.set_program(m_shader);
			renderer.set_uniform_4x4f(m_u_MVP, mvp);
			renderer.set_vertex_array(m_vertex_array);

			renderer.draw_arrays(GL_POINTS, 1, instance_count);

			renderer.clear_vertex_array();
			renderer.clear_program();

			// clear frame instance buffers
			m_frame_instance_colors.clear();
			m_frame_instance_positions.clear();
			m_frame_instance_directions.clear();
			m_frame_instance_beam_lengths.clear();
		}

		player_weapons::player_weapons(entt::registry& registry, gl::shader_program const& laser_shader):
			m_lasers(registry, laser_shader)
			{ }
			
		void player_weapons::update(bool fire, glm::mat4 const& player_transform, high_res_duration_t dt)
		{
			m_lasers.update(fire, player_transform, dt);
		}

		void player_weapons::render(gl::renderer& renderer, camera_matrices const& matrices)
		{
			m_lasers.render(renderer, matrices);
		}

		
		player_health::player_health():
			m_shield_max_hp(100.f),
			m_armor_max_hp(250.f),
			m_shield_hp(m_shield_max_hp),
			m_armor_hp(m_armor_max_hp),
			m_shield_recharge_rate_hp_per_s(20.f),
			m_shield_recharge_delay(std::chrono::duration_cast<high_res_duration_t>(std::chrono::duration<float>(5.f))),
			m_time_since_last_hit(0) { }

		void player_health::update(high_res_duration_t dt)
		{
			m_time_since_last_hit += dt;

			if (m_time_since_last_hit > m_shield_recharge_delay)
			{
				auto const dt_s = std::chrono::duration_cast<std::chrono::duration<float>>(dt).count();
				m_shield_hp = std::clamp(m_shield_hp + m_shield_recharge_rate_hp_per_s * dt_s, 0.f, m_shield_max_hp);
			}
		}

		void player_health::take_damage(float damage)
		{
			if (has_shield())
				m_shield_hp = std::clamp(m_shield_hp - damage, 0.f, m_shield_max_hp);
			else
				m_armor_hp = std::clamp(m_armor_hp - damage, 0.f, m_armor_max_hp);

			m_time_since_last_hit = high_res_duration_t{ 0 };
		}


		player::player(entt::registry& registry, assets& assets):
			m_registry(registry),
			m_entity(entt::null_t()),
			m_renderable(assets.m_shaders.at("player_ship"), assets.m_models.at("player_ship")),
			m_controls(),
			m_weapons(registry, assets.m_shaders.at("player_laser"))
		{
			m_entity = registry.create();

			auto const mass_kg = 20.f;
			auto const radius_m = 3.f;

			auto& player_physics = registry.emplace<physics::rigidbody>(m_entity);
			player_physics.set_mass(mass_kg);

			player_physics.set_local_inertia_tensor(physics::make_sphere_inertia_tensor(mass_kg, radius_m));
			player_physics.set_linear_damping(0.998f);
			player_physics.set_angular_damping(0.998f);
			player_physics.set_linear_factor({ 1.f, 0.f, 1.f }); // restrict movement on y axis
			player_physics.set_angular_factor({ 0.f, 1.f, 0.f }); // restrict rotation to y axis only

			auto& player_collision = registry.emplace<physics::collider>(m_entity);
			player_collision.set_shape({ physics::sphere_shape{ radius_m } });
			player_collision.set_collision_layer(physics::collision_layers::PLAYER);
			player_collision.set_collision_mask(~physics::collision_layers::PLAYER_WEAPONS);
			player_collision.set_restitution(m_player_shield_restitution);

			auto hit_callback = [=] (entt::entity other, physics::collision_data const&, float rv)
			{
				if (m_registry.has<asteroid_field::asteroid_data>(other))
				{
					auto const max_damage = 100.f;
					auto const vf = glm::pow(glm::clamp(rv / 100.f, 0.f, 1.f), 2.f); // damage factor from velocity
					auto const damage = glm::mix(0.f, max_damage, vf);

					m_health.take_damage(damage);
				}
				else if (m_registry.has<powerups::powerup_data>(other))
				{
					auto const& powerup = m_registry.get<powerups::powerup_data>(other);

					if (powerup.m_type == powerups::powerup_type::RESET_SHIELDS)
					{
						m_health.reset_shield_hp();
					}
					else if (powerup.m_type == powerups::powerup_type::RESET_ARMOR)
					{
						m_health.reset_armor_hp();
					}
					else if (powerup.m_type == powerups::powerup_type::UPGRADE_LASERS)
					{
						m_weapons.m_lasers.upgrade();
					}
				}
			};

			player_collision.set_callback(std::move(hit_callback));
		}

		void player::update(high_res_duration_t dt)
		{
			auto const& rigidbody = m_registry.get<physics::rigidbody>(m_entity);

			m_renderable.set_transform(rigidbody.get_transform());

			m_weapons.update(m_controls.m_firing, rigidbody.get_transform(), dt);

			m_health.update(dt);

			// if we have shields, make collisions "bouncier" by increasing restitution
			auto& collider = m_registry.get<physics::collider>(m_entity);
			collider.set_restitution(m_health.has_shield() ? m_player_shield_restitution : m_player_armor_restitution);
		}

		void player::render(gl::renderer& renderer, camera_matrices const& matrices)
		{
			m_renderable.render(renderer, matrices);
			m_weapons.render(renderer, matrices);
		}
		
	} // game
	
} // bump
