#include "bump_game.hpp"

#include "bump_camera.hpp"
#include "bump_die.hpp"
#include "bump_game_app.hpp"
#include "bump_gl.hpp"
#include "bump_font.hpp"
#include "bump_log.hpp"
#include "bump_mbp_model.hpp"
#include "bump_narrow_cast.hpp"
#include "bump_render_text.hpp"
#include "bump_time.hpp"
#include "bump_timer.hpp"
#include "bump_transform.hpp"

#include <GL/glew.h>
#include <glm/gtx/transform.hpp>
#include <glm/glm.hpp>
#include <stb_image.h>
#include <stb_image_write.h>

#include <algorithm>
#include <chrono>
#include <deque>
#include <iterator>
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

		class test_cube
		{
		public:

			explicit test_cube(mbp_model const& model, gl::shader_program const& shader):
				m_shader(shader),
				m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
				m_u_MVP(shader.get_uniform_location("u_MVP")),
				m_u_Color(shader.get_uniform_location("u_Color"))
			{
				for (auto const& submesh : model.m_submeshes)
				{
					auto u = uniform_data{ submesh.m_material.m_base_color };
					
					auto m = mesh_data();
					m.m_vertices.set_data(GL_ARRAY_BUFFER, submesh.m_mesh.m_vertices.data(), 3, submesh.m_mesh.m_vertices.size() / 3, GL_STATIC_DRAW);
					m.m_indices.set_data(GL_ELEMENT_ARRAY_BUFFER, submesh.m_mesh.m_indices.data(), 1, submesh.m_mesh.m_indices.size(), GL_STATIC_DRAW);
					m.m_vertex_array.set_array_buffer(m_in_VertexPosition, m.m_vertices);
					m.m_vertex_array.set_index_buffer(m.m_indices);

					auto s = submesh_data{ std::move(u), std::move(m) };
					m_submeshes.push_back(std::move(s));
				}
			}

			void render(gl::renderer& renderer, camera_matrices const& matrices)
			{
				auto mvp = matrices.model_view_projection_matrix(glm::mat4(1.f));

				renderer.set_program(m_shader);

				for (auto const& submesh : m_submeshes)
				{
					renderer.set_uniform_4x4f(m_u_MVP, mvp);
					renderer.set_uniform_3f(m_u_Color, submesh.m_uniform_data.m_color);

					renderer.set_vertex_array(submesh.m_mesh_data.m_vertex_array);

					renderer.draw_indexed(GL_TRIANGLES, submesh.m_mesh_data.m_indices.get_element_count(), submesh.m_mesh_data.m_indices.get_component_type());

					renderer.clear_vertex_array();
				}

				renderer.clear_program();
			}

		private:

			gl::shader_program const& m_shader;
			GLint m_in_VertexPosition;
			GLint m_u_MVP;
			GLint m_u_Color;

			struct uniform_data
			{
				glm::vec3 m_color;
			};

			struct mesh_data
			{
				gl::buffer m_vertices;
				gl::buffer m_indices;
				gl::vertex_array m_vertex_array;
			};

			struct submesh_data
			{
				uniform_data m_uniform_data;
				mesh_data m_mesh_data;
			};

			std::vector<submesh_data> m_submeshes;
		};

		class skybox
		{
		public:

			skybox(mbp_model const& model, gl::shader_program const& shader, gl::texture_cubemap const& texture):
				m_shader(shader),
				m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
				m_u_MVP(shader.get_uniform_location("u_MVP")),
				m_u_Scale(shader.get_uniform_location("u_Scale")),
				m_u_CubemapTexture(shader.get_uniform_location("u_CubemapTexture")),
				m_texture(texture)
			{
				die_if(model.m_submeshes.size() != 1);

				auto const& mesh = model.m_submeshes.front();

				m_vertices.set_data(GL_ARRAY_BUFFER, mesh.m_mesh.m_vertices.data(), 3, mesh.m_mesh.m_vertices.size() / 3, GL_STATIC_DRAW);
				m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertices);

				m_indices.set_data(GL_ELEMENT_ARRAY_BUFFER, mesh.m_mesh.m_indices.data(), 1, mesh.m_mesh.m_indices.size(), GL_STATIC_DRAW);
				m_vertex_array.set_index_buffer(m_indices);
			}

			void render(gl::renderer& renderer, perspective_camera const& scene_camera, camera_matrices const& matrices)
			{
				auto model = glm::translate(glm::mat4(1.f), get_position(scene_camera.m_transform));
				auto mvp = matrices.model_view_projection_matrix(model);

				// note: don't write depth
				renderer.set_depth_write(gl::renderer::depth_write::DISABLED);

				renderer.set_program(m_shader);

				renderer.set_uniform_4x4f(m_u_MVP, mvp);
				renderer.set_uniform_1f(m_u_Scale, 5.f); // note: ensure we render beyond the camera's near plane!
				renderer.set_uniform_1i(m_u_CubemapTexture, 0);

				renderer.set_texture_cubemap(0, m_texture);
				renderer.set_seamless_cubemaps(gl::renderer::seamless_cubemaps::ENABLED);
				
				renderer.set_vertex_array(m_vertex_array);

				renderer.draw_indexed(GL_TRIANGLES, m_indices.get_element_count(), m_indices.get_component_type());

				renderer.clear_vertex_array();
				renderer.set_seamless_cubemaps(gl::renderer::seamless_cubemaps::DISABLED);
				renderer.clear_texture_cubemap(0);
				renderer.clear_program();
				renderer.set_depth_write(gl::renderer::depth_write::ENABLED);
			}

		private:

			gl::shader_program const& m_shader;
			GLint m_in_VertexPosition;
			GLint m_u_MVP;
			GLint m_u_Scale;
			GLint m_u_CubemapTexture;

			gl::texture_cubemap const& m_texture;
			
			gl::buffer m_vertices;
			gl::buffer m_indices;
			gl::vertex_array m_vertex_array;
		};

		class debug_camera_controls
		{
		public:

			debug_camera_controls();

			void update(high_res_duration_t dt);

			bool m_move_fast;
			bool m_move_forwards, m_move_backwards;
			bool m_move_left, m_move_right;
			bool m_move_up, m_move_down;
			glm::vec2 m_rotate;

			glm::mat4 m_transform;
		};
		
		debug_camera_controls::debug_camera_controls():
			m_move_fast(false),
			m_move_forwards(false), m_move_backwards(false),
			m_move_left(false), m_move_right(false),
			m_move_up(false), m_move_down(false),
			m_rotate(0.f),
			m_transform(1.f) { }
			
		void debug_camera_controls::update(high_res_duration_t dt)
		{
			auto const dt_seconds = std::chrono::duration_cast<std::chrono::duration<float>>(dt).count();
			auto const movement_speed = 5.f; // metres per second
			auto const rotation_speed = 100.f; // degrees per mouse input unit per second (?)
			auto const speed_modifier = m_move_fast ? 5.f : 1.f;

			if (m_rotate.y != 0.f)
			{
				auto const amount = rotation_speed * m_rotate.y * dt_seconds;
				rotate_around_local_axis(m_transform, { 1.f, 0.f, 0.f }, glm::radians(amount));
			}

			if (m_rotate.x != 0.f)
			{
				auto const amount = rotation_speed * m_rotate.x * dt_seconds;
				rotate_around_world_axis(m_transform, { 0.f, 1.f, 0.f }, glm::radians(amount));
			}

			m_rotate = glm::vec2(0.f);

			if (m_move_forwards)
			{
				auto const amount = movement_speed * speed_modifier * dt_seconds;
				translate_in_local(m_transform, glm::vec3{ 0.f, 0.f, -1.f } * amount);
			}

			if (m_move_backwards)
			{
				auto const amount = movement_speed * speed_modifier * dt_seconds;
				translate_in_local(m_transform, glm::vec3{ 0.f, 0.f, 1.f } * amount);
			}

			if (m_move_left)
			{
				auto const amount = movement_speed * speed_modifier * dt_seconds;
				translate_in_local(m_transform, glm::vec3{ -1.f, 0.f, 0.f } * amount);
			}

			if (m_move_right)
			{
				auto const amount = movement_speed * speed_modifier * dt_seconds;
				translate_in_local(m_transform, glm::vec3{ 1.f, 0.f, 0.f } * amount);
			}

			if (m_move_up)
			{
				auto const amount = movement_speed * speed_modifier * dt_seconds;
				translate_in_local(m_transform, glm::vec3{ 0.f, 1.f, 0.f } * amount);
			}

			if (m_move_down)
			{
				auto const amount = movement_speed * speed_modifier * dt_seconds;
				translate_in_local(m_transform, glm::vec3{ 0.f, -1.f, 0.f } * amount);
			}
		}

		gamestate do_start(app& app)
		{
			app.m_window.set_cursor_mode(sdl::window::cursor_mode::RELATIVE);

			auto press_start = press_start_text(app.m_assets.m_fonts.at("press_start"), app.m_assets.m_shaders.at("press_start"));
			auto cube = test_cube(app.m_assets.m_models.at("test_cube"), app.m_assets.m_shaders.at("test_cube"));
			auto sky = skybox(app.m_assets.m_models.at("skybox"), app.m_assets.m_shaders.at("skybox"), app.m_assets.m_cubemaps.at("skybox"));

			auto scene_camera = perspective_camera();
			scene_camera.m_transform = glm::translate(glm::mat4(1.f), { 0.f, 0.f, 10.f });

			auto debug_camera = debug_camera_controls();
			debug_camera.m_transform = scene_camera.m_transform;

			auto ui_camera = orthographic_camera();

			{
				auto const size = glm::vec2(app.m_window.get_size());
				scene_camera.m_projection.m_size = size;
				scene_camera.m_viewport.m_size = size;
				ui_camera.m_projection.m_size = size;
				ui_camera.m_viewport.m_size = size;
			}

			auto paused = false;
			auto timer = frame_timer();

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

						else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)
						{
							app.m_window.set_cursor_mode(sdl::window::cursor_mode::FREE);
							return { };
						}

						else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN && (e.key.keysym.mod & KMOD_LALT) != 0)
						{
							auto mode = app.m_window.get_display_mode();
							using display_mode = sdl::window::display_mode;
							
							if (mode != display_mode::FULLSCREEN)
								app.m_window.set_display_mode(mode == display_mode::BORDERLESS_WINDOWED ? display_mode::WINDOWED : display_mode::BORDERLESS_WINDOWED);
						}

						// window focus and grabbing:
						else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
						{
							paused = true;

							app.m_window.set_cursor_mode(sdl::window::cursor_mode::FREE);
						}
						else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
						{
							app.m_window.set_cursor_mode(sdl::window::cursor_mode::RELATIVE);

							paused = false;
							timer = frame_timer();
						}
						
						// debug camera controls:
						else if (!paused && e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_LSHIFT)
							debug_camera.m_move_fast = true;
						else if (!paused && e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_LSHIFT)
							debug_camera.m_move_fast = false;
						else if (!paused && e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_w)
							debug_camera.m_move_forwards = true;
						else if (!paused && e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_w)
							debug_camera.m_move_forwards = false;
						else if (!paused && e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_s)
							debug_camera.m_move_backwards = true;
						else if (!paused && e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_s)
							debug_camera.m_move_backwards = false;
						else if (!paused && e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_a)
							debug_camera.m_move_left = true;
						else if (!paused && e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_a)
							debug_camera.m_move_left = false;
						else if (!paused && e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_d)
							debug_camera.m_move_right = true;
						else if (!paused && e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_d)
							debug_camera.m_move_right = false;
						else if (!paused && e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_PAGEUP)
							debug_camera.m_move_up = true;
						else if (!paused && e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_PAGEUP)
							debug_camera.m_move_up = false;
						else if (!paused && e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_PAGEDOWN)
							debug_camera.m_move_down = true;
						else if (!paused && e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_PAGEDOWN)
							debug_camera.m_move_down = false;
						else if (!paused && e.type == SDL_MOUSEMOTION)
						{
							debug_camera.m_rotate.x -= e.motion.xrel;
							debug_camera.m_rotate.y -= e.motion.yrel;
						}
					}
				}

				// update
				{
					// debug camera:
					if (!paused)
						debug_camera.update(timer.get_last_frame_time());
					
					scene_camera.m_transform = debug_camera.m_transform;
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
					
					auto scene_camera_matrices = camera_matrices(scene_camera);
					sky.render(app.m_renderer, scene_camera, scene_camera_matrices);
					cube.render(app.m_renderer, scene_camera_matrices);

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
