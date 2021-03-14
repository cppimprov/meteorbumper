#include "bump_gl_renderer.hpp"

#include "bump_gl_error.hpp"
#include "bump_gl_shader.hpp"
#include "bump_gl_texture.hpp"
#include "bump_gl_vertex_array.hpp"
#include "bump_narrow_cast.hpp"

#include <GL/glew.h>

#include <map>

namespace bump
{
	
	namespace gl
	{

		renderer::renderer()
		{
			set_depth_test(depth_test::LESS);
		}

		void renderer::set_viewport(glm::ivec2 position, glm::uvec2 size)
		{
			glViewport(position.x, position.y, narrow_cast<GLsizei>(size.x), narrow_cast<GLsizei>(size.y));
		}
		
		void renderer::clear_color_buffers(glm::f32vec4 color)
		{
			glClearColor(color.x, color.y, color.z, color.w);
			glClear(GL_COLOR_BUFFER_BIT);
			
			die_if_error();
		}

		void renderer::clear_depth_buffers(float depth)
		{
			glClearDepth(depth);
			glClear(GL_DEPTH_BUFFER_BIT);

			die_if_error();
		}

		void renderer::set_blending(blending mode)
		{
			if (mode == blending::NONE)
			{
				glDisable(GL_BLEND);
			}
			else if (mode == blending::BLEND)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			else if (mode == blending::ADD)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE, GL_ONE);
			}
			else if (mode == blending::MOD)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_DST_COLOR, GL_ZERO);
			}
		}

		void renderer::set_depth_test(depth_test mode)
		{
			glEnable(GL_DEPTH_TEST);

			using p = std::tuple<depth_test, GLenum>;

			auto const funcs =
			{ 
				p{ depth_test::LESS, GL_LESS }, 
				p{ depth_test::LESS_EQUAL, GL_LEQUAL }, 
				p{ depth_test::GREATER, GL_GREATER }, 
				p{ depth_test::GREATER_EQUAL, GL_GEQUAL }, 
				p{ depth_test::EQUAL, GL_EQUAL }, 
				p{ depth_test::NOT_EQUAL, GL_NOTEQUAL }, 
				p{ depth_test::ALWAYS, GL_ALWAYS }, 
				p{ depth_test::NEVER, GL_NEVER },
			};

			auto f = std::find_if(funcs.begin(), funcs.end(), [&] (p p) { return std::get<0>(p) == mode; });
			die_if(f == funcs.end());

			glDepthFunc(std::get<1>(*f));
		}

		void renderer::set_depth_write(depth_write mode)
		{
			if (mode == depth_write::ENABLED)
				glDepthMask(GL_TRUE);
			else
				glDepthMask(GL_FALSE);
		}
		
		void renderer::set_seamless_cubemaps(seamless_cubemaps mode)
		{
			if (mode == seamless_cubemaps::ENABLED)
				glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
			else
				glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		}
		
		void renderer::set_program(shader_program const& program)
		{
			glUseProgram(program.get_id());
		}

		void renderer::clear_program()
		{
			glUseProgram(0);
		}
		
		void renderer::set_texture_2d(GLuint location, texture_2d const& texture)
		{
			glActiveTexture(GL_TEXTURE0 + location);
			glBindTexture(GL_TEXTURE_2D, texture.get_id());
		}
		
		void renderer::set_texture_cubemap(GLuint location, texture_cubemap const& texture)
		{
			glActiveTexture(GL_TEXTURE0 + location);
			glBindTexture(GL_TEXTURE_CUBE_MAP, texture.get_id());
		}

		void renderer::set_texture_2d_array(GLuint location, texture_2d_array const& texture)
		{
			glActiveTexture(GL_TEXTURE0 + location);
			glBindTexture(GL_TEXTURE_2D_ARRAY, texture.get_id());
		}

		void renderer::set_texture_3d(GLuint location, texture_3d const& texture)
		{
			glActiveTexture(GL_TEXTURE0 + location);
			glBindTexture(GL_TEXTURE_3D, texture.get_id());
		}

		void renderer::clear_texture_2d(GLuint location)
		{
			glActiveTexture(GL_TEXTURE0 + location);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		
		void renderer::clear_texture_cubemap(GLuint location)
		{
			glActiveTexture(GL_TEXTURE0 + location);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		}

		void renderer::clear_texture_2d_array(GLuint location)
		{
			glActiveTexture(GL_TEXTURE0 + location);
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		}

		void renderer::clear_texture_3d(GLuint location)
		{
			glActiveTexture(GL_TEXTURE0 + location);
			glBindTexture(GL_TEXTURE_3D, 0);
		}
		
		void renderer::set_vertex_array(vertex_array const& vertex_array)
		{
			glBindVertexArray(vertex_array.get_id());
		}

		void renderer::clear_vertex_array()
		{
			glBindVertexArray(0);
		}

		void renderer::draw_arrays(GLenum primitive_type, std::size_t primitive_count, std::size_t instance_count)
		{
			glDrawArraysInstanced(primitive_type, 0, narrow_cast<GLsizei>(primitive_count), narrow_cast<GLsizei>(instance_count));
		}

		void renderer::draw_indexed(GLenum primitive_type, std::size_t index_count, GLenum index_type, std::size_t instance_count)
		{
			glDrawElementsInstanced(primitive_type, narrow_cast<GLsizei>(index_count), index_type, nullptr, narrow_cast<GLsizei>(instance_count));
		}
		
	} // gl
	
} // bump