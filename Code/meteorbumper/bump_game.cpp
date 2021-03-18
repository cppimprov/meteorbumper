#include "bump_game.hpp"

#include "bump_camera.hpp"
#include "bump_die.hpp"
#include "bump_game_app.hpp"
#include "bump_game_asteroids.hpp"
#include "bump_game_debug_camera.hpp"
#include "bump_game_ecs_physics.hpp"
#include "bump_game_ecs_render.hpp"
#include "bump_game_skybox.hpp"
#include "bump_gl.hpp"
#include "bump_font.hpp"
#include "bump_log.hpp"
#include "bump_mbp_model.hpp"
#include "bump_narrow_cast.hpp"
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

			float m_roll_axis = 0.f;
			float m_pitch_axis = 0.f;
			float m_boost_axis = 0.f;

			void apply(ecs::physics_component& physics, high_res_duration_t dt)
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
				auto const boost_force_N = 20000.f;
				physics.add_force(world_forward * boost_force_N * m_boost_axis);

				// apply roll!
				auto const roll_torque = 10000.f;
				physics.add_torque(world_forward * roll_torque * m_roll_axis);
				
				// apply pitch!
				auto const pitch_torque = 10000.f;
				physics.add_torque(world_right * pitch_torque * m_pitch_axis);

				// linear drag
				{
					// Fd (drag force) = 0.5 * rho * v^2 * Cd * A, where:
					// rho = fluid mass density (mass / volume)
					// v = speed relative to fluid
					// Cd = drag coefficient (based on shape of object)
					// A = cross-sectional area

					auto const rho_kg_per_m3 = 1.255f; // density of air
					auto const v_m_per_s = transform_vector_to_local(transform, physics.get_velocity()); // in local space
					auto const Cd = glm::vec3{ 0.6f, 1.5f, 0.05f }; // unitless multipliers over each primary axis
					auto const A_m2 = glm::vec3{ 4.f, 20.f, 2.5f }; // very approx surface area

					auto Fd = 0.5f * rho_kg_per_m3 * v_m_per_s * v_m_per_s * Cd * A_m2; // in local space, always positive
					Fd *= -glm::sign(v_m_per_s); // apply in opposite direction to velocity

					//std::cout << glm::to_string(v_m_per_s) << " " << glm::to_string(Fd) << std::endl;
					physics.add_force(transform_vector_to_world(transform, Fd));
				}

				// rotational drag
				{
					// Td (drag torque) = rho * (2 * pi * w * r)^2 * A * r * Cd, where:
					// rho = fluid mass density (mass / volume)
					// w = angular velocity relative to fluid
					// r = radius
					// A = effective area
					// Cd = drag coefficient (based on object shape)

					// for this implementation:
					// Ar = area multiplied by radius
					// Cr = distance from axis (multiplier for speed)

					// todo: fix pitch!!!!!

					auto const rho_kg_per_m3 = 1.255f;
					auto const w_per_s = transform_vector_to_local(transform, physics.get_angular_velocity()); // in local space
					auto const Cr_m = glm::vec3{ 1.75f, 1.f, 2.f }; // very approx average radius for surface around each axis
					auto const Cd = glm::vec3{ 1.2f, 0.4f, 0.75f }; // unitless shape multipliers
					auto const Ar_m3 = glm::vec3{ 40.f, 24.f, 28.f }; // very approx surface area * radius

					auto Td = rho_kg_per_m3 * glm::pow(2.f * glm::pi<float>() * w_per_s * Cr_m, glm::vec3(2.f)) * Ar_m3 * Cd;
					Td *= -glm::sign(w_per_s);

					std::cout << glm::to_string(w_per_s) << " " << glm::to_string(Td) << std::endl;
					physics.add_torque(transform_vector_to_world(transform, Td));
				}

				// todo: some kind of translation of "pancake" movement into forward movement?

				// boost:
					// todo: judder? (smooth, judder at mid speeds, smooth at high speeds).
			}

		};

		gamestate do_start(app& app)
		{
			app.m_window.set_cursor_mode(sdl::window::cursor_mode::RELATIVE);

			auto registry = entt::registry();
			auto physics_system = ecs::physics_system();

			auto scene_camera = perspective_camera();
			scene_camera.m_transform = glm::translate(glm::mat4(1.f), { 0.f, 0.f, 10.f });

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
				auto& player_physics = registry.emplace<ecs::physics_component>(player);
				player_physics.set_mass(4000.f);
				player_physics.set_local_inertia_tensor(ecs::make_sphere_inertia_tensor(4000.f, 3.f));
			}

			auto controls = player_controls();

			auto asteroids = asteroid_field(registry, app.m_assets.m_models.at("asteroid"), app.m_assets.m_shaders.at("asteroid"));

			auto press_start = press_start_text(app.m_assets.m_fonts.at("press_start"), app.m_assets.m_shaders.at("press_start"));

			auto paused = false;
			auto timer = frame_timer();

			std::vector<sdl::gamepad> gamepads;

			while (true)
			{
				// input
				{
					SDL_Event e;

					while (SDL_PollEvent(&e))
					{
						// quitting:
						if (e.type == SDL_QUIT)
						{
							app.m_window.set_cursor_mode(sdl::window::cursor_mode::FREE);
							return { };
						}

						if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)
						{
							app.m_window.set_cursor_mode(sdl::window::cursor_mode::FREE);
							return { };
						}

						if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN && (e.key.keysym.mod & KMOD_LALT) != 0)
						{
							auto mode = app.m_window.get_display_mode();
							using display_mode = sdl::window::display_mode;
							
							if (mode != display_mode::FULLSCREEN)
								app.m_window.set_display_mode(mode == display_mode::BORDERLESS_WINDOWED ? display_mode::WINDOWED : display_mode::BORDERLESS_WINDOWED);
							
							continue;
						}

						// window focus and grabbing:
						if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
						{
							paused = true;

							app.m_window.set_cursor_mode(sdl::window::cursor_mode::FREE);
							continue;
						}
						if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
						{
							app.m_window.set_cursor_mode(sdl::window::cursor_mode::RELATIVE);

							paused = false;
							timer = frame_timer();
							continue;
						}

						// controller connection / disconnection:
						if (e.type == SDL_CONTROLLERDEVICEADDED)
						{
							gamepads.push_back(sdl::gamepad(e.cdevice.which));
							continue;
						}
						if (e.type == SDL_CONTROLLERDEVICEREMOVED)
						{
							auto entry = std::find_if(gamepads.begin(), gamepads.end(),
								[&] (sdl::gamepad const& p) { return p.get_joystick_id() == e.cdevice.which; });
							
							die_if(entry == gamepads.end());

							gamepads.erase(entry);
							continue;
						}
						// todo: how to handle SDL_CONTROLLERDEVICEREMAPPED events?

						// player input:
						if (!paused)
						{
							if (e.type == SDL_CONTROLLERAXISMOTION && e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT)
								controls.m_boost_axis = (float)e.caxis.value / 32767;
							else if (e.type == SDL_CONTROLLERAXISMOTION && e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
								controls.m_roll_axis = e.caxis.value > 0 ? (float)e.caxis.value / 32767.f : (float)e.caxis.value / 32768.f;
							else if (e.type == SDL_CONTROLLERAXISMOTION && e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
								controls.m_pitch_axis = e.caxis.value > 0 ? (float)e.caxis.value / 32767.f : (float)e.caxis.value / 32768.f;
						}

						// if (!paused)
						// {
						// 	if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_w)
						// 	{
						// 		controls.m_pitch_down = true;
						// 		continue;
						// 	}
						// 	if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_w)
						// 	{
						// 		controls.m_pitch_down = false;
						// 		continue;
						// 	}
						// 	if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_s)
						// 	{
						// 		controls.m_pitch_up = true;
						// 		continue;
						// 	}
						// 	if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_s)
						// 	{
						// 		controls.m_pitch_up = false;
						// 		continue;
						// 	}
						// 	if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_a)
						// 	{
						// 		controls.m_roll_left = true;
						// 		continue;
						// 	}
						// 	if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_a)
						// 	{
						// 		controls.m_roll_left = false;
						// 		continue;
						// 	}
						// 	if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_d)
						// 	{
						// 		controls.m_roll_right = true;
						// 		continue;
						// 	}
						// 	if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_d)
						// 	{
						// 		controls.m_roll_right = false;
						// 		continue;
						// 	}
						// 	if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE)
						// 	{
						// 		controls.m_boost = true;
						// 		continue;
						// 	}
						// 	if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_SPACE)
						// 	{
						// 		controls.m_boost = false;
						// 		continue;
						// 	}
						// }
					}
				}

				// update
				{
					auto dt = timer.get_last_frame_time();

					if (!paused)
					{
						// apply player input:
						auto& player_physics = registry.get<ecs::physics_component>(player);
						controls.apply(player_physics, dt);

						// physics:
						physics_system.update(registry, dt);

						// update camera position
						auto transform = player_physics.get_transform();
						translate_in_local(transform, { 0.f, 3.f, 50.f });
						scene_camera.m_transform = transform;
						
						// update basic_renderable transforms for physics objects
						{
							auto view = registry.view<ecs::physics_component, ecs::basic_renderable>();
							for (auto id : view)
								view.get<ecs::basic_renderable>(id).set_transform(view.get<ecs::physics_component>(id).get_transform());
						}
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
