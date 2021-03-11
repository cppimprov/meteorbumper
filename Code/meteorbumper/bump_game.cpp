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

				renderer.set_blend_mode(gl::renderer::blend_mode::BLEND);
				
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
				renderer.set_blend_mode(gl::renderer::blend_mode::NONE);
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

		gamestate do_start(app& app)
		{
			auto const& font = app.m_assets.m_fonts.at("press_start");
			auto const& shader = app.m_assets.m_shaders.at("press_start");
			auto press_start = press_start_text(font, shader);

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

					press_start.render(app.m_renderer, glm::vec2(app.m_window.get_size()));

					app.m_window.swap_buffers();
				}
			}

			return { };
		}

	} // game
	
} // bump
