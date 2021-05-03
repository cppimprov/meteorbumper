#include "bump_game.hpp"

#include "bump_camera.hpp"
#include "bump_game_app.hpp"
#include "bump_game_asteroids.hpp"
#include "bump_game_bounds.hpp"
#include "bump_game_crosshair.hpp"
#include "bump_game_fps_counter.hpp"
#include "bump_game_indicators.hpp"
#include "bump_game_particle_effect.hpp"
#include "bump_game_particle_field.hpp"
#include "bump_game_player.hpp"
#include "bump_game_powerups.hpp"
#include "bump_game_skybox.hpp"
#include "bump_lighting.hpp"
#include "bump_physics.hpp"
#include "bump_timer.hpp"

#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <Tracy.hpp>

#include <iostream>

namespace bump
{
	
	namespace game
	{

		gamestate do_game(app& app)
		{
			auto registry = entt::registry();
			auto physics_system = physics::physics_system(registry);
			
			auto gbuf = lighting::gbuffers(3u, app.m_window.get_size());
			auto blit_pass = lighting::textured_quad(app.m_assets.m_shaders.at("temp_blit_renderpass"));
			auto lighting_rt = lighting::lighting_rendertarget(app.m_window.get_size(), gbuf.m_depth_stencil);
			auto lighting = lighting::lighting_system(registry, app.m_assets.m_shaders.at("light_directional"), app.m_assets.m_shaders.at("light_point"), app.m_assets.m_models.at("point_light"));

			{
				// directions from:
				// C.selected_objects[0].matrix_world.to_3x3() @ Vector((0.0, 0.0, -1.0)) in Blender, with y = z, and z = -y

				auto dir_light_1 = registry.create();
				auto& l1 = registry.emplace<lighting::directional_light>(dir_light_1);
				l1.m_direction = glm::vec3(-0.9245213270187378f, -1.4393192415695921e-08f, -0.3811303377151489f);
				l1.m_color = glm::vec3(1.00f, 0.998f, 0.629f) * 2.5f;

				auto dir_light_2 = registry.create();
				auto& l2 = registry.emplace<lighting::directional_light>(dir_light_2);
				l2.m_direction = glm::vec3(0.3472469449043274f, -0.9376746416091919f, 0.013631058856844902f);
				l2.m_color = glm::vec3(0.047f, 0.326f, 0.638f) * 0.15f;

				auto dir_light_3 = registry.create();
				auto& l3 = registry.emplace<lighting::directional_light>(dir_light_3);
				l3.m_direction = glm::vec3(0.5147935748100281f, 0.8573000431060791f, -0.004921185318380594f);
				l3.m_color = glm::vec3(0.638f, 0.359f, 0.584f) * 0.20f;
			}

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

			auto powerups = game::powerups(registry, app.m_assets.m_shaders.at("powerup"), app.m_assets.m_models.at("powerup_shield"), app.m_assets.m_models.at("powerup_armor"), app.m_assets.m_models.at("powerup_lasers"));

			auto asteroids = asteroid_field(registry, powerups, app.m_assets.m_models.at("asteroid"), app.m_assets.m_shaders.at("asteroid"), app.m_assets.m_shaders.at("particle_effect"));

			auto bounds = game::bounds(registry, 300.f);

			auto space_dust = particle_field(app.m_assets.m_shaders.at("particle_field"), 25.f, 20);
			space_dust.set_base_color_rgb({ 0.25, 0.20, 0.15 });
			space_dust.set_color_variation_hsv({ 0.05, 0.25, 0.05 });

			auto indicators = game::indicators(registry, app.m_assets.m_shaders.at("indicator"));

			auto crosshair = game::crosshair(app.m_assets.m_shaders.at("crosshair"));
			crosshair.m_position = glm::vec2
			{
				(float)app.m_window.get_size().x / 2.f,
				(float)app.m_window.get_size().y * (5.f / 8.f),
			};

			auto fps = fps_counter(app.m_ft_context, app.m_assets.m_fonts.at("fps_counter"), app.m_assets.m_shaders.at("fps_counter"));

			auto paused = false;
			auto timer = frame_timer();

			while (true)
			{
				// input
				{
					ZoneScopedN("MainLoop - Input");
					
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
						else if (id == control_id::GAMEPADSTICK_RIGHTX) { player.m_controls.m_controller_position.x = in.m_value;  player.m_controls.m_controller_update = true; }
						else if (id == control_id::GAMEPADSTICK_RIGHTY) { player.m_controls.m_controller_position.y = -in.m_value; player.m_controls.m_controller_update = true; }
						else if (id == control_id::GAMEPADTRIGGER_RIGHT) player.m_controls.m_firing = (in.m_value == 0.f ? 0.f : 1.f);

						else if (id == control_id::KEYBOARDKEY_W) player.m_controls.m_vertical_axis = in.m_value;
						else if (id == control_id::KEYBOARDKEY_S) player.m_controls.m_vertical_axis = -in.m_value;
						else if (id == control_id::KEYBOARDKEY_A) player.m_controls.m_horizontal_axis = -in.m_value;
						else if (id == control_id::KEYBOARDKEY_D) player.m_controls.m_horizontal_axis = in.m_value;
						else if (id == control_id::KEYBOARDKEY_SPACE) player.m_controls.m_boost_axis = in.m_value;
						else if (id == control_id::MOUSEMOTION_X) { player.m_controls.m_mouse_motion.x += (in.m_value / app.m_window.get_size().x); player.m_controls.m_mouse_update = true; }
						else if (id == control_id::MOUSEMOTION_Y) { player.m_controls.m_mouse_motion.y += (in.m_value / app.m_window.get_size().y); player.m_controls.m_mouse_update = true; }
						else if (id == control_id::MOUSEBUTTON_LEFT) player.m_controls.m_firing = in.m_value;

						else if (id == control_id::KEYBOARDKEY_ESCAPE && in.m_value == 1.f) quit = true;
					};
					callbacks.m_resize = [&] (glm::ivec2 size)
					{
						gbuf.recreate(gbuf.m_buffers.size(), size);
						lighting_rt.recreate(size, gbuf.m_depth_stencil);
					};

					app.m_input_handler.poll_input(callbacks);

					if (quit)
						return { };
				}

				// update
				{
					ZoneScopedN("MainLoop - Update");

					auto dt = timer.get_last_frame_time();

					if (!paused)
					{
						// apply player input:
						player.m_controls.apply(registry.get<physics::rigidbody>(player.m_entity), crosshair, glm::vec2(app.m_window.get_size()), camera_matrices(scene_camera));

						// physics:
						physics_system.update(dt);

						// update player state:
						player.update(dt);

						if (!player.m_health.is_alive()) 
							return { do_start }; // player died! todo: explosion effect, game over text!

						// update camera position
						auto player_position = get_position(registry.get<physics::rigidbody>(player.m_entity).get_transform());
						set_position(scene_camera.m_transform, player_position + glm::vec3{ 0.f, camera_height, 0.f });
						
						// update particle field position
						space_dust.set_position(get_position(scene_camera.m_transform));
						
						// update asteroids
						asteroids.update(dt);
						
						// update powerups
						powerups.update(dt);

						// update indicators
						indicators.set_player_position(player_position);

						fps.update(dt);
					}
				}

				// render
				{
					ZoneScopedN("MainLoop - Render");

					{ 
						auto const size = glm::vec2(app.m_window.get_size());
						scene_camera.m_projection.m_size = size;
						scene_camera.m_viewport.m_size = size;
						ui_camera.m_projection.m_size = size;
						ui_camera.m_viewport.m_size = size;
					}
					
					auto& renderer = app.m_renderer;
					auto scene_matrices = camera_matrices(scene_camera);
					auto ui_matrices = camera_matrices(ui_camera);

					renderer.set_framebuffer(gbuf.m_framebuffer);

					renderer.clear_color_buffers({ 0.f, 0.f, 0.f, 1.f });
					renderer.clear_depth_buffers();
					renderer.set_viewport({ 0, 0 }, glm::uvec2(app.m_window.get_size()));
					
					// render scene
					{
						ZoneScopedN("MainLoop - Render Scene");

						asteroids.render_scene(renderer, scene_matrices);
						player.render_scene(renderer, scene_matrices);
						powerups.render_scene(renderer, scene_matrices);
					}

					renderer.set_framebuffer(lighting_rt.m_framebuffer);

					renderer.clear_color_buffers({ 0.f, 0.f, 0.f, 1.f });

					// lighting
					{
						lighting.render(renderer, glm::vec2(app.m_window.get_size()), scene_matrices, ui_matrices, gbuf);
						skybox.render_scene(renderer, scene_camera, scene_matrices);
					}
					
					// render particles
					{
						asteroids.render_particles(renderer, scene_matrices);
						player.render_particles(renderer, scene_matrices);
						space_dust.render_particles(renderer, scene_matrices);
					}

					// todo: transparent object pass

					renderer.clear_framebuffer();
					
					renderer.set_framebuffer_color_encoding(gl::renderer::framebuffer_color_encoding::SRGB);

					renderer.clear_color_buffers({ 0.93f, 0.58f, 0.39f, 1.f });
					renderer.clear_depth_buffers();

					// blit pass (temp)
					{
						blit_pass.m_size = glm::vec2(app.m_window.get_size());
						blit_pass.render(lighting_rt.m_texture, renderer, ui_matrices);
					}
					
					// render ui
					{
						ZoneScopedN("MainLoop - Render UI");

						indicators.render(renderer, glm::vec2(app.m_window.get_size()), scene_matrices, ui_matrices);
						crosshair.render(renderer, ui_matrices);
						fps.render(renderer, ui_matrices);
					}

					renderer.set_framebuffer_color_encoding(gl::renderer::framebuffer_color_encoding::RGB);

					{
						ZoneScopedN("MainLoop - Swap Buffers ");
						app.m_window.swap_buffers();
					}
				}

				timer.tick();

				FrameMark;
			}

			return { };
		}

	} // game
	
} // bump
