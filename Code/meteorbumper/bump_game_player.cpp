#include "bump_game_player.hpp"

#include "bump_game_assets.hpp"
#include "bump_game_asteroids.hpp"
#include "bump_game_crosshair.hpp"
#include "bump_game_powerups.hpp"
#include "bump_lighting.hpp"
#include "bump_physics.hpp"
#include "bump_random.hpp"

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
				m_low_damage_hit_effects.set_color_update_fn(make_color_update_fn(m_low_damage_hit_effects, low_damage_color_map));
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
				m_medium_damage_hit_effects.set_color_update_fn(make_color_update_fn(m_medium_damage_hit_effects, medium_damage_color_map));
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
				m_high_damage_hit_effects.set_color_update_fn(make_color_update_fn(m_high_damage_hit_effects, high_damage_color_map));
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

					auto& light = m_registry.emplace<lighting::point_light>(beam_entity);
					light.m_position = beam_physics.get_position();
					light.m_color = segment.m_color * 10.5f;
					light.m_radius = 15.f;

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
				auto view = m_registry.view<beam_segment, physics::rigidbody, lighting::point_light>();

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
						auto [b, rb, l] = view.get<beam_segment, physics::rigidbody, lighting::point_light>(id);

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

		void player_lasers::render(gl::renderer& renderer, camera_matrices const& light_matrices, camera_matrices const& matrices, gl::texture_2d const& shadow_map)
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
				
				renderer.set_depth_write(gl::renderer::depth_write::DISABLED);

				renderer.set_program(m_shader);
				renderer.set_uniform_4x4f(m_u_MVP, mvp);
				renderer.set_vertex_array(m_vertex_array);

				renderer.draw_arrays(GL_POINTS, 1, instance_count);

				renderer.clear_vertex_array();
				renderer.clear_program();

				renderer.set_depth_write(gl::renderer::depth_write::ENABLED);

				// clear frame instance buffers
				m_frame_instance_colors.clear();
				m_frame_instance_positions.clear();
				m_frame_instance_directions.clear();
				m_frame_instance_beam_lengths.clear();
			}

			m_low_damage_hit_effects.render(renderer, light_matrices, matrices, shadow_map);
			m_medium_damage_hit_effects.render(renderer, light_matrices, matrices, shadow_map);
			m_high_damage_hit_effects.render(renderer, light_matrices, matrices, shadow_map);
		}

		player_weapons::player_weapons(entt::registry& registry, gl::shader_program const& laser_shader, gl::shader_program const& laser_hit_shader):
			m_lasers(registry, laser_shader, laser_hit_shader)
			{ }
			
		void player_weapons::update(bool fire, glm::mat4 const& player_transform, glm::vec3 player_velocity, high_res_duration_t dt)
		{
			m_lasers.update(fire, player_transform, player_velocity, dt);
		}

		void player_weapons::render(gl::renderer& renderer, camera_matrices const& light_matrices, camera_matrices const& matrices, gl::texture_2d const& shadow_map)
		{
			m_lasers.render(renderer, light_matrices, matrices, shadow_map);
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

			if (is_alive())
			{
				if (m_time_since_last_hit > m_shield_recharge_delay)
				{
					auto const dt_s = high_res_duration_to_seconds(dt);
					m_shield_hp = std::clamp(m_shield_hp + m_shield_recharge_rate_hp_per_s * dt_s, 0.f, m_shield_max_hp);
				}
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
			m_entity(entt::null),
			m_ship_renderable(assets.m_shaders.at("player_ship_depth"), assets.m_shaders.at("player_ship"), assets.m_models.at("player_ship")),
			m_shield_renderable_lower(assets.m_shaders.at("player_shield"), assets.m_models.at("player_shield_lower")),
			m_shield_renderable_upper(assets.m_shaders.at("player_shield"), assets.m_models.at("player_shield_upper")),
			m_controls(),
			m_weapons(registry, assets.m_shaders.at("player_laser"), assets.m_shaders.at("particle_effect")),
			m_left_engine_boost_effect(registry, assets.m_shaders.at("particle_effect")),
			m_right_engine_boost_effect(registry, assets.m_shaders.at("particle_effect")),
			m_engine_light_l(entt::null),
			m_engine_light_r(entt::null),
			m_shield_hit_effect(registry, assets.m_shaders.at("particle_effect")),
			m_armor_hit_effect(registry, assets.m_shaders.at("particle_effect")),
			m_rng(std::random_device()())
		{
			m_entity = registry.create();
			registry.emplace<player_tag>(m_entity);

			// setup player physics
			{
				auto const mass_kg = 20.f;

				auto& rigidbody = registry.emplace<physics::rigidbody>(m_entity);
				rigidbody.set_mass(mass_kg);

				rigidbody.set_local_inertia_tensor(physics::make_sphere_inertia_tensor(mass_kg, m_player_ship_radius_m));
				rigidbody.set_linear_damping(0.998f);
				rigidbody.set_angular_damping(0.998f);
				rigidbody.set_linear_factor({ 1.f, 0.f, 1.f }); // restrict movement on y axis
				rigidbody.set_angular_factor({ 0.f, 0.f, 0.f }); // no rotation!

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

						// player died, turn off collision
						if (!m_health.is_alive())
						{
							m_frame_player_death = true;
							m_registry.get<physics::collider>(m_entity).set_collision_mask(0);
						}
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
					{ 0.0f, { 1.f, 0.85f, 0.42f, 1.f } },
					{ 0.4f, { 0.75f, 0.20f, 0.09f, 0.95f } },
					{ 0.7f, { 0.32f, 0.20f, 0.12f, 0.80f } },
					{ 1.0f, { 0.0f, 0.0f, 0.0f, 0.25f } },
				};

				auto const size_map = std::map<float, float>
				{
					{ 0.0f, 4.5f },
					{ 0.4f, 1.0f },
					{ 1.0f, 0.f },
				};

				auto const l_pos = glm::vec3{ -0.8f, 0.1f, 2.2f };
				auto l_mat = glm::mat4(1.f);
				set_position(l_mat, l_pos);

				m_left_engine_boost_effect.set_origin(l_mat);
				m_left_engine_boost_effect.set_base_velocity({ 0.f, 0.f, 15.f });
				m_left_engine_boost_effect.set_random_velocity({ 10.f, 10.f, 0.5f });
				m_left_engine_boost_effect.set_max_lifetime(high_res_duration_from_seconds(0.75f));
				m_left_engine_boost_effect.set_max_lifetime_random(high_res_duration_from_seconds(0.25f));
				m_left_engine_boost_effect.set_spawn_period(high_res_duration_from_seconds(1.f / 500.f));
				m_left_engine_boost_effect.set_color_update_fn(make_color_update_fn(m_left_engine_boost_effect, color_map));
				m_left_engine_boost_effect.set_size_update_fn(make_size_update_fn(m_left_engine_boost_effect, size_map));
				m_left_engine_boost_effect.set_collision_mask(physics::collision_layers::ASTEROIDS | physics::collision_layers::POWERUPS); // not player!

				auto const r_pos = glm::vec3{ 0.8f, 0.1f, 2.2f };
				auto r_mat = glm::mat4(1.f);
				set_position(r_mat, r_pos);

				m_right_engine_boost_effect.set_origin(r_mat);
				m_right_engine_boost_effect.set_base_velocity({ 0.f, 0.f, 15.f });
				m_right_engine_boost_effect.set_random_velocity({ 10.f, 10.f, 0.5f });
				m_right_engine_boost_effect.set_max_lifetime(high_res_duration_from_seconds(0.75f));
				m_right_engine_boost_effect.set_max_lifetime_random(high_res_duration_from_seconds(0.25f));
				m_right_engine_boost_effect.set_spawn_period(high_res_duration_from_seconds(1.f / 500.f));
				m_right_engine_boost_effect.set_color_update_fn(make_color_update_fn(m_right_engine_boost_effect, color_map));
				m_right_engine_boost_effect.set_size_update_fn(make_size_update_fn(m_right_engine_boost_effect, size_map));
				m_right_engine_boost_effect.set_collision_mask(physics::collision_layers::ASTEROIDS | physics::collision_layers::POWERUPS); // not player!
			}

			// set up engine lights
			{
				auto transform = m_registry.get<physics::rigidbody>(m_entity).get_transform();

				m_engine_light_l = m_registry.create();
				auto& light_l = m_registry.emplace<lighting::point_light>(m_engine_light_l);
				light_l.m_color = glm::vec3{ 1.0f, 0.577093f, 0.33323f } * 100.f;
				light_l.m_position = transform_point_to_world(transform, glm::vec3{ 0.8f, 0.1f, 2.5f });
				light_l.m_radius = 0.f;

				m_engine_light_r = m_registry.create();
				auto& light_r = m_registry.emplace<lighting::point_light>(m_engine_light_r);
				light_r.m_color = glm::vec3{ 1.0f, 0.577093f, 0.33323f } * 100.f;
				light_r.m_position = transform_point_to_world(transform, glm::vec3{ -0.8f, 0.1f, 2.5f });
				light_r.m_radius = 0.f;
			}

			// setup shield / armor hit effects
			{
				auto const shield_color_map = std::map<float, glm::vec4>
				{
					{ 0.0f, { 0.507f, 0.627f, 0.840f, 1.f } },
					{ 0.7f, { 0.507f, 0.627f, 0.840f, 0.5f } },
					{ 1.0f, { 1.0f, 1.0f, 1.0f, 0.2f } },
				};

				auto const shield_size_map = std::map<float, float>
				{
					{ 0.0f, 0.8f },
					{ 0.8f, 1.2f },
					{ 1.0f, 3.0f },
				};

				m_shield_hit_effect.set_spawn_radius(0.1f);
				m_shield_hit_effect.set_random_velocity({ 10.f, 10.f, 10.f });
				m_shield_hit_effect.set_max_lifetime(high_res_duration_from_seconds(1.0f));
				m_shield_hit_effect.set_max_lifetime_random(high_res_duration_from_seconds(0.25f));
				m_shield_hit_effect.set_color_update_fn(make_color_update_fn(m_shield_hit_effect, shield_color_map));
				m_shield_hit_effect.set_size_update_fn(make_size_update_fn(m_shield_hit_effect, shield_size_map));

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
				m_armor_hit_effect.set_color_update_fn(make_color_update_fn(m_armor_hit_effect, armor_color_map));
			}

			// setup fragment models
			{
				auto num_player_fragment_models = 6;
				for (auto i = 0; i != num_player_fragment_models; ++i)
					m_fragment_renderables.emplace_back(assets.m_shaders.at("player_ship_depth"), assets.m_shaders.at("player_ship"), assets.m_models.at("player_ship_fragment_" + std::to_string(i)));
			}
		}

		player::~player()
		{
			m_registry.destroy(m_engine_light_l);
			m_registry.destroy(m_engine_light_r);
			m_registry.destroy(m_entity);

			for (auto id : m_fragment_entities)
				m_registry.destroy(id);
		}

		void player::update(high_res_duration_t dt)
		{
			auto const player_transform = m_registry.get<physics::rigidbody>(m_entity).get_transform();
			auto const player_velocity = m_registry.get<physics::rigidbody>(m_entity).get_velocity();

			m_ship_renderable.set_transform(player_transform);
			m_shield_renderable_lower.set_transform(player_transform);
			m_shield_renderable_upper.set_transform(player_transform);

			m_weapons.update(m_controls.m_firing, player_transform, player_velocity, dt);

			m_health.update(dt);

			// update particle effects
			{
				auto const enabled = (m_health.is_alive() && (m_controls.m_boost_axis == 1.f));

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

			// update engine lights
			{
				auto const enabled = (m_health.is_alive() && (m_controls.m_boost_axis == 1.f));

				auto& light_l = m_registry.get<lighting::point_light>(m_engine_light_l);
				light_l.m_position = transform_point_to_world(player_transform, glm::vec3{ 0.8f, 0.1f, 2.5f });
				light_l.m_radius = enabled ? 10.f : 0.f;
				
				auto& light_r = m_registry.get<lighting::point_light>(m_engine_light_r);
				light_r.m_position = transform_point_to_world(player_transform, glm::vec3{ -0.8f, 0.1f, 2.5f });
				light_r.m_radius = enabled ? 10.f : 0.f;
			}

			// if we have shields, make collisions "bouncier" by increasing restitution
			if (m_health.is_alive())
			{
				auto& collider = m_registry.get<physics::collider>(m_entity);
				collider.set_restitution(m_health.has_shield() ? m_player_shield_restitution : m_player_armor_restitution);
				collider.set_shape({ physics::sphere_shape{ m_health.has_shield() ? m_player_shield_radius_m : m_player_ship_radius_m } });
			}

			// update fragments
			if (!m_health.is_alive())
			{
				if (m_frame_player_death)
				{
					m_frame_player_death = false;

					// spawn fragments:
					for (auto i = std::size_t{ 0 }; i != m_fragment_renderables.size(); ++i)
					{
						auto id = m_registry.create();

						auto& data = m_registry.emplace<player_fragment_data>(id);
						data.m_model_index = i;

						auto const mass = random::scale(m_rng, 10.f, 5.f);
						auto const size = glm::vec3(random::scale(m_rng, 3.f, 1.5f), random::scale(m_rng, 3.f, 1.5f), random::scale(m_rng, 3.f, 1.5f));

						auto const transform = player_transform * m_fragment_renderables[i].get_transform();
						auto const position = get_position(transform);
						auto const orientation = glm::quat_cast(glm::mat3(transform));

						auto const vel_direction = glm::normalize(position);
						auto const vel_magnitude = random::scale(m_rng, 15.f, 5.f);
						auto const vel_random = random::point_in_ring_3d(m_rng, 0.f, 5.f);
						auto const velocity = vel_direction * vel_magnitude + vel_random;

						auto const ang_axis = random::point_in_ring_3d(m_rng, 0.f, 1.f);
						auto const ang_magnitude = random::scale(m_rng, 2.5f, 1.f);
						auto const ang_velocity = ang_axis * ang_magnitude;

						auto& rb = m_registry.emplace<physics::rigidbody>(id);
						rb.set_mass(mass);
						rb.set_local_inertia_tensor(physics::make_cuboid_inertia_tensor(mass, size));
						rb.set_position(position);
						rb.set_orientation(orientation);
						rb.set_velocity(velocity);
						rb.set_angular_velocity(ang_velocity);

						m_fragment_entities.push_back(id);
					}
				}

				// update fragments:
				auto view = m_registry.view<player_fragment_data, physics::rigidbody>();

				for (auto id : view)
				{
					auto [d, rb] = view.get<player_fragment_data, physics::rigidbody>(id);

					m_fragment_renderables[d.m_model_index].set_transform(rb.get_transform());
				}
			}
		}

		void player::render_depth(gl::renderer& renderer, camera_matrices const& matrices)
		{
			ZoneScopedN("player::render_depth()");

			if (m_health.is_alive())
			{
				m_ship_renderable.render_depth(renderer, matrices);
			}
			else
			{
				for (auto& f : m_fragment_renderables)
					f.render_depth(renderer, matrices);
			}
		}

		void player::render_scene(gl::renderer& renderer, camera_matrices const& matrices)
		{
			ZoneScopedN("player::render_scene()");

			if (m_health.is_alive())
			{
				m_ship_renderable.render(renderer, matrices);
			}
			else
			{
				for (auto& f : m_fragment_renderables)
					f.render(renderer, matrices);
			}
		}

		void player::render_particles(gl::renderer& renderer, camera_matrices const& light_matrices, camera_matrices const& matrices, gl::texture_2d const& shadow_map)
		{
			ZoneScopedN("player::render_particles()");

			m_weapons.render(renderer, light_matrices, matrices, shadow_map);

			{
				ZoneScopedN("player::render_particles() - engine boost effects");

				m_left_engine_boost_effect.render(renderer, light_matrices, matrices, shadow_map);
				m_right_engine_boost_effect.render(renderer, light_matrices, matrices, shadow_map);
			}

			{
				ZoneScopedN("player::render_particles() - shield / armor effects");
				
				m_shield_hit_effect.render(renderer, light_matrices, matrices, shadow_map);
				m_armor_hit_effect.render(renderer, light_matrices, matrices, shadow_map);
			}
		}

		void player::render_transparent(gl::renderer& renderer, camera_matrices const& light_matrices, camera_matrices const& matrices, gl::texture_2d const& shadow_map)
		{
			ZoneScopedN("player::render_transparent()");

			if (m_health.is_alive() && m_health.has_shield())
			{
				// get directional lights:
				{
					auto view = m_registry.view<lighting::directional_light>();

					auto lights = std::vector<lighting::directional_light>();
					lights.reserve(view.size());

					auto shadows = std::vector<bool>();
					shadows.reserve(view.size());

					for (auto id : view)
					{
						lights.push_back(view.get<lighting::directional_light>(id));
						shadows.push_back(m_registry.has<lighting::main_light_tag>(id));
					}

					if (lights.size() > 3) lights.resize(3); // ick
					if (shadows.size() > 3) shadows.resize(3);

					m_shield_renderable_lower.set_directional_lights(lights, shadows);
					m_shield_renderable_upper.set_directional_lights(lights, shadows);
				}

				// get brightest point lights:
				{
					auto view = m_registry.view<lighting::point_light>();
					
					struct id_brightness
					{
						entt::entity m_id;
						float m_brightness;
					};

					auto ranking = std::vector<id_brightness>();
					ranking.reserve(view.size());

					auto const player_position = m_registry.get<physics::rigidbody>(m_entity).get_position();

					// calculate a "brightness" value from distance from the player and light color
					for (auto id : view)
					{
						auto const& l = view.get<lighting::point_light>(id);
						auto const distance = std::max(glm::distance(l.m_position, player_position), 0.0001f);
						auto const intensity = glm::length(l.m_color);
						auto const brightness = glm::clamp(1.f / (distance * distance), 0.f, 1.f) * intensity;
						// todo: don't clamp brightness??
						ranking.push_back({ id, brightness });
					}

					// find the "brightest" 5 (or so) lights
					auto const num = std::min(ranking.size(), std::size_t{ 5 });
					std::partial_sort(ranking.begin(), ranking.begin() + num, ranking.end(),
						[] (id_brightness const& a, id_brightness const& b) { return a.m_brightness > b.m_brightness; });
					
					ranking.resize(num);

					// copy those lights to a vector and pass to the renderables!
					auto lights = std::vector<lighting::point_light>();
					lights.reserve(num);

					for (auto const& b : ranking)
						lights.push_back(view.get<lighting::point_light>(b.m_id));
					
					m_shield_renderable_lower.set_point_lights(lights);
					m_shield_renderable_upper.set_point_lights(lights);
				}

				m_shield_renderable_lower.render(renderer, light_matrices, matrices, shadow_map);
				m_shield_renderable_upper.render(renderer, light_matrices, matrices, shadow_map);
			}
		}

	} // game
	
} // bump
