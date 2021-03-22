#include "bump_game.hpp"

#include "bump_camera.hpp"
#include "bump_die.hpp"
#include "bump_game_app.hpp"
#include "bump_game_asteroids.hpp"
#include "bump_game_debug_camera.hpp"
#include "bump_game_ecs_render.hpp"
#include "bump_game_skybox.hpp"
#include "bump_gl.hpp"
#include "bump_font.hpp"
#include "bump_log.hpp"
#include "bump_mbp_model.hpp"
#include "bump_narrow_cast.hpp"
#include "bump_physics.hpp"
#include "bump_render_text.hpp"
#include "bump_time.hpp"
#include "bump_timer.hpp"
#include "bump_transform.hpp"
#include "bump_sdl_gamepad.hpp"

#include <entt.hpp>
#include <GL/glew.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/glm.hpp>
#include <stb_image.h>
#include <stb_image_write.h>

#include <algorithm>
#include <chrono>
#include <deque>
#include <iterator>
#include <iostream>
#include <random>
#include <string>

namespace bump
{
	
	namespace game
	{
		
		class press_start_text
		{
		public:

			explicit press_start_text(font::font_asset const& font, gl::shader_program const& shader):
				m_shader(shader),
				m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
				m_u_MVP(shader.get_uniform_location("u_MVP")),
				m_u_Position(shader.get_uniform_location("u_Position")),
				m_u_Size(shader.get_uniform_location("u_Size")),
				m_u_Color(shader.get_uniform_location("u_Color")),
				m_u_TextTexture(shader.get_uniform_location("u_TextTexture")),
				m_texture(render_text_to_gl_texture(font, "Press any key to start!").m_texture),
				m_vertex_buffer(),
				m_vertex_array()
			{
				auto vertices = { 0.f, 0.f,  1.f, 0.f,  1.f, 1.f,  0.f, 0.f,  1.f, 1.f,  0.f, 1.f, };
				m_vertex_buffer.set_data(GL_ARRAY_BUFFER, vertices.begin(), 2, 6, GL_STATIC_DRAW);

				m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertex_buffer);
			}

			void render(gl::renderer& renderer, camera_matrices const& matrices, glm::vec2 window_size)
			{
				auto mvp = matrices.model_view_projection_matrix(glm::mat4(1.f));

				auto size = glm::vec2(m_texture.get_size());

				auto pos = glm::round(glm::vec2{
					(window_size.x - size.x) / 2, // center
					window_size.y / 8.f, // offset from bottom
				});

				renderer.set_blending(gl::renderer::blending::BLEND);
				
				renderer.set_program(m_shader);

				renderer.set_uniform_4x4f(m_u_MVP, mvp);
				renderer.set_uniform_2f(m_u_Position, pos);
				renderer.set_uniform_2f(m_u_Size, size);
				renderer.set_uniform_3f(m_u_Color, glm::f32vec3(1.f));
				renderer.set_uniform_1i(m_u_TextTexture, 0);

				renderer.set_texture_2d(0, m_texture);

				renderer.set_vertex_array(m_vertex_array);

				renderer.draw_arrays(GL_TRIANGLES, m_vertex_buffer.get_element_count());

				renderer.clear_vertex_array();
				renderer.clear_texture_2d(0);
				renderer.clear_program();
				renderer.set_blending(gl::renderer::blending::NONE);
			}

		private:

			gl::shader_program const& m_shader;
			GLint m_in_VertexPosition;
			GLint m_u_MVP;
			GLint m_u_Position;
			GLint m_u_Size;
			GLint m_u_Color;
			GLint m_u_TextTexture;

			gl::texture_2d m_texture;
			gl::buffer m_vertex_buffer;
			gl::vertex_array m_vertex_array;
		};
		
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

		gamestate do_start(app& app)
		{
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
			auto press_start = press_start_text(app.m_assets.m_fonts.at("press_start"), app.m_assets.m_shaders.at("press_start"));

			auto paused = false;
			auto timer = frame_timer();
			
			while (true)
			{
				
				// input
				{
					auto quit = false;
					auto enter_game = false;
					auto callbacks = input::input_callbacks();
					callbacks.m_quit = [&] () { quit = true; };
					callbacks.m_pause = [&] (bool pause) { paused = pause; if (!paused) timer = frame_timer(); };

					callbacks.m_input = [&] (input::control_id id, input::raw_input in)
					{
						if (paused) return;

						using input::control_id;

						if (id == control_id::KEYBOARDKEY_ESCAPE && in.m_value == 1.f) quit = true;
						else enter_game = true;
					};

					app.m_input_handler.poll_input(callbacks);

					if (quit)
						return { };

					if (enter_game)
						return { do_game };
				}

				// todo: update

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

						// render asteroids...
					}

					// render ui
					auto ui_camera_matrices = camera_matrices(ui_camera);
					press_start.render(app.m_renderer, ui_camera_matrices, glm::vec2(app.m_window.get_size()));

					app.m_window.swap_buffers();
				}

				timer.tick();
			}

			return { };
		}

	} // game
	
} // bump
