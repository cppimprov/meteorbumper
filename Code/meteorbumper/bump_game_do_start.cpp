#include "bump_game.hpp"

#include "bump_camera.hpp"
#include "bump_game_app.hpp"
#include "bump_game_debug_camera.hpp"
#include "bump_game_particle_field.hpp"
#include "bump_game_press_start_text.hpp"
#include "bump_game_skybox.hpp"
#include "bump_timer.hpp"
#include "bump_transform.hpp"

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

			auto scene_camera = perspective_camera();
			scene_camera.m_projection.m_near = 0.1f;
			scene_camera.m_transform = glm::translate(glm::mat4(1.f), { 0.f, 0.f, 0.f });

			//auto debug_cam = debug_camera_controls();

			auto ui_camera = orthographic_camera();

			{
				auto const size = glm::vec2(app.m_window.get_size());
				scene_camera.m_projection.m_size = size;
				scene_camera.m_viewport.m_size = size;
				ui_camera.m_projection.m_size = size;
				ui_camera.m_viewport.m_size = size;
			}
			
			auto skybox = game::skybox(app.m_assets.m_models.at("skybox"), app.m_assets.m_shaders.at("skybox"), app.m_assets.m_cubemaps.at("skybox"));

			auto particles = particle_field(app.m_assets.m_shaders.at("particle_field"), 5.f, 20);
			particles.set_base_color_rgb({ 0.75, 0.60, 0.45 });
			particles.set_color_variation_hsv({ 0.05, 0.25, 0.05 });

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

						// if (id == control_id::MOUSEMOTION_X)
						// 	debug_cam.m_rotate.x -= in.m_value;
						// if (id == control_id::MOUSEMOTION_Y)
						// 	debug_cam.m_rotate.y += in.m_value;
						// if (id == control_id::KEYBOARDKEY_W)
						// 	debug_cam.m_move_forwards = (bool)in.m_value;
						// if (id == control_id::KEYBOARDKEY_S)
						// 	debug_cam.m_move_backwards = (bool)in.m_value;
						// if (id == control_id::KEYBOARDKEY_A)
						// 	debug_cam.m_move_left = (bool)in.m_value;
						// if (id == control_id::KEYBOARDKEY_D)
						// 	debug_cam.m_move_right = (bool)in.m_value;
						// if (id == control_id::KEYBOARDKEY_LEFTSHIFT)
						// 	debug_cam.m_move_fast = (bool)in.m_value;

						if (id == control_id::KEYBOARDKEY_ESCAPE && in.m_value == 1.f) quit = true;
						else if (id == control_id::GAMEPADSTICK_LEFTX || id == control_id::GAMEPADSTICK_LEFTY || id == control_id::GAMEPADSTICK_RIGHTX || id == control_id::GAMEPADSTICK_RIGHTY) return;
						else if (id == control_id::MOUSEPOSITION_X || id == control_id::MOUSEPOSITION_Y || id == control_id::MOUSEMOTION_X || id == control_id::MOUSEMOTION_Y) return;
						else enter_game = true;
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
					//debug_cam.update(timer.get_last_frame_time());
					//scene_camera.m_transform = debug_cam.m_transform;

					particles.set_position(get_position(scene_camera.m_transform));
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

						particles.render(app.m_renderer, scene_camera_matrices);

						// ...
					}

					// render ui
					auto ui_camera_matrices = camera_matrices(ui_camera);
					press_start.render(app.m_renderer, ui_camera_matrices, glm::vec2(app.m_window.get_size()), paused ? high_res_duration_t{ 0 } : timer.get_last_frame_time());

					app.m_window.swap_buffers();
				}

				timer.tick();
			}

			return { };
		}
		
	} // game
	
} // bump
