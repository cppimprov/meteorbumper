#include "bump_game_player.hpp"

#include "bump_game_assets.hpp"
#include "bump_game_asteroids.hpp"
#include "bump_game_crosshair.hpp"
#include "bump_game_powerups.hpp"
#include "bump_gbuffers.hpp"
#include "bump_physics.hpp"

#include <Tracy.hpp>

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

		player_lasers::player_lasers(entt::registry& registry, gl::shader_program const& shader, gl::shader_program const& hit_shader):
			m_registry(registry),
			m_shader(shader),
			m_in_Color(shader.get_attribute_location("in_Color")),
			m_in_Position(shader.get_attribute_location("in_Position")),
			m_in_Direction(shader.get_attribute_location("in_Direction")),
			m_in_BeamLength(shader.get_attribute_location("in_BeamLength")),
			m_u_MVP(shader.get_uniform_location("u_MVP")),
			m_firing_period(high_res_duration_from_seconds(0.125f)),
			m_time_since_firing(m_firing_period),
			m_beam_speed_m_per_s(100.f),
			m_beam_length_factor(0.5f),
			m_low_damage_hit_effects(registry, hit_shader), 
			m_medium_damage_hit_effects(registry, hit_shader),
			m_high_damage_hit_effects(registry, hit_shader)
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
				high_res_duration_from_seconds(1.f),
				{},
			};

			m_emitters.push_back(std::move(left_emitter));
			
			auto right_emitter = emitter
			{
				g_low_damage,
				g_low_damage_color,
				{ 2.f, -0.1f, -1.f }, // origin
				high_res_duration_from_seconds(1.f),
				{},
			};

			m_emitters.push_back(std::move(right_emitter));

			// set up hit effects
			{
				auto const low_damage_color_map = std::map<float, glm::vec4>
				{
					{ 0.0f, { g_low_damage_color, 1.f } },
					{ 0.7f, { g_low_damage_color, 0.8f } },
					{ 1.0f, { g_low_damage_color, 0.2f } },
				};

				m_low_damage_hit_effects.set_spawn_radius(0.2f);
				m_low_damage_hit_effects.set_random_velocity({ 10.f, 10.f, 10.f });
				m_low_damage_hit_effects.set_max_lifetime(high_res_duration_from_seconds(0.75f));
				m_low_damage_hit_effects.set_max_lifetime_random(high_res_duration_from_seconds(0.05f));
				m_low_damage_hit_effects.set_color_map(low_damage_color_map);
				m_low_damage_hit_effects.set_max_particle_count(100);
				
				auto const medium_damage_color_map = std::map<float, glm::vec4>
				{
					{ 0.0f, { g_medium_damage_color, 1.f } },
					{ 0.7f, { g_medium_damage_color, 0.8f } },
					{ 1.0f, { g_medium_damage_color, 0.2f } },
				};

				m_medium_damage_hit_effects.set_spawn_radius(0.2f);
				m_medium_damage_hit_effects.set_random_velocity({ 10.f, 10.f, 10.f });
				m_medium_damage_hit_effects.set_max_lifetime(high_res_duration_from_seconds(0.75f));
				m_medium_damage_hit_effects.set_max_lifetime_random(high_res_duration_from_seconds(0.05f));
				m_medium_damage_hit_effects.set_color_map(medium_damage_color_map);
				m_medium_damage_hit_effects.set_max_particle_count(100);
				
				auto const high_damage_color_map = std::map<float, glm::vec4>
				{
					{ 0.0f, { g_high_damage_color, 1.f } },
					{ 0.7f, { g_high_damage_color, 0.8f } },
					{ 1.0f, { g_high_damage_color, 0.2f } },
				};

				m_high_damage_hit_effects.set_spawn_radius(0.2f);
				m_high_damage_hit_effects.set_random_velocity({ 10.f, 10.f, 10.f });
				m_high_damage_hit_effects.set_max_lifetime(high_res_duration_from_seconds(0.75f));
				m_high_damage_hit_effects.set_max_lifetime_random(high_res_duration_from_seconds(0.05f));
				m_high_damage_hit_effects.set_color_map(high_damage_color_map);
				m_high_damage_hit_effects.set_max_particle_count(100);
			}
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
						high_res_duration_from_seconds(1.f),
						{},
					};

					m_emitters.push_back(std::move(left_emitter));

					auto right_emitter = emitter
					{
						g_low_damage,
						g_low_damage_color,
						{ 3.f, -0.1f, -0.4f }, // origin
						high_res_duration_from_seconds(1.f),
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
						high_res_duration_from_seconds(1.f),
						{},
					};

					m_emitters.push_back(std::move(left_emitter));

					auto right_emitter = emitter
					{
						g_low_damage,
						g_low_damage_color,
						{ 4.f, -0.1f, 0.3f }, // origin
						high_res_duration_from_seconds(1.f),
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

		void player_lasers::update(bool fire, glm::mat4 const& player_transform, glm::vec3 player_velocity, high_res_duration_t dt)
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
					beam_physics.set_velocity(player_velocity + forwards(player_transform) * m_beam_speed_m_per_s);

					auto& beam_collision = m_registry.emplace<physics::collider>(beam_entity);
					beam_collision.set_shape({ physics::sphere_shape{ 0.1f } }); // todo: line!
					beam_collision.set_collision_layer(physics::collision_layers::PLAYER_WEAPONS);
					beam_collision.set_collision_mask(~physics::collision_layers::PLAYER);

					auto deleter = [=] (entt::entity, physics::collision_data const& hit, float)
					{
						auto& bs = m_registry.get<beam_segment>(beam_entity);
						bs.m_collided = true;

						if (bs.m_color == g_low_damage_color)
							m_low_damage_frame_hit_positions.push_back(hit.m_point);
						else if (bs.m_color == g_medium_damage_color)
							m_medium_damage_frame_hit_positions.push_back(hit.m_point);
						else if (bs.m_color == g_high_damage_color)
							m_high_damage_frame_hit_positions.push_back(hit.m_point);
					};

					beam_collision.set_callback(std::move(deleter));

					auto& segment = m_registry.emplace<beam_segment>(beam_entity);
					segment.m_color = emitter.m_color;
					segment.m_beam_length = 0.f;
					segment.m_lifetime = high_res_duration_t{ 0 };
					segment.m_collided = false;

					auto& damage = m_registry.emplace<player_weapon_damage>(beam_entity);
					damage.m_damage = emitter.m_damage;

					auto& light = m_registry.emplace<point_light>(beam_entity);
					light.m_position = beam_physics.get_position();
					light.m_color = segment.m_color;
					light.m_radius = 5.f; // ?

					emitter.m_beams.push_back(beam_entity);
				}

				// reset firing timer
				m_time_since_firing = high_res_duration_t{ 0 };
			}

			auto const dt_s = high_res_duration_to_seconds(dt);
			auto const firing_period_s = high_res_duration_to_seconds(m_firing_period);
			auto const max_beam_length = m_beam_speed_m_per_s * firing_period_s * m_beam_length_factor;

			// update beam lifetimes
			{
				auto view = m_registry.view<beam_segment, physics::rigidbody, point_light>();

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
					for (auto id : emitter.m_beams)
					{
						auto [b, rb, l] = view.get<beam_segment, physics::rigidbody, point_light>(id);

						b.m_lifetime += dt;
						l.m_position = rb.get_position();
						// todo: dim the light / reduce light radius based on lifetime!
					}
					
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

			// update hit effects
			for (auto p : m_low_damage_frame_hit_positions)
			{
				auto m = glm::mat4(1.f);
				set_position(m, p);

				m_low_damage_hit_effects.set_origin(m);
				m_low_damage_hit_effects.spawn_once(15);
			}
			m_low_damage_frame_hit_positions.clear();
			
			for (auto p : m_medium_damage_frame_hit_positions)
			{
				auto m = glm::mat4(1.f);
				set_position(m, p);

				m_medium_damage_hit_effects.set_origin(m);
				m_medium_damage_hit_effects.spawn_once(15);
			}
			m_medium_damage_frame_hit_positions.clear();
			
			for (auto p : m_high_damage_frame_hit_positions)
			{
				auto m = glm::mat4(1.f);
				set_position(m, p);

				m_high_damage_hit_effects.set_origin(m);
				m_high_damage_hit_effects.spawn_once(15);
			}
			m_high_damage_frame_hit_positions.clear();

			m_low_damage_hit_effects.update(dt);
			m_medium_damage_hit_effects.update(dt);
			m_high_damage_hit_effects.update(dt);
		}

		void player_lasers::render(gl::renderer& renderer, camera_matrices const& matrices)
		{
			ZoneScopedN("player_lasers::render()");

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

			if (instance_count != 0)
			{
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

			m_low_damage_hit_effects.render(renderer, matrices);
			m_medium_damage_hit_effects.render(renderer, matrices);
			m_high_damage_hit_effects.render(renderer, matrices);
		}

		player_weapons::player_weapons(entt::registry& registry, gl::shader_program const& laser_shader, gl::shader_program const& laser_hit_shader):
			m_lasers(registry, laser_shader, laser_hit_shader)
			{ }
			
		void player_weapons::update(bool fire, glm::mat4 const& player_transform, glm::vec3 player_velocity, high_res_duration_t dt)
		{
			m_lasers.update(fire, player_transform, player_velocity, dt);
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
			m_shield_recharge_delay(high_res_duration_from_seconds(5.f)),
			m_time_since_last_hit(0) { }

		void player_health::update(high_res_duration_t dt)
		{
			m_time_since_last_hit += dt;

			if (m_time_since_last_hit > m_shield_recharge_delay)
			{
				auto const dt_s = high_res_duration_to_seconds(dt);
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
			m_ship_renderable(assets.m_shaders.at("player_ship"), assets.m_models.at("player_ship")),
			m_shield_renderable(assets.m_shaders.at("player_shield"), assets.m_models.at("player_shield")),
			m_controls(),
			m_weapons(registry, assets.m_shaders.at("player_laser"), assets.m_shaders.at("particle_effect")),
			m_left_engine_boost_effect(registry, assets.m_shaders.at("particle_effect")),
			m_right_engine_boost_effect(registry, assets.m_shaders.at("particle_effect")),
			m_shield_hit_effect(registry, assets.m_shaders.at("particle_effect")),
			m_armor_hit_effect(registry, assets.m_shaders.at("particle_effect"))
		{
			// setup player physics
			{
				m_entity = registry.create();

				registry.emplace<player_tag>(m_entity);

				auto const mass_kg = 20.f;

				auto& rigidbody = registry.emplace<physics::rigidbody>(m_entity);
				rigidbody.set_mass(mass_kg);

				rigidbody.set_local_inertia_tensor(physics::make_sphere_inertia_tensor(mass_kg, m_player_ship_radius_m));
				rigidbody.set_linear_damping(0.998f);
				rigidbody.set_angular_damping(0.998f);
				rigidbody.set_linear_factor({ 1.f, 0.f, 1.f }); // restrict movement on y axis
				rigidbody.set_angular_factor({ 0.f, 1.f, 0.f }); // restrict rotation to y axis only

				auto& collider = registry.emplace<physics::collider>(m_entity);
				collider.set_shape({ physics::sphere_shape{ m_player_shield_radius_m } });
				collider.set_collision_layer(physics::collision_layers::PLAYER);
				collider.set_collision_mask(~physics::collision_layers::PLAYER_WEAPONS);
				collider.set_restitution(m_player_shield_restitution);

				auto hit_callback = [this] (entt::entity other, physics::collision_data const& hit, float rv)
				{
					if (m_registry.has<asteroid_field::asteroid_data>(other))
					{
						auto const max_damage = 100.f;
						auto const vf = glm::pow(glm::clamp(rv / 100.f, 0.f, 1.f), 2.f); // damage factor from velocity
						auto const damage = glm::mix(0.f, max_damage, vf);

						// do shield hit effect
						if (m_health.has_shield())
							m_frame_shield_hits.push_back({ hit.m_point, hit.m_normal });
						else
							m_frame_armor_hits.push_back({ hit.m_point, hit.m_normal });

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

				collider.set_callback(std::move(hit_callback));
			}

			// setup engine effects
			{
				auto const color_map = std::map<float, glm::vec4>
				{
					{ 0.0f, { 1.f, 1.f, 0.9f, 1.f } },
					{ 0.2f, { 1.f, 0.85f, 0.42f, 1.f } },
					{ 0.4f, { 0.95f, 0.30f, 0.09f, 0.95f } },
					{ 0.7f, { 0.52f, 0.40f, 0.29f, 0.80f } },
					{ 1.0f, { 0.29f, 0.22f, 0.17f, 0.25f } },
				};

				auto const l_pos = glm::vec3{ -0.8f, 0.1f, 2.2f };
				auto l_mat = glm::mat4(1.f);
				set_position(l_mat, l_pos);

				m_left_engine_boost_effect.set_origin(l_mat);
				m_left_engine_boost_effect.set_base_velocity({ 0.f, 0.f, 15.f });
				m_left_engine_boost_effect.set_random_velocity({ 5.f, 5.f, 0.5f });
				m_left_engine_boost_effect.set_max_lifetime(high_res_duration_from_seconds(0.75f));
				m_left_engine_boost_effect.set_max_lifetime_random(high_res_duration_from_seconds(0.25f));
				m_left_engine_boost_effect.set_spawn_period(high_res_duration_from_seconds(1.f / 400.f));
				m_left_engine_boost_effect.set_color_map(color_map);

				auto const r_pos = glm::vec3{ 0.8f, 0.1f, 2.2f };
				auto r_mat = glm::mat4(1.f);
				set_position(r_mat, r_pos);

				m_right_engine_boost_effect.set_origin(r_mat);
				m_right_engine_boost_effect.set_base_velocity({ 0.f, 0.f, 15.f });
				m_right_engine_boost_effect.set_random_velocity({ 5.f, 5.f, 0.5f });
				m_right_engine_boost_effect.set_max_lifetime(high_res_duration_from_seconds(0.75f));
				m_right_engine_boost_effect.set_max_lifetime_random(high_res_duration_from_seconds(0.25f));
				m_right_engine_boost_effect.set_spawn_period(high_res_duration_from_seconds(1.f / 400.f));
				m_right_engine_boost_effect.set_color_map(color_map);
			}

			// setup shield / armor hit effects
			{
				auto const shield_color_map = std::map<float, glm::vec4>
				{
					{ 0.0f, { 0.507f, 0.627f, 0.840f, 1.f } },
					{ 0.7f, { 0.507f, 0.627f, 0.840f, 0.5f } },
					{ 1.0f, { 1.0f, 1.0f, 1.0f, 0.2f } },
				};

				m_shield_hit_effect.set_spawn_radius(0.1f);
				m_shield_hit_effect.set_random_velocity({ 10.f, 10.f, 10.f });
				m_shield_hit_effect.set_max_lifetime(high_res_duration_from_seconds(1.0f));
				m_shield_hit_effect.set_max_lifetime_random(high_res_duration_from_seconds(0.25f));
				m_shield_hit_effect.set_color_map(shield_color_map);

				auto const armor_color_map = std::map<float, glm::vec4>
				{
					{ 0.0f, { 0.828f, 0.801f, 0.044f, 1.f } },
					{ 0.7f, { 0.828f, 0.445f, 0.033f, 0.5f } },
					{ 1.0f, { 1.0f, 1.0f, 1.0f, 0.2f } },
				};
				
				m_armor_hit_effect.set_spawn_radius(0.1f);
				m_armor_hit_effect.set_random_velocity({ 5.f, 5.f, 5.f });
				m_armor_hit_effect.set_max_lifetime(high_res_duration_from_seconds(5.0f));
				m_armor_hit_effect.set_max_lifetime_random(high_res_duration_from_seconds(1.0f));
				m_armor_hit_effect.set_color_map(armor_color_map);
			}
		}

		player::~player()
		{
			m_registry.destroy(m_entity);
		}

		void player::update(high_res_duration_t dt)
		{
			auto const player_transform = m_registry.get<physics::rigidbody>(m_entity).get_transform();
			auto const player_velocity = m_registry.get<physics::rigidbody>(m_entity).get_velocity();

			m_ship_renderable.set_transform(player_transform);
			m_shield_renderable.set_transform(player_transform);

			m_weapons.update(m_controls.m_firing, player_transform, player_velocity, dt);

			m_health.update(dt);

			// update particle effects
			{
				auto const enabled = (m_controls.m_boost_axis == 1.f);

				auto const l_pos = glm::vec3{ -0.8f, 0.1f, 2.2f };
				auto l_mat = player_transform;
				translate_in_local(l_mat, l_pos);

				m_left_engine_boost_effect.set_spawn_enabled(enabled);
				m_left_engine_boost_effect.set_origin(l_mat);
				m_left_engine_boost_effect.update(dt);

				auto const r_pos = glm::vec3{ 0.8f, 0.1f, 2.2f };
				auto r_mat = player_transform;
				translate_in_local(r_mat, r_pos);
				
				m_right_engine_boost_effect.set_spawn_enabled(enabled);
				m_right_engine_boost_effect.set_origin(r_mat);
				m_right_engine_boost_effect.update(dt);

				for (auto const& hit : m_frame_shield_hits)
				{
					auto m = glm::mat4(1.f);
					set_position(m, std::get<0>(hit));

					m_shield_hit_effect.set_origin(m);
					//m_shield_hit_effect.set_base_velocity(std::get<1>(hit));
					m_shield_hit_effect.spawn_once(75);
				}
				m_frame_shield_hits.clear();

				for (auto const& hit : m_frame_armor_hits)
				{
					auto m = glm::mat4(1.f);
					set_position(m, std::get<0>(hit));

					m_armor_hit_effect.set_origin(m);
					//m_armor_hit_effect.set_base_velocity(std::get<1>(hit));
					m_armor_hit_effect.spawn_once(25);
				}
				m_frame_armor_hits.clear();

				m_shield_hit_effect.update(dt);
				m_armor_hit_effect.update(dt);
			}

			// if we have shields, make collisions "bouncier" by increasing restitution
			auto& collider = m_registry.get<physics::collider>(m_entity);
			collider.set_restitution(m_health.has_shield() ? m_player_shield_restitution : m_player_armor_restitution);
			collider.set_shape({ physics::sphere_shape{ m_health.has_shield() ? m_player_shield_radius_m : m_player_ship_radius_m } });
		}

		void player::render_scene(gl::renderer& renderer, camera_matrices const& matrices)
		{
			ZoneScopedN("player::render_scene()");

			m_ship_renderable.render(renderer, matrices);

			// if (m_health.has_shield())
			// 	m_shield_renderable.render(renderer, matrices);
		}

		void player::render_particles(gl::renderer& renderer, camera_matrices const& matrices)
		{
			ZoneScopedN("player::render_particles()");

			m_weapons.render(renderer, matrices);

			{
				ZoneScopedN("player::render_particles() - engine boost effects");

				m_left_engine_boost_effect.render(renderer, matrices);
				m_right_engine_boost_effect.render(renderer, matrices);
			}

			{
				ZoneScopedN("player::render_particles() - shield / armor effects");
				
				m_shield_hit_effect.render(renderer, matrices);
				m_armor_hit_effect.render(renderer, matrices);
			}
		}

	} // game
	
} // bump
