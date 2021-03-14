#include "bump_game.hpp"

#include "bump_die.hpp"
#include "bump_game_app.hpp"
#include "bump_gl.hpp"
#include "bump_font.hpp"
#include "bump_log.hpp"
#include "bump_mbp_model.hpp"
#include "bump_narrow_cast.hpp"
#include "bump_render_text.hpp"

#include <GL/glew.h>
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

			void render(gl::renderer& renderer, glm::vec2 window_size)
			{

				// note: ortho * view * model... but view and model are both identity
				auto mvp = glm::ortho(0.f, window_size.x, 0.f, window_size.y, -1.f, 1.f);

				auto size = glm::vec2(m_texture.get_size());

				auto pos = glm::round(glm::vec2{
					(window_size.x - size.x) / 2, // center
					window_size.y / 8.f, // offset from bottom
				});
				
				renderer.set_viewport({ 0, 0 }, glm::uvec2(window_size));

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

			void render(gl::renderer& renderer, glm::vec2 window_size)
			{
				auto camera = glm::translate(glm::mat4(1.f), { 0.f, 0.f, 10.f });

				auto view = glm::inverse(camera);
				auto projection = glm::perspective(glm::radians(45.f), window_size.x / window_size.y, 0.5f, 1000.f);
				auto model = glm::mat4(1.f);

				auto mvp = projection * view * model;

				renderer.set_viewport({ 0, 0 }, glm::uvec2(window_size));

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

			void render(gl::renderer& renderer, glm::vec2 window_size)
			{
				auto view = glm::mat4(1.f);
				auto projection = glm::perspective(glm::radians(45.f), window_size.x / window_size.y, 0.5f, 1000.f);
				auto model = glm::mat4(1.f);

				auto mvp = projection * view * model;

				renderer.set_viewport({ 0, 0 }, glm::uvec2(window_size));

				renderer.set_program(m_shader);

				renderer.set_uniform_4x4f(m_u_MVP, mvp);
				renderer.set_uniform_1f(m_u_Scale, 500.f);
				renderer.set_uniform_1i(m_u_CubemapTexture, 0);

				renderer.set_texture_cubemap(0, m_texture);
				renderer.set_vertex_array(m_vertex_array);

				renderer.draw_indexed(GL_TRIANGLES, m_indices.get_element_count(), m_indices.get_component_type());

				renderer.clear_vertex_array();
				renderer.clear_texture_cubemap(0);
				renderer.clear_program();

				// todo: render last with depth testing trick...
				// todo: set viewport only one time!
				// todo: set scene camera matrices only one time!
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

		gamestate do_start(app& app)
		{
			auto const& press_start_font = app.m_assets.m_fonts.at("press_start");
			auto const& press_start_shader = app.m_assets.m_shaders.at("press_start");
			auto press_start = press_start_text(press_start_font, press_start_shader);

			auto const& test_cube_model = app.m_assets.m_models.at("test_cube");
			auto const& test_cube_shader = app.m_assets.m_shaders.at("test_cube");
			auto cube = test_cube(test_cube_model, test_cube_shader);

			auto const& skybox_model = app.m_assets.m_models.at("skybox");
			auto const& skybox_shader = app.m_assets.m_shaders.at("skybox");
			auto const& skybox_cubemap = app.m_assets.m_cubemaps.at("skybox");
			auto sky = skybox(skybox_model, skybox_shader, skybox_cubemap);

			while (true)
			{
				// input
				{
					SDL_Event e;

					while (SDL_PollEvent(&e))
					{
						if (e.type == SDL_QUIT)
							return { };

						if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)
							return { };

						if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN && (e.key.keysym.mod & KMOD_LALT) != 0)
						{
							auto mode = app.m_window.get_display_mode();
							using display_mode = sdl::window::display_mode;
							
							if (mode != display_mode::FULLSCREEN)
								app.m_window.set_display_mode(mode == display_mode::BORDERLESS_WINDOWED ? display_mode::WINDOWED : display_mode::BORDERLESS_WINDOWED);
						}
					}
				}

				// update
				{
					// ...
				}

				// render
				{
					app.m_renderer.clear_color_buffers({ 1.f, 0.f, 0.f, 1.f });
					app.m_renderer.clear_depth_buffers();

					sky.render(app.m_renderer, glm::vec2(app.m_window.get_size()));
					//cube.render(app.m_renderer, glm::vec2(app.m_window.get_size()));
					press_start.render(app.m_renderer, glm::vec2(app.m_window.get_size()));

					app.m_window.swap_buffers();
				}
			}

			return { };
		}

	} // game
	
} // bump
