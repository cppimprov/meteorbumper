#include "bump_game.hpp"

#include "bump_die.hpp"
#include "bump_game_app.hpp"
#include "bump_gl.hpp"
#include "bump_font.hpp"
#include "bump_log.hpp"
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
		
		// temp!
		void write_png(std::string const& filename, image<std::uint8_t> const& image)
		{
			die_if(image.channels() <= 0 || image.channels() > 4);

			auto const stride = sizeof(std::uint8_t) * image.channels() * image.size().x;
			if (!stbi_write_png(filename.c_str(), narrow_cast<int>(image.size().x), narrow_cast<int>(image.size().y), narrow_cast<int>(image.channels()), image.data(), narrow_cast<int>(stride)))
			{
				log_error("stbi_write_png() failed: " + std::string(stbi_failure_reason()));
				die();
			}
		}

		struct press_start_shader_locations
		{
			press_start_shader_locations(gl::shader_program const& shader):
				m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
				m_u_MVP(shader.get_uniform_location("u_MVP")),
				m_u_Position(shader.get_uniform_location("u_Position")),
				m_u_Size(shader.get_uniform_location("u_Size")),
				m_u_Color(shader.get_uniform_location("u_Color")),
				m_u_TextTexture(shader.get_uniform_location("u_TextTexture"))
				{ }

			GLint m_in_VertexPosition;
			GLint m_u_MVP;
			GLint m_u_Position;
			GLint m_u_Size;
			GLint m_u_Color;
			GLint m_u_TextTexture;
		};

		gamestate do_start(app& app)
		{
			auto const& font = app.m_assets.m_fonts.at("press_start");
			auto text = std::string("Press any key to start!");
			auto texture = render_text_to_gl_texture(font, text);

			auto const& shader = app.m_assets.m_shaders.at("press_start");
			auto const locations = press_start_shader_locations(shader);

			auto vertices = { 0.f, 0.f,  1.f, 0.f,  1.f, 1.f,  0.f, 0.f,  1.f, 1.f,  0.f, 1.f, };

			auto vertex_buffer = gl::buffer();
			vertex_buffer.set_data(GL_ARRAY_BUFFER, vertices.begin(), 2, 6, GL_STATIC_DRAW);

			auto vertex_array = gl::vertex_array();
			vertex_array.set_array_buffer(locations.m_in_VertexPosition, vertex_buffer);

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

					auto view = glm::mat4(1.f);
					auto projection = glm::ortho(0.f, (float)app.m_window.get_size().x, 0.f, (float)app.m_window.get_size().y, -1.f, 1.f);
					auto model = glm::mat4(1.f);

					auto mvp = projection * view * model;

					// renderer.set_blend_mode(blend_mode::BLEND);

					// renderer.set_program();
					// renderer.set_uniform(locations.m_u_MVP, mvp);
					// ...
					
					// renderer.set_texture(0, texture);
					// renderer.draw(quad_mesh);

					// renderer.clear_texture(0);
					// renderer.clear_program();
					// renderer.set_blend_mode(blend_mode::NONE);

					{
						glEnable(GL_BLEND);
						glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

						glUseProgram(shader.get_id());

						glUniformMatrix4fv(locations.m_u_MVP, 1, GL_FALSE, &mvp[0][0]);
						glUniform2f(locations.m_u_Position, (float)texture.m_pos.x, (float)texture.m_pos.y);
						glUniform2f(locations.m_u_Size, (float)texture.m_texture.get_size().x, (float)texture.m_texture.get_size().y);
						glUniform3f(locations.m_u_Color, 1.f, 1.f, 1.f);
						glUniform1i(locations.m_u_TextTexture, 0);

						glActiveTexture(GL_TEXTURE0 + 0);
						glBindTexture(GL_TEXTURE_2D, texture.m_texture.get_id());

						glBindVertexArray(vertex_array.get_id());

						glDrawArraysInstanced(GL_TRIANGLES, 0, narrow_cast<GLsizei>(vertex_buffer.get_element_count()), 1);

						glBindVertexArray(0);

						glActiveTexture(GL_TEXTURE0 + 0);
						glBindTexture(GL_TEXTURE_2D, 0);
						
						glUseProgram(0);

						glDisable(GL_BLEND);
					}

					app.m_window.swap_buffers();
				}
			}

			return { };
		}

	} // game
	
} // bump
