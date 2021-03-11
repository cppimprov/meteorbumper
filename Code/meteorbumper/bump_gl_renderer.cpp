#include "bump_gl_renderer.hpp"

#include "bump_gl_error.hpp"
#include "bump_gl_shader.hpp"
#include "bump_gl_texture.hpp"
#include "bump_gl_vertex_array.hpp"
#include "bump_narrow_cast.hpp"

#include <GL/glew.h>

namespace bump
{
	
	namespace gl
	{

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

		void renderer::set_blend_mode(blend_mode mode)
		{
			if (mode == blend_mode::NONE)
			{
				glDisable(GL_BLEND);
			}
			else if (mode == blend_mode::BLEND)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			else if (mode == blend_mode::ADD)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE, GL_ONE);
			}
			else if (mode == blend_mode::MOD)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_DST_COLOR, GL_ZERO);
			}
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
		
	} // gl
	
} // bump