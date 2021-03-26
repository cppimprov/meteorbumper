#include "bump_game.hpp"

#include "bump_camera.hpp"
#include "bump_game_app.hpp"
#include "bump_game_asteroids.hpp"
#include "bump_game_crosshair.hpp"
#include "bump_game_ecs_render.hpp"
#include "bump_game_particle_field.hpp"
#include "bump_game_skybox.hpp"
#include "bump_physics.hpp"
#include "bump_timer.hpp"

#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <iostream>

namespace bump
{
	
	namespace game
	{

		class player_controls
		{
		public:

			float m_boost_axis = 0.f;
			float m_vertical_axis = 0.f;
			float m_horizontal_axis = 0.f;

			bool m_mouse_update = false;
			bool m_controller_update = false;

			glm::vec2 m_mouse_motion = glm::vec2(0.f);
			glm::vec2 m_controller_position = glm::vec2(0.f);
			
			void apply(physics::physics_component& physics, crosshair& crosshair, glm::vec2 window_size, camera_matrices const& matrices)
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

		};

		class player_lasers
		{
		public:

			explicit player_lasers(gl::shader_program const& shader);
			
			void update(entt::registry& registry, bool fire, glm::mat4 const& player_transform, high_res_duration_t dt);
			void render(gl::renderer& renderer, entt::registry& registry, camera_matrices const& matrices);

		private:

			gl::shader_program const& m_shader;
			GLint m_in_VertexPosition;
			GLint m_in_Color;
			GLint m_in_Position;
			GLint m_in_Direction;
			GLint m_in_BeamLength;
			GLint m_u_MVP;

			gl::buffer m_vertices;
			gl::buffer m_instance_color;
			gl::buffer m_instance_position;
			gl::buffer m_instance_direction;
			gl::buffer m_instance_beam_length;
			gl::vertex_array m_vertex_array;

			struct beam_instance_data
			{
				glm::vec3 m_color;
				glm::vec3 m_position;
				glm::vec3 m_direction;
				float m_beam_length;
			};

			std::vector<beam_instance_data> m_instance_data;

			struct beam
			{
				entt::entity m_entity; // physics / collision components
				float m_beam_length;
				high_res_duration_t m_lifetime;
			};

			struct emitter
			{
				glm::vec3 m_color;
				glm::vec3 m_origin; // in player model space
				high_res_duration_t m_max_lifetime;
				std::vector<beam> m_beams;
			};

			high_res_duration_t m_firing_period;
			high_res_duration_t m_time_since_firing;

			float m_beam_speed_m_per_s;
			float m_beam_length_factor; // max_beam_length = beam_speed * firing_period * length_factor;

			std::vector<emitter> m_emitters;
		};

		player_lasers::player_lasers(gl::shader_program const& shader):
			m_shader(shader),
			m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
			m_in_Color(shader.get_attribute_location("in_Color")),
			m_in_Position(shader.get_attribute_location("in_Position")),
			m_in_Direction(shader.get_attribute_location("in_Direction")),
			m_in_BeamLength(shader.get_attribute_location("in_BeamLength")),
			m_u_MVP(shader.get_uniform_location("u_MVP")),
			m_firing_period(std::chrono::duration_cast<high_res_duration_t>(std::chrono::duration<float>(0.125f))),
			m_time_since_firing(m_firing_period),
			m_beam_speed_m_per_s(100.f),
			m_beam_length_factor(0.5f)
		{
			// set up vertex buffer: one instanced vertex, w/ geometry shader to make lines
			auto vertices = { 0.f, 0.f, 0.f };
			m_vertices.set_data(GL_ARRAY_BUFFER, vertices.begin(), 3, 1, GL_STATIC_DRAW);
			m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertices);

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
				{ 0.f, 1.f, 0.f }, // green
				{ -2.f, -0.1f, -1.f }, // origin
				std::chrono::duration_cast<high_res_duration_t>(std::chrono::duration<float>(1.f)),
				{},
			};

			m_emitters.push_back(std::move(left_emitter));
			
			auto right_emitter = emitter
			{
				{ 0.f, 1.f, 0.f }, // green
				{ 2.f, -0.1f, -1.f }, // origin
				std::chrono::duration_cast<high_res_duration_t>(std::chrono::duration<float>(1.f)),
				{},
			};

			m_emitters.push_back(std::move(right_emitter));
		}

		void player_lasers::update(entt::registry& registry, bool fire, glm::mat4 const& player_transform, high_res_duration_t dt)
		{
			m_time_since_firing += dt;

			if (fire && m_time_since_firing >= m_firing_period)
			{
				std::cout << "pew" << std::endl;

				// add a new beam to each emitter
				for (auto& emitter : m_emitters)
				{
					auto beam_entity = registry.create();

					auto& beam_physics = registry.emplace<physics::physics_component>(beam_entity);
					beam_physics.set_mass(0.1f);
					beam_physics.set_local_inertia_tensor(physics::make_sphere_inertia_tensor(0.1f, 0.1f));
					beam_physics.set_position(transform_point_to_world(player_transform, emitter.m_origin));
					beam_physics.set_velocity(forwards(player_transform) * m_beam_speed_m_per_s);

					auto& beam_collision = registry.emplace<physics::collision_component>(beam_entity);
					beam_collision.set_shape({ physics::sphere_shape{ 0.1f } }); // todo: line!

					emitter.m_beams.push_back({ beam_entity, 0.f, high_res_duration_t{ 0 } });
				}

				// reset firing timer
				m_time_since_firing = high_res_duration_t{ 0 };
			}

			auto const dt_s = std::chrono::duration_cast<std::chrono::duration<float>>(dt).count();
			auto const firing_period_s = std::chrono::duration_cast<std::chrono::duration<float>>(m_firing_period).count();
			auto const max_beam_length = m_beam_speed_m_per_s * firing_period_s * m_beam_length_factor;

			for (auto& emitter : m_emitters)
			{
				// update the length of the most recently fired beam
				if (!emitter.m_beams.empty())
				{
					auto& first = emitter.m_beams.front();

					if (first.m_beam_length < max_beam_length)
					{
						first.m_beam_length += m_beam_speed_m_per_s * dt_s;
						first.m_beam_length = std::min(first.m_beam_length, max_beam_length);
					}
				}

				// update beam lifetimes
				for (auto& beam : emitter.m_beams)
					beam.m_lifetime += dt;
				
				// find and remove dead beams
				auto first_dead_beam = std::remove_if(emitter.m_beams.begin(), emitter.m_beams.end(),
						[&] (beam const& b) { return b.m_lifetime > emitter.m_max_lifetime; });
				
				for (auto b = first_dead_beam; b != emitter.m_beams.end(); ++b)
				{
					std::cout << "fwip" << std::endl;
					registry.destroy(b->m_entity);
				}

				emitter.m_beams.erase(first_dead_beam, emitter.m_beams.end());
			}
		}

		void player_lasers::render(gl::renderer& renderer, entt::registry& registry, camera_matrices const& matrices)
		{
			(void)renderer;
			(void)registry;
			(void)matrices;

			// get beam instance data

			// upload instance data to buffers

			// set shader
			// set uniforms
			// set vertex array

			// draw
			
			// clear stuff
		}

		class player_weapons
		{
		public:

			explicit player_weapons(gl::shader_program const& laser_shader);

			void update(entt::registry& registry, bool fire, glm::mat4 const& player_transform, high_res_duration_t dt)
			{
				m_lasers.update(registry, fire, player_transform, dt);
			}

			player_lasers m_lasers;
		};

		player_weapons::player_weapons(gl::shader_program const& laser_shader):
			m_lasers(laser_shader)
			{ }

		class player
		{
		public:

			explicit player(entt::registry& registry, assets& assets);

			void update(entt::registry& registry, high_res_duration_t dt);

			entt::entity m_entity;

			player_controls m_controls;
			player_weapons m_weapons;
		};

		player::player(entt::registry& registry, assets& assets):
			m_entity(entt::null_t()),
			m_controls(),
			m_weapons(assets.m_shaders.at("player_laser"))
		{
			m_entity = registry.create();

			registry.emplace<ecs::basic_renderable>(m_entity, assets.m_models.at("player_ship"), assets.m_shaders.at("player_ship"));
			auto& player_physics = registry.emplace<physics::physics_component>(m_entity);
			player_physics.set_mass(20.f);

			player_physics.set_local_inertia_tensor(physics::make_sphere_inertia_tensor(20.f, 10.f));
			player_physics.set_linear_damping(0.998f);
			player_physics.set_angular_damping(0.998f);

			auto& player_collision = registry.emplace<physics::collision_component>(m_entity);
			player_collision.set_shape({ physics::sphere_shape{ 5.f } });
		}

		void player::update(entt::registry& registry, high_res_duration_t dt)
		{
			auto const& physics = registry.get<physics::physics_component>(m_entity);

			m_weapons.update(registry, true, physics.get_transform(), dt);
		}

		gamestate do_game(app& app)
		{
			auto registry = entt::registry();
			auto physics_system = physics::physics_system();

			auto const camera_height = 150.f;
			auto scene_camera = perspective_camera();

			{
				auto translation = glm::translate(glm::mat4(1.f), { 0.f, camera_height, 0.f });
				auto rotation = glm::rotate(glm::mat4(1.f), glm::radians(-90.f), { 1.f, 0.f, 0.f });
				scene_camera.m_transform = rotation * translation;
			}

			auto ui_camera = orthographic_camera();

			{
				auto const size = glm::vec2(app.m_window.get_size());
				scene_camera.m_projection.m_size = size;
				scene_camera.m_viewport.m_size = size;
				ui_camera.m_projection.m_size = size;
				ui_camera.m_viewport.m_size = size;
			}
			
			auto skybox = game::skybox(app.m_assets.m_models.at("skybox"), app.m_assets.m_shaders.at("skybox"), app.m_assets.m_cubemaps.at("skybox"));

			auto player = game::player(registry, app.m_assets);

			auto asteroids = asteroid_field(registry, app.m_assets.m_models.at("asteroid"), app.m_assets.m_shaders.at("asteroid"));

			auto particles = particle_field(app.m_assets.m_shaders.at("particle_field"), 25.f, 20);
			particles.set_base_color_rgb({ 0.75, 0.60, 0.45 });
			particles.set_color_variation_hsv({ 0.05, 0.25, 0.05 });

			auto crosshair = game::crosshair(app.m_assets.m_shaders.at("crosshair"));
			crosshair.m_position = glm::vec2
			{
				(float)app.m_window.get_size().x / 2.f,
				(float)app.m_window.get_size().y * (5.f / 8.f),
			};

			auto paused = false;
			auto timer = frame_timer();

			while (true)
			{
				// input
				{
					auto quit = false;
					auto callbacks = input::input_callbacks();
					callbacks.m_quit = [&] () { quit = true; };
					callbacks.m_pause = [&] (bool pause) { paused = pause; if (!paused) timer = frame_timer(); };

					callbacks.m_input = [&] (input::control_id id, input::raw_input in)
					{
						if (paused) return;

						using input::control_id;

						if (id == control_id::GAMEPADTRIGGER_LEFT) player.m_controls.m_boost_axis = in.m_value;
						else if (id == control_id::GAMEPADSTICK_LEFTX) player.m_controls.m_horizontal_axis = in.m_value;
						else if (id == control_id::GAMEPADSTICK_LEFTY) player.m_controls.m_vertical_axis = -in.m_value;
						else if (id == control_id::GAMEPADSTICK_RIGHTX) { player.m_controls.m_controller_position.x = in.m_value; player.m_controls.m_controller_update = true; }
						else if (id == control_id::GAMEPADSTICK_RIGHTY) { player.m_controls.m_controller_position.y = -in.m_value; player.m_controls.m_controller_update = true; }

						else if (id == control_id::KEYBOARDKEY_W) player.m_controls.m_vertical_axis = in.m_value;
						else if (id == control_id::KEYBOARDKEY_S) player.m_controls.m_vertical_axis = -in.m_value;
						else if (id == control_id::KEYBOARDKEY_A) player.m_controls.m_horizontal_axis = -in.m_value;
						else if (id == control_id::KEYBOARDKEY_D) player.m_controls.m_horizontal_axis = in.m_value;
						else if (id == control_id::KEYBOARDKEY_SPACE) player.m_controls.m_boost_axis = in.m_value;
						else if (id == control_id::MOUSEMOTION_X) { player.m_controls.m_mouse_motion.x = (in.m_value / app.m_window.get_size().x); player.m_controls.m_mouse_update = true; }
						else if (id == control_id::MOUSEMOTION_Y) { player.m_controls.m_mouse_motion.y = (in.m_value / app.m_window.get_size().y); player.m_controls.m_mouse_update = true; }

						else if (id == control_id::KEYBOARDKEY_ESCAPE && in.m_value == 1.f) quit = true;
					};

					app.m_input_handler.poll_input(callbacks);

					if (quit)
						return { };
				}

				// update
				{
					auto dt = timer.get_last_frame_time();

					if (!paused)
					{
						// apply player input:
						auto& player_physics = registry.get<physics::physics_component>(player.m_entity);
						player.m_controls.apply(player_physics, crosshair, glm::vec2(app.m_window.get_size()), camera_matrices(scene_camera));

						// physics:
						physics_system.update(registry, dt);

						// update player state:
						player.update(registry, dt);

						// update camera position
						auto player_position = get_position(player_physics.get_transform());
						set_position(scene_camera.m_transform, player_position + glm::vec3{ 0.f, camera_height, 0.f });
						
						// update particle field position
						particles.set_position(get_position(scene_camera.m_transform));
						
						// update basic_renderable transforms for physics objects
						{
							auto view = registry.view<physics::physics_component, ecs::basic_renderable>();
							for (auto id : view)
								view.get<ecs::basic_renderable>(id).set_transform(view.get<physics::physics_component>(id).get_transform());
						}

						// update asteroid transforms
						asteroids.update(registry);
					}
				}

				// render
				{
					app.m_renderer.clear_color_buffers({ 1.f, 0.f, 0.f, 1.f });
					app.m_renderer.clear_depth_buffers();

					{ 
						auto const size = glm::vec2(app.m_window.get_size());
						scene_camera.m_projection.m_size = size;
						scene_camera.m_viewport.m_size = size;
						ui_camera.m_projection.m_size = size;
						ui_camera.m_viewport.m_size = size;
					}

					app.m_renderer.set_viewport({ 0, 0 }, glm::uvec2(app.m_window.get_size()));
					
					// render scene
					{
						auto scene_camera_matrices = camera_matrices(scene_camera);

						// render skybox
						skybox.render(app.m_renderer, scene_camera, scene_camera_matrices);

						// render basic renderables...
						{
							auto view = registry.view<ecs::basic_renderable>();

							for (auto id : view)
								view.get<ecs::basic_renderable>(id).render(app.m_renderer, scene_camera_matrices);
						}
						
						asteroids.render(registry, app.m_renderer, scene_camera_matrices);
						
						particles.render(app.m_renderer, scene_camera_matrices);
					}

					// render ui
					auto ui_camera_matrices = camera_matrices(ui_camera);
					crosshair.render(app.m_renderer, ui_camera_matrices);

					app.m_window.swap_buffers();
				}

				timer.tick();
			}

			return { };
		}

	} // game
	
} // bump
