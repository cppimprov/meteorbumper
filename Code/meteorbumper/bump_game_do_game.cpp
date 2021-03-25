#include "bump_game.hpp"

#include "bump_camera.hpp"
#include "bump_game_app.hpp"
#include "bump_game_asteroids.hpp"
#include "bump_game_ecs_render.hpp"
#include "bump_game_skybox.hpp"
#include "bump_physics.hpp"
#include "bump_timer.hpp"

#include <glm/ext.hpp>
#include <glm/glm.hpp>

namespace bump
{
	
	namespace game
	{

		class crosshair
		{
		public:

			explicit crosshair(gl::shader_program const& shader);

			glm::vec2 m_position;
			glm::vec2 m_size;
			glm::vec3 m_color;

			void render(gl::renderer& renderer, camera_matrices const& matrices);

		private:

			gl::shader_program const& m_shader;
			GLint m_in_VertexPosition;
			GLint m_u_MVP;
			GLint m_u_Position;
			GLint m_u_Size;
			GLint m_u_Color;

			gl::buffer m_vertex_buffer;
			gl::vertex_array m_vertex_array;
		};

		crosshair::crosshair(gl::shader_program const& shader):
			m_position(0.f),
			m_size(20.f),
			m_color(1.f),
			m_shader(shader),
			m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
			m_u_MVP(shader.get_uniform_location("u_MVP")),
			m_u_Position(shader.get_uniform_location("u_Position")),
			m_u_Size(shader.get_uniform_location("u_Size")),
			m_u_Color(shader.get_uniform_location("u_Color"))
		{
			auto vertices = { 0.f, 0.f,  1.f, 0.f,  1.f, 1.f,  0.f, 0.f,  1.f, 1.f,  0.f, 1.f, };
			m_vertex_buffer.set_data(GL_ARRAY_BUFFER, vertices.begin(), 2, 6, GL_STATIC_DRAW);

			m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertex_buffer);
		}

		void crosshair::render(gl::renderer& renderer, camera_matrices const& matrices)
		{
			auto const mvp = matrices.model_view_projection_matrix(glm::mat4(1.f));

			renderer.set_blending(gl::renderer::blending::BLEND);
			renderer.set_depth_test(gl::renderer::depth_test::ALWAYS);

			renderer.set_program(m_shader);
			renderer.set_uniform_4x4f(m_u_MVP, mvp);
			renderer.set_uniform_2f(m_u_Position, m_position);
			renderer.set_uniform_2f(m_u_Size, m_size);
			renderer.set_uniform_3f(m_u_Color, glm::vec3(1.f));
			renderer.set_vertex_array(m_vertex_array);

			renderer.draw_arrays(GL_TRIANGLES, m_vertex_buffer.get_element_count());

			renderer.clear_vertex_array();
			renderer.clear_program();
			renderer.set_depth_test(gl::renderer::depth_test::LESS);
			renderer.set_blending(gl::renderer::blending::NONE);
		}
		
		class player_controls
		{
		public:

			float m_boost_axis = 0.f;
			float m_vertical_axis = 0.f;
			float m_horizontal_axis = 0.f;

			bool m_mouse_update = false;
			bool m_controller_update = false;

			glm::vec2 m_mouse_motion;
			glm::vec2 m_controller_position;

			void apply(physics::physics_component& physics, crosshair& crosshair, glm::vec2 window_size, high_res_duration_t dt)
			{
				auto dt_s = std::chrono::duration_cast<std::chrono::duration<float>>(dt).count();
				(void)dt_s;
				
				auto const player_transform = physics.get_transform();

				// apply maneuvering forces
				{
					auto const move_force_N = 1000.f;

					auto const up = glm::vec3{ 0.f, 0.f, -1.f };
					auto const down = -up;
					auto const left = glm::vec3{ -1.f, 0.f, 0.f };
					auto const right = -left;
					
					physics.add_force(up * move_force_N * m_vertical_axis);
					physics.add_force(right * move_force_N * m_horizontal_axis);
				}

				// apply boost force
				// todo: judder? (smooth, judder at mid speeds, smooth at high speeds).
				{
					auto const boost_dir = forwards(player_transform);
					auto const boost_force_N = 2500.f;

					physics.add_force(boost_dir * boost_force_N * m_boost_axis);
				}

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

				// crosshair
				{
					if (m_mouse_update)
					{
						auto const move = m_mouse_motion * window_size;
						crosshair.m_position += move;
						crosshair.m_position = glm::clamp(crosshair.m_position, glm::vec2(0.f), window_size);

						m_mouse_motion = glm::vec2(0.f);
						m_mouse_update = false;
					}

					if (m_controller_update)
					{
						auto const vector = glm::length(m_controller_position) <= 1.f ? m_controller_position : glm::normalize(m_controller_position);
						crosshair.m_position = (window_size * 0.5f) + vector * glm::min(window_size.x, window_size.y) * 0.5f;
						crosshair.m_position = glm::clamp(crosshair.m_position, glm::vec2(0.f), window_size);

						m_controller_update = false;
					}
				}
			}

		};

		gamestate do_game(app& app)
		{
			auto registry = entt::registry();
			auto physics_system = physics::physics_system();

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

			auto crosshair = game::crosshair(app.m_assets.m_shaders.at("crosshair"));

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
						else if (id == control_id::GAMEPADSTICK_LEFTX) controls.m_horizontal_axis = in.m_value;
						else if (id == control_id::GAMEPADSTICK_LEFTY) controls.m_vertical_axis = -in.m_value;
						else if (id == control_id::GAMEPADSTICK_RIGHTX) { controls.m_controller_position.x = in.m_value; controls.m_controller_update = true; }
						else if (id == control_id::GAMEPADSTICK_RIGHTY) { controls.m_controller_position.y = -in.m_value; controls.m_controller_update = true; }

						else if (id == control_id::KEYBOARDKEY_W) controls.m_vertical_axis = in.m_value;
						else if (id == control_id::KEYBOARDKEY_S) controls.m_vertical_axis = -in.m_value;
						else if (id == control_id::KEYBOARDKEY_A) controls.m_horizontal_axis = -in.m_value;
						else if (id == control_id::KEYBOARDKEY_D) controls.m_horizontal_axis = in.m_value;
						else if (id == control_id::KEYBOARDKEY_SPACE) controls.m_boost_axis = in.m_value;
						else if (id == control_id::MOUSEMOTION_X) { controls.m_mouse_motion.x = (in.m_value / app.m_window.get_size().x); controls.m_mouse_update = true; }
						else if (id == control_id::MOUSEMOTION_Y) { controls.m_mouse_motion.y = (in.m_value / app.m_window.get_size().y); controls.m_mouse_update = true; }

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
						controls.apply(player_physics, crosshair, glm::vec2(app.m_window.get_size()), dt);

						// physics:
						physics_system.update(registry, dt);

						// update camera position
						auto player_position = get_position(player_physics.get_transform());
						set_position(scene_camera.m_transform, player_position + glm::vec3{ 0.f, camera_height, 0.f });
						
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
					auto ui_camera_matrices = camera_matrices(ui_camera);
					crosshair.render(app.m_renderer, ui_camera_matrices);

					app.m_window.swap_buffers();
				}

				timer.tick();
			}

			return { };
		}

	} // game
	
} // bump
