#include "bump_game.hpp"

#include "bump_camera.hpp"
#include "bump_game_app.hpp"
#include "bump_game_asteroids.hpp"
#include "bump_game_ecs_render.hpp"
#include "bump_game_skybox.hpp"
#include "bump_physics.hpp"
#include "bump_timer.hpp"

#include <glm/glm.hpp>

namespace bump
{
	
	namespace game
	{
		
		class player_controls
		{
		public:

			float m_boost_axis = 0.f;
			float m_pitch_axis = 0.f;
			float m_roll_axis = 0.f;
			float m_yaw_axis = 0.f;

			void apply(physics::physics_component& physics, high_res_duration_t dt)
			{
				auto dt_s = std::chrono::duration_cast<std::chrono::duration<float>>(dt).count();
				(void)dt_s;

				// apply deadzone to axes (todo: move this to input handling?)
				auto const deadzone = 0.1f;
				m_roll_axis = glm::abs(m_roll_axis) < deadzone ? 0.f : m_roll_axis;
				m_pitch_axis = glm::abs(m_pitch_axis) < deadzone ? 0.f : m_pitch_axis;

				//std::cout << m_roll_axis << " " << m_pitch_axis << std::endl;

				auto transform = physics.get_transform();
				auto const world_right = right(transform);
				auto const world_up = up(transform);
				auto const world_forward = forwards(transform);
				
				// apply boost
				auto const boost_force_N = 2500.f;
				physics.add_force(world_forward * boost_force_N * m_boost_axis);

				// apply pitch
				auto const pitch_torque = 1000.f;
				physics.add_torque(world_right * pitch_torque * m_pitch_axis);

				// apply roll
				auto const roll_torque = 1000.f;
				physics.add_torque(world_forward * roll_torque * m_roll_axis);
								
				// apply yaw
				auto const yaw_torque = 500.f;
				physics.add_torque(-world_up * yaw_torque * m_yaw_axis);

				auto const rho_kg_per_m3 = 1.255f; // density of air

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


				// boost:
					// todo: judder? (smooth, judder at mid speeds, smooth at high speeds).
			}

		};

		gamestate do_game(app& app)
		{
			auto registry = entt::registry();
			auto physics_system = physics::physics_system();

			auto scene_camera = perspective_camera();
			scene_camera.m_transform = glm::translate(glm::mat4(1.f), { 0.f, 0.f, 50.f });

			auto ui_camera = orthographic_camera();

			{
				auto const size = glm::vec2(app.m_window.get_size());
				scene_camera.m_projection.m_size = size;
				scene_camera.m_viewport.m_size = size;
				ui_camera.m_projection.m_size = size;
				ui_camera.m_viewport.m_size = size;
			}
			
			auto skybox = game::skybox(app.m_assets.m_models.at("skybox"), app.m_assets.m_shaders.at("skybox"), app.m_assets.m_cubemaps.at("skybox"));

			auto player = registry.create();
			{
				registry.emplace<ecs::basic_renderable>(player, app.m_assets.m_models.at("player_ship"), app.m_assets.m_shaders.at("player_ship"));
				auto& player_physics = registry.emplace<physics::physics_component>(player);
				player_physics.set_mass(20.f);

				player_physics.set_local_inertia_tensor(physics::make_sphere_inertia_tensor(20.f, 10.f));
				player_physics.set_linear_damping(0.998f);
				player_physics.set_angular_damping(0.998f);

				auto& player_collision = registry.emplace<physics::collision_component>(player);
				player_collision.set_shape({ physics::sphere_shape{ 5.f } });
			}

			auto controls = player_controls();

			auto asteroids = asteroid_field(registry, app.m_assets.m_models.at("asteroid"), app.m_assets.m_shaders.at("asteroid"));

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

						if (id == control_id::GAMEPADTRIGGER_LEFT) controls.m_boost_axis = in.m_value;
						else if (id == control_id::GAMEPADSTICK_LEFTY) controls.m_pitch_axis = in.m_value;
						else if (id == control_id::GAMEPADSTICK_LEFTX) controls.m_roll_axis = in.m_value;
						else if (id == control_id::KEYBOARDKEY_SPACE) controls.m_boost_axis = in.m_value;
						else if (id == control_id::KEYBOARDKEY_W) controls.m_pitch_axis = -in.m_value;
						else if (id == control_id::KEYBOARDKEY_S) controls.m_pitch_axis = in.m_value;
						else if (id == control_id::KEYBOARDKEY_A) controls.m_roll_axis = -in.m_value;
						else if (id == control_id::KEYBOARDKEY_D) controls.m_roll_axis = in.m_value;

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
						auto& player_physics = registry.get<physics::physics_component>(player);
						controls.apply(player_physics, dt);

						// physics:
						physics_system.update(registry, dt);

						// update camera position
						auto transform = player_physics.get_transform();
						translate_in_local(transform, { 0.f, 2.f, 15.f });
						scene_camera.m_transform = transform;
						
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
					}

					// render ui
					//auto ui_camera_matrices = camera_matrices(ui_camera);
					//...

					app.m_window.swap_buffers();
				}

				timer.tick();
			}

			return { };
		}

	} // game
	
} // bump