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
			
			auto gbuf = lighting::gbuffers(app.m_window.get_size());
			auto shadow_rt = lighting::shadow_rendertarget(glm::ivec2{ 1920, 1080 });
			auto lighting_rt = lighting::lighting_rendertarget(app.m_window.get_size(), gbuf.m_depth_stencil);
			auto lighting = lighting::lighting_system(registry, 
				app.m_assets.m_shaders.at("light_directional"), 
				app.m_assets.m_shaders.at("light_point"), app.m_assets.m_models.at("point_light"),
				app.m_assets.m_shaders.at("light_emissive"));
			auto tone_map_blit = lighting::tone_map_quad(app.m_assets.m_shaders.at("tone_mapping"));


			// light directions from blender:
			// C.selected_objects[0].matrix_world.to_3x3() @ Vector((0.0, 0.0, -1.0)) # (and with y = z, z = -y)
			auto dir_light_1 = registry.create();
			registry.emplace<lighting::main_light_tag>(dir_light_1);
			auto& l1 = registry.emplace<lighting::directional_light>(dir_light_1);
			l1.m_direction = glm::vec3(-0.9245213270187378f, -1.4393192415695921e-08f, -0.3811303377151489f);
			l1.m_color = glm::vec3(1.00f, 0.998f, 0.629f) * 2.5f;
			auto dir_light_2 = registry.create();
			auto& l2 = registry.emplace<lighting::directional_light>(dir_light_2);
			l2.m_direction = glm::vec3(0.3472469449043274f, -0.9376746416091919f, 0.013631058856844902f);
			l2.m_color = glm::vec3(0.047f, 0.326f, 0.638f) * 0.05f;
			auto dir_light_3 = registry.create();
			auto& l3 = registry.emplace<lighting::directional_light>(dir_light_3);
			l3.m_direction = glm::vec3(0.5147935748100281f, 0.8573000431060791f, -0.004921185318380594f);
			l3.m_color = glm::vec3(0.638f, 0.359f, 0.584f) * 0.10f;

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

			auto powerups = game::powerups(registry, app.m_assets.m_shaders.at("powerup_depth"), app.m_assets.m_shaders.at("powerup"), app.m_assets.m_models.at("powerup_shield"), app.m_assets.m_models.at("powerup_armor"), app.m_assets.m_models.at("powerup_lasers"));

			auto num_asteroid_fragment_models = 15;

			auto asteroid_fragment_names = std::vector<std::string>();
			asteroid_fragment_names.reserve(num_asteroid_fragment_models);

			for (auto i = 0; i != num_asteroid_fragment_models; ++i)
				asteroid_fragment_names.push_back("asteroid_fragment_" + std::to_string(i));

			auto asteroid_fragment_models = std::vector<std::reference_wrapper<const mbp_model>>();
			asteroid_fragment_models.reserve(num_asteroid_fragment_models);

			for (auto const& n : asteroid_fragment_names)
				asteroid_fragment_models.emplace_back(app.m_assets.m_models.at(n));
			
			auto asteroids = asteroid_field(registry, powerups, app.m_assets.m_models.at("asteroid"), asteroid_fragment_models, app.m_assets.m_shaders.at("asteroid_depth"), app.m_assets.m_shaders.at("asteroid"), app.m_assets.m_shaders.at("particle_effect"));

			auto const bounds_radius = 300.f;
			auto bounds = game::bounds(registry, bounds_radius, app.m_assets.m_shaders.at("bouy_depth"),app.m_assets.m_shaders.at("bouy"), app.m_assets.m_models.at("bouy"));

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
						gbuf.recreate(size);
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

						bounds.render_scene(renderer, scene_matrices);
						asteroids.render_scene(renderer, scene_matrices);
						player.render_scene(renderer, scene_matrices);
						powerups.render_scene(renderer, scene_matrices);
					}

					renderer.set_framebuffer(shadow_rt.m_framebuffer);
					renderer.set_viewport({ 0, 0 }, glm::uvec2(shadow_rt.m_texture.get_size()));
					renderer.clear_depth_buffers();

					auto light_matrices = camera_matrices();

					// render depth for shadows
					{
						ZoneScopedN("MainLoop - Render Shadow Depth");

						// unproject screen corner to world space
						auto corner_screen = glm::vec4{ 0.f, 0.f, 1.f, 1.f };
						auto corner_clip = scene_matrices.m_inv_viewport * corner_screen;
						auto corner_world = scene_matrices.m_inv_view_projection * corner_clip;
						corner_world.x /= corner_world.w;
						corner_world.y /= corner_world.w;
						corner_world.z /= corner_world.w;
						corner_world.w = 1.f;

						// project ray from camera position through near-plane point
						auto ray_origin = get_position(scene_matrices.m_inv_view);
						auto ray_dir = glm::normalize(glm::vec3(corner_world) - ray_origin);

						// intersect ray with y = 0 plane
						auto plane_point = glm::vec3(0.f);
						auto plane_normal = glm::vec3{ 0.f, 1.f, 0.f };
						auto p1 = glm::dot(ray_origin - plane_point, plane_normal);
						auto p2 = glm::dot(ray_dir, plane_normal);
						auto corner_pos = ray_origin - (ray_dir * (p1 / p2));

						// get distance from center of screen to corner
						auto center_pos = get_position(scene_camera.m_transform);
						center_pos.y = 0;
						auto screen_radius = glm::length(corner_pos - center_pos);
						
						// set up light camera:
						// set rotation from light dir (-z), up (y) and cross product (x)
						// set position on a tangent (along the camera x axis) to the world bounds
						auto light_dir = registry.get<lighting::directional_light>(dir_light_1).m_direction;
						auto light_camera = orthographic_camera();
						auto const up = glm::vec3{ 0.f, 1.f, 0.f };
						light_camera.m_transform[0] = glm::vec4(glm::normalize(glm::cross(light_dir, up)), 0.f);
						light_camera.m_transform[1] = glm::vec4(up, 0.f);
						light_camera.m_transform[2] = -glm::vec4(light_dir, 0.f);
						translate_in_local(light_camera.m_transform, glm::vec3{ 0.f, 0.f, bounds_radius + 5.f });
						translate_in_local(light_camera.m_transform, glm::vec3{ 1.f, 0.f, 0.f} * glm::dot(glm::vec3(light_camera.m_transform[0]), center_pos));
						light_camera.m_projection.m_near = 0.f;
						light_camera.m_projection.m_far = 2.f * (bounds_radius + 5.f);
						auto scene_height = ((2.f * screen_radius) / (float)shadow_rt.m_texture.get_size().x) * (float)shadow_rt.m_texture.get_size().y;
						light_camera.m_projection.m_position = glm::vec2{ -screen_radius, -scene_height * 0.5 };
						light_camera.m_projection.m_size = glm::vec2{ 2.f * screen_radius, scene_height };
						light_camera.m_viewport.m_position = glm::vec2(0.f);
						light_camera.m_viewport.m_size = glm::vec2(shadow_rt.m_texture.get_size());

						light_matrices = camera_matrices(light_camera);

						// render scene
						renderer.set_face_culling(gl::renderer::face_culling::COUNTER_CLOCKWISE);

						//bounds.render_depth(renderer, light_matrices);
						asteroids.render_depth(renderer, light_matrices);
						player.render_depth(renderer, light_matrices);
						powerups.render_depth(renderer, light_matrices);

						renderer.set_face_culling(gl::renderer::face_culling::CLOCKWISE);
					}

					renderer.set_framebuffer(lighting_rt.m_framebuffer);
					renderer.set_viewport({ 0, 0 }, glm::uvec2(app.m_window.get_size()));
					renderer.clear_color_buffers({ 0.f, 0.f, 0.f, 1.f });

					// lighting
					{
						lighting.render(renderer, glm::vec2(app.m_window.get_size()), light_matrices, scene_matrices, ui_matrices, gbuf, shadow_rt.m_texture);
						skybox.render_scene(renderer, scene_camera, scene_matrices);
					}
					
					// render particles
					{
						asteroids.render_particles(renderer, light_matrices, scene_matrices, shadow_rt.m_texture);
						player.render_particles(renderer, light_matrices, scene_matrices, shadow_rt.m_texture);
						space_dust.render_particles(renderer, scene_matrices);
					}

					// render transparent objects
					{
						player.render_transparent(renderer, light_matrices, scene_matrices, shadow_rt.m_texture);
					}

					renderer.clear_framebuffer();
					
					renderer.set_framebuffer_color_encoding(gl::renderer::framebuffer_color_encoding::SRGB);
					renderer.clear_color_buffers({ 0.93f, 0.58f, 0.39f, 1.f });
					renderer.clear_depth_buffers();

					// blit pass (temp)
					{
						tone_map_blit.m_size = glm::vec2(app.m_window.get_size());
						tone_map_blit.render(lighting_rt.m_texture, renderer, ui_matrices);
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
