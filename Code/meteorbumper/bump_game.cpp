#include "bump_game.hpp"

#include "bump_camera.hpp"
#include "bump_die.hpp"
#include "bump_game_app.hpp"
#include "bump_game_debug_camera.hpp"
#include "bump_game_ecs_physics.hpp"
#include "bump_game_ecs_render.hpp"
#include "bump_gl.hpp"
#include "bump_font.hpp"
#include "bump_log.hpp"
#include "bump_mbp_model.hpp"
#include "bump_narrow_cast.hpp"
#include "bump_render_text.hpp"
#include "bump_time.hpp"
#include "bump_timer.hpp"
#include "bump_transform.hpp"

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

		struct player_controls
		{
			player_controls():
				m_roll_left(false), m_roll_right(false),
				m_pitch_up(false), m_pitch_down(false),
				m_boost(false),
				m_window_cursor(0) { }

			// input
			bool m_roll_left, m_roll_right;
			bool m_pitch_up, m_pitch_down;
			bool m_boost;
			glm::i32vec2 m_window_cursor;

			// scaling
			// float m_boost_time;
			// float m_boost_time_multiplier;

			void apply(ecs::physics_component& physics, high_res_duration_t dt)
			{
				auto dt_s = std::chrono::duration_cast<std::chrono::duration<float>>(dt).count();

				auto transform = physics.get_transform();

				auto world_right = right(transform);
				auto world_up = up(transform);
				auto world_forward = forwards(transform);

				auto roll_force = 3500.f;
				auto pitch_force = 1000.f;
				auto boost_force = 1000.f;

				if (m_roll_left)
				{
					auto max_roll = 5.f;
					auto current_roll = glm::dot(physics.get_angular_velocity(), -world_forward);
					auto current_roll_factor = glm::clamp(current_roll / max_roll, 0.f, 1.f);
					auto roll_scale = 1.f - current_roll_factor * current_roll_factor;

					//std::cout << current_roll << " " << current_roll_factor << " " << roll_scale << std::endl;

					physics.add_torque(-world_forward * roll_force * dt_s * roll_scale);
				}
				
				if (m_roll_right)
				{
					auto max_roll = 5.f;
					auto current_roll = glm::dot(physics.get_angular_velocity(), world_forward);
					auto current_roll_factor = glm::clamp(current_roll / max_roll, 0.f, 1.f);
					auto roll_scale = 1.f - current_roll_factor * current_roll_factor;

					//std::cout << current_roll << " " << current_roll_factor << " " << roll_scale << std::endl;

					physics.add_torque(world_forward * roll_force * dt_s * roll_scale);
				}
				
				if (m_pitch_down)
					physics.add_torque(-world_right * pitch_force * dt_s);

				if (m_pitch_up)
					physics.add_torque(world_right * pitch_force * dt_s);
				
				if (m_boost)
					physics.add_force(world_forward * boost_force * dt_s);

				
				// boost:
					// gradually increase force as boost is held.
					// when released, gradually decrease boost.
					// start w/ fast increase, then slower, then fast again.
					// start smooth, then randomly vary direction a little then level out again.

				// yaw slightly towards cursor direction?

				// drag forces:

					// at max speed, drag force should equal -thrust force (?)
					// apply drag down to a certain "coasting" speed?

					// at max roll speed, drag force should equal -roll force
					// at max pitch speed, drag force should equal -pitch force

					// "lift" force?
					// when velocity is forwards, it's zero.
					// when velocity doesn't match the forward direction
						// apply some thrust in the up ship direction?

					// so if we're "drifting" perpendicular to the 

			}
		};

		namespace ecs
		{

			class asteroid_data
			{
			public:

				glm::mat4 m_transform;
				glm::vec3 m_color;
			};

		} // ecs

		class asteroid_field
		{
		public:

			explicit asteroid_field(entt::registry& registry, mbp_model const& model, gl::shader_program const& shader);

			void render(entt::registry& registry, gl::renderer& renderer, camera_matrices const& matrices);

		private:

			gl::shader_program const* m_shader;

			GLint m_in_VertexPosition;
			GLint m_in_MVP;
			GLint m_in_Color;
			
			gl::buffer m_vertices;
			gl::buffer m_indices;
			gl::buffer m_transforms;
			gl::buffer m_colors;
			gl::vertex_array m_vertex_array;

			std::vector<glm::mat4> m_instance_transforms;
			std::vector<glm::vec3> m_instance_colors;
		};

		asteroid_field::asteroid_field(entt::registry& registry, mbp_model const& model, gl::shader_program const& shader):
			m_shader(&shader),
			m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
			m_in_MVP(shader.get_attribute_location("in_MVP")),
			m_in_Color(shader.get_attribute_location("in_Color"))
		{
			// setup mesh buffers
			die_if(model.m_submeshes.size() != 1);

			auto const& mesh = model.m_submeshes.front();

			m_vertices.set_data(GL_ARRAY_BUFFER, mesh.m_mesh.m_vertices.data(), 3, mesh.m_mesh.m_vertices.size() / 3, GL_STATIC_DRAW);
			m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertices);

			m_indices.set_data(GL_ELEMENT_ARRAY_BUFFER, mesh.m_mesh.m_indices.data(), 1, mesh.m_mesh.m_indices.size(), GL_STATIC_DRAW);
			m_vertex_array.set_index_buffer(m_indices);

			// set up instance buffers
			m_transforms.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 16, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_MVP, m_transforms, 1);
			m_colors.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 3, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_Color, m_colors, 1);

			// create asteroids
			auto spacing = 50.f;
			auto grid_size = glm::ivec3(10);

			auto rng = std::mt19937(std::random_device()());
			auto base_color = glm::vec3(0.8f);
			auto max_color_offset = glm::vec3(0.2f);
			auto dist = std::uniform_real_distribution<float>(-1.f, 1.f);

			for (auto z = 0; z != grid_size.z; ++z)
			{
				for (auto y = 0; y != grid_size.y; ++y)
				{
					for (auto x = 0; x != grid_size.x; ++x)
					{
						auto transform = glm::mat4(1.f);
						set_position(transform, glm::vec3(x, y, -(z + 1)) * spacing);
						
						auto color = base_color + glm::vec3(dist(rng), dist(rng), dist(rng)) * max_color_offset;

						auto id = registry.create();
						registry.emplace<ecs::asteroid_data>(id, transform, color);
					}
				}
			}
		}

		void asteroid_field::render(entt::registry& registry, gl::renderer& renderer, camera_matrices const& matrices)
		{
			// get instance data from components
			auto view = registry.view<ecs::asteroid_data>();

			if (view.empty())
				return;

			for (auto id : view)
			{
				auto const& data = view.get<ecs::asteroid_data>(id);
				m_instance_transforms.push_back(matrices.model_view_projection_matrix(data.m_transform));
				m_instance_colors.push_back(data.m_color);
			}

			// upload instance data to buffers
			static_assert(sizeof(glm::mat4) == sizeof(float) * 16, "Unexpected matrix size.");
			m_transforms.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_instance_transforms.front()), 16, m_instance_transforms.size(), GL_STREAM_DRAW);
			
			static_assert(sizeof(glm::vec3) == sizeof(float) * 3, "Unexpected vector size.");
			m_colors.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_instance_colors.front()), 3, m_instance_colors.size(), GL_STREAM_DRAW);

			// render
			renderer.set_program(*m_shader);
			renderer.set_vertex_array(m_vertex_array);

			renderer.draw_indexed(GL_TRIANGLES, m_indices.get_element_count(), m_indices.get_component_type(), view.size());

			renderer.clear_vertex_array();
			renderer.clear_program();

			// clear instance data
			m_instance_transforms.clear();
			m_instance_colors.clear();
		}

		gamestate do_start(app& app)
		{
			app.m_window.set_cursor_mode(sdl::window::cursor_mode::RELATIVE);

			auto registry = entt::registry();
			auto physics_system = ecs::physics_system();
			auto render_system = ecs::render_system();

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
			
			auto skybox = registry.create();
			registry.emplace<ecs::skybox_renderable>(skybox, app.m_assets.m_models.at("skybox"), app.m_assets.m_shaders.at("skybox"), app.m_assets.m_cubemaps.at("skybox"));

			auto player = registry.create();
			registry.emplace<ecs::basic_renderable>(player, app.m_assets.m_models.at("player_ship"), app.m_assets.m_shaders.at("player_ship"));
			registry.emplace<ecs::physics_component>(player);
			auto controls = player_controls();

			auto asteroids = asteroid_field(registry, app.m_assets.m_models.at("asteroid"), app.m_assets.m_shaders.at("asteroid"));

			auto press_start = press_start_text(app.m_assets.m_fonts.at("press_start"), app.m_assets.m_shaders.at("press_start"));

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

						if (!paused)
						{
							if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_w)
							{
								controls.m_pitch_down = true;
								continue;
							}
							if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_w)
							{
								controls.m_pitch_down = false;
								continue;
							}
							if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_s)
							{
								controls.m_pitch_up = true;
								continue;
							}
							if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_s)
							{
								controls.m_pitch_up = false;
								continue;
							}
							if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_a)
							{
								controls.m_roll_left = true;
								continue;
							}
							if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_a)
							{
								controls.m_roll_left = false;
								continue;
							}
							if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_d)
							{
								controls.m_roll_right = true;
								continue;
							}
							if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_d)
							{
								controls.m_roll_right = false;
								continue;
							}
							if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE)
							{
								controls.m_boost = true;
								continue;
							}
							if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_SPACE)
							{
								controls.m_boost = false;
								continue;
							}
						}
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
					
					auto scene_camera_matrices = camera_matrices(scene_camera);
					render_system.render(registry, app.m_renderer, scene_camera, scene_camera_matrices);
					asteroids.render(registry, app.m_renderer, scene_camera_matrices);

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
