#include "bump_game.hpp"

#include "bump_camera.hpp"
#include "bump_game_app.hpp"
#include "bump_game_basic_renderable.hpp"
#include "bump_game_debug_camera.hpp"
#include "bump_game_particle_field.hpp"
#include "bump_game_press_start_text.hpp"
#include "bump_game_skybox.hpp"
#include "bump_lighting.hpp"
#include "bump_timer.hpp"
#include "bump_transform.hpp"

#include <entt.hpp>

#include <glm/glm.hpp>

namespace bump
{
	
	namespace game
	{
		
		gamestate do_start(app& app)
		{
			//auto const& intro_music = app.m_assets.m_music.at("intro");
			//app.m_mixer_context.play_music(intro_music);
			//app.m_mixer_context.set_music_volume(MIX_MAX_VOLUME / 8);

			auto registry = entt::registry();

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
			//registry.emplace<lighting::main_light_tag>(dir_light_1);
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

			auto scene_camera = perspective_camera();
			scene_camera.m_projection.m_near = 0.1f;
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

			auto asteroid = basic_renderable(app.m_assets.m_shaders.at("start_asteroid_depth"), app.m_assets.m_shaders.at("start_asteroid"), app.m_assets.m_models.at("asteroid"));

			auto space_dust = particle_field(app.m_assets.m_shaders.at("particle_field"), 5.f, 20);
			space_dust.set_base_color_rgb({ 0.25, 0.20, 0.15 });
			space_dust.set_color_variation_hsv({ 0.05, 0.25, 0.05 });

			auto press_start = press_start_text(app.m_ft_context, app.m_assets.m_fonts.at("press_start"), app.m_assets.m_shaders.at("text_quad"), app.m_assets.m_shaders.at("press_start"));

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
						else if (id == control_id::GAMEPADSTICK_LEFTX || id == control_id::GAMEPADSTICK_LEFTY || id == control_id::GAMEPADSTICK_RIGHTX || id == control_id::GAMEPADSTICK_RIGHTY) return;
						else if (id == control_id::MOUSEPOSITION_X || id == control_id::MOUSEPOSITION_Y || id == control_id::MOUSEMOTION_X || id == control_id::MOUSEMOTION_Y) return;
						else if (id != control_id::KEYBOARDKEY_LEFTALT && id != control_id::KEYBOARDKEY_LEFTWINDOWS && id != control_id::KEYBOARDKEY_RIGHTWINDOWS)
							enter_game = true;
					};
					callbacks.m_resize = [&] (glm::ivec2 size)
					{
						gbuf.recreate(size);
						lighting_rt.recreate(size, gbuf.m_depth_stencil);
					};

					app.m_input_handler.poll_input(callbacks);

					if (quit)
						return { };

					if (enter_game)
					{
						app.m_mixer_context.stop_music();
						return { do_game };
					}
				}

				// update
				{
					auto const dt = timer.get_last_frame_time();
					auto const dt_s = high_res_duration_to_seconds(dt);

					set_position(scene_camera.m_transform, glm::vec3(0.f));

					auto axis = glm::normalize(glm::vec3(0.4f, 2.f, 0.8f));
					auto angle = 2.f * glm::pi<float>() * 0.01f;
					rotate_around_world_axis(scene_camera.m_transform, axis, angle * dt_s);

					translate_in_local(scene_camera.m_transform, glm::vec3{ 0.f, 0.f, 50.f });

					space_dust.set_position(get_position(scene_camera.m_transform));
				}

				// render
				{
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
						asteroid.render(renderer, scene_matrices);
					}

					renderer.set_framebuffer(shadow_rt.m_framebuffer);
					renderer.set_viewport({ 0, 0 }, glm::uvec2(shadow_rt.m_texture.get_size()));
					renderer.clear_depth_buffers();

					auto light_matrices = camera_matrices();

					// render depth for shadows
					{
						// ... set light matrices

						// render scene depth
						//asteroid.render_depth(renderer, light_matrices);
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
						space_dust.render_particles(renderer, scene_matrices);
					}

					renderer.clear_framebuffer();
					
					renderer.set_framebuffer_color_encoding(gl::renderer::framebuffer_color_encoding::SRGB);
					renderer.clear_color_buffers({ 0.93f, 0.58f, 0.39f, 1.f });
					renderer.clear_depth_buffers();

					// blit to backbuffer (w/ tonemapping)
					{
						tone_map_blit.m_size = glm::vec2(app.m_window.get_size());
						tone_map_blit.render(lighting_rt.m_texture, renderer, ui_matrices);
					}

					renderer.set_framebuffer_color_encoding(gl::renderer::framebuffer_color_encoding::RGB);

					// render ui
					{
						press_start.render(renderer, ui_matrices, glm::vec2(app.m_window.get_size()), paused ? high_res_duration_t{ 0 } : timer.get_last_frame_time());
					}

					app.m_window.swap_buffers();
				}

				timer.tick();
			}

			return { };
		}
		
	} // game
	
} // bump
