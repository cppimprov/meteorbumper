#include "bump_game.hpp"

#include "bump_camera.hpp"
#include "bump_game_app.hpp"
#include "bump_game_debug_camera.hpp"
#include "bump_game_press_start_text.hpp"
#include "bump_game_skybox.hpp"
#include "bump_timer.hpp"
#include "bump_transform.hpp"

#include <iostream>
#include <random>

#include <glm/gtx/string_cast.hpp>
#include <glm/glm.hpp>

namespace bump
{
	
	namespace game
	{
		
		
		class particle_field
		{
		public:

			explicit particle_field(gl::shader_program const& shader, float radius, std::size_t grid_size):
				m_shader(shader),
				m_radius(radius),
				m_position(0.f),
				m_base_color_rgb(1.f),
				m_color_variation_hsv(0.f),
				m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
				m_u_MVP(shader.get_uniform_location("u_MVP")),
				m_u_Radius(shader.get_uniform_location("u_Radius")),
				m_u_Offset(shader.get_uniform_location("u_Offset")),
				m_u_BaseColorRGB(shader.get_uniform_location("u_BaseColorRGB")),
				m_u_ColorVariationHSV(shader.get_uniform_location("u_ColorVariationHSV"))
			{
				die_if(grid_size == 0);

				auto particle_count = grid_size * grid_size * grid_size;
				auto cell_size = 1.f / static_cast<float>(grid_size);
				auto vertices = std::vector<glm::vec3>();
				vertices.reserve(particle_count);

				auto rng = std::mt19937(std::random_device()());
				auto dist = std::uniform_real_distribution(0.f, 1.f);

				for (auto z = std::size_t{ 0 }; z != grid_size; ++z)
				{
					for (auto y = std::size_t{ 0 }; y != grid_size; ++y)
					{
						for (auto x = std::size_t{ 0 }; x != grid_size; ++x)
						{
							auto origin = glm::vec3{ x, y, z } * cell_size;
							auto pos = glm::vec3{ dist(rng), dist(rng), dist(rng) } * cell_size;
							vertices.push_back(origin + pos);
						}
					}
				}
				
				static_assert(sizeof(glm::vec3) == sizeof(float) * 3, "Array of vec3 will not be a flat array.");
				m_vertices.set_data(GL_ARRAY_BUFFER, glm::value_ptr(vertices.front()), 3, vertices.size(), GL_STATIC_DRAW);

				m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertices);
			}

			void set_position(glm::vec3 position) { m_position = position; }
			glm::vec3 get_position() const { return m_position; }

			void set_base_color_rgb(glm::vec3 base_color_rgb) { m_base_color_rgb = base_color_rgb; }
			glm::vec3 get_base_color_rgb() const { return m_base_color_rgb; }

			void set_color_variation_hsv(glm::vec3 color_variation_hsv) { m_color_variation_hsv = color_variation_hsv; }
			glm::vec3 get_color_variation_hsv() { return m_color_variation_hsv; }

			void render(gl::renderer& renderer, camera_matrices const& matrices)
			{
				// transparency / sorting?
				
				float radius = m_radius;
				float diameter = 2.f * radius;
				glm::vec3 origin = glm::floor(m_position / diameter) * diameter;
				glm::vec3 offset = glm::mod(m_position, diameter) / diameter;

				auto m = glm::translate(glm::mat4(1.f), origin);
				auto mvp = matrices.model_view_projection_matrix(m);

				renderer.set_blending(gl::renderer::blending::BLEND);

				renderer.set_program(m_shader);

				renderer.set_uniform_4x4f(m_u_MVP, mvp);
				renderer.set_uniform_1f(m_u_Radius, m_radius);
				renderer.set_uniform_3f(m_u_Offset, offset);
				renderer.set_uniform_3f(m_u_BaseColorRGB, m_base_color_rgb);
				renderer.set_uniform_3f(m_u_ColorVariationHSV, m_color_variation_hsv);

				renderer.set_vertex_array(m_vertex_array);

				renderer.draw_arrays(GL_POINTS, m_vertices.get_element_count());

				renderer.clear_vertex_array();
				renderer.clear_program();
				renderer.set_blending(gl::renderer::blending::NONE);
			}

		private:

			gl::shader_program const& m_shader;

			float m_radius;
			glm::vec3 m_position;
			glm::vec3 m_base_color_rgb;
			glm::vec3 m_color_variation_hsv;

			GLint m_in_VertexPosition;
			GLint m_u_MVP;
			GLint m_u_Radius;
			GLint m_u_Offset;
			GLint m_u_BaseColorRGB;
			GLint m_u_ColorVariationHSV;

			gl::buffer m_vertices;
			gl::vertex_array m_vertex_array;
		};

		gamestate do_start(app& app)
		{
			//auto const& intro_music = app.m_assets.m_music.at("intro");
			//app.m_mixer_context.play_music(intro_music);
			//app.m_mixer_context.set_music_volume(MIX_MAX_VOLUME / 8);

			auto scene_camera = perspective_camera();
			scene_camera.m_projection.m_near = 0.1f;
			scene_camera.m_transform = glm::translate(glm::mat4(1.f), { 0.f, 0.f, 0.f });

			auto debug_cam = debug_camera_controls();

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

						if (id == control_id::MOUSEMOTION_X)
							debug_cam.m_rotate.x -= in.m_value;
						if (id == control_id::MOUSEMOTION_Y)
							debug_cam.m_rotate.y += in.m_value;
						
						if (id == control_id::KEYBOARDKEY_W)
							debug_cam.m_move_forwards = (bool)in.m_value;
						if (id == control_id::KEYBOARDKEY_S)
							debug_cam.m_move_backwards = (bool)in.m_value;
						if (id == control_id::KEYBOARDKEY_A)
							debug_cam.m_move_left = (bool)in.m_value;
						if (id == control_id::KEYBOARDKEY_D)
							debug_cam.m_move_right = (bool)in.m_value;
						if (id == control_id::KEYBOARDKEY_LEFTSHIFT)
							debug_cam.m_move_fast = (bool)in.m_value;

						

						if (id == control_id::KEYBOARDKEY_ESCAPE && in.m_value == 1.f) quit = true;
						// else if (id == control_id::GAMEPADSTICK_LEFTX || id == control_id::GAMEPADSTICK_LEFTY || id == control_id::GAMEPADSTICK_RIGHTX || id == control_id::GAMEPADSTICK_RIGHTY) return;
						// else if (id == control_id::MOUSEPOSITION_X || id == control_id::MOUSEPOSITION_Y || id == control_id::MOUSEMOTION_X || id == control_id::MOUSEMOTION_Y) return;
						// else enter_game = true;
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
					debug_cam.update(timer.get_last_frame_time());
					scene_camera.m_transform = debug_cam.m_transform;

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
