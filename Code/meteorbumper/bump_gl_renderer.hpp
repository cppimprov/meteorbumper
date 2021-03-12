#pragma once

#include "bump_narrow_cast.hpp"

#include <glm/glm.hpp>
#include <GL/glew.h>

namespace bump
{
	
	namespace gl
	{

		// struct uniform_render_data
		// {
		// 	GLint m_location;
		// 	std::variant<...all_uniform_types...> m_data; // ???
		// };

		// struct texture_render_data
		// {
		// 	GLint m_location;
		// 	GLenum m_target;
		// 	GLuint m_id;
		// };

		// struct array_mesh_render_data
		// {
		// 	GLuint m_program_id; // must not be 0
		// 	std::vector<uniform_render_data> m_uniforms;
		// 	std::vector<texture_render_data> m_textures;

		// 	GLuint m_vertex_array_id; // must not be 0

		// 	GLenum m_primitive_type;
		// 	GLsizei m_vertex_count;
		// 	GLsizei m_instance_count;
		// };

		// struct indexed_mesh_render_data
		// {
		// 	GLuint m_program_id; // must not be 0
		// 	std::vector<uniform_render_data> m_uniforms;
		// 	std::vector<texture_render_data> m_textures;

		// 	GLuint m_vertex_array_id; // must not be 0

		// 	GLenum m_primitive_type;
		// 	GLsizei m_index_count;
		// 	GLenum m_index_type;
		// 	GLsizei m_instance_count;
		// };

		class shader_program;
		class texture_2d;
		class texture_2d_array;
		class texture_3d;
		class vertex_array;

		class renderer
		{
		public:

			void set_viewport(glm::ivec2 position, glm::uvec2 size);

			void clear_color_buffers(glm::f32vec4 color = { 0.f, 0.f, 0.f, 0.f });
			void clear_depth_buffers(float depth = 1.f);
			
			enum class blend_mode { NONE, BLEND, ADD, MOD };
			void set_blend_mode(blend_mode mode);

			void set_program(shader_program const& program);
			void clear_program();

			void set_texture_2d(GLuint location, texture_2d const& texture);
			void set_texture_2d_array(GLuint location, texture_2d_array const& texture);
			void set_texture_3d(GLuint location, texture_3d const& texture);

			void clear_texture_2d(GLuint location);
			void clear_texture_2d_array(GLuint location);
			void clear_texture_3d(GLuint location);

			void set_vertex_array(vertex_array const& vertex_array);
			void clear_vertex_array();

			void set_uniform_1i(GLint location, std::int32_t value)  { glUniform1i(location, value); }
			void set_uniform_2i(GLint location, glm::i32vec2 value)  { glUniform2i(location, value.x, value.y); }
			void set_uniform_3i(GLint location, glm::i32vec3 value)  { glUniform3i(location, value.x, value.y, value.z); }
			void set_uniform_4i(GLint location, glm::i32vec4 value)  { glUniform4i(location, value.x, value.y, value.z, value.w); }
			void set_uniform_1u(GLint location, std::uint32_t value) { glUniform1ui(location, value); }
			void set_uniform_2u(GLint location, glm::u32vec2 value)  { glUniform2ui(location, value.x, value.y); }
			void set_uniform_3u(GLint location, glm::u32vec3 value)  { glUniform3ui(location, value.x, value.y, value.z); }
			void set_uniform_4u(GLint location, glm::u32vec4 value)  { glUniform4ui(location, value.x, value.y, value.z, value.w); }
			void set_uniform_1f(GLint location, float value)         { glUniform1f(location, value); }
			void set_uniform_2f(GLint location, glm::f32vec2 value)  { glUniform2f(location, value.x, value.y); }
			void set_uniform_3f(GLint location, glm::f32vec3 value)  { glUniform3f(location, value.x, value.y, value.z); }
			void set_uniform_4f(GLint location, glm::f32vec4 value)  { glUniform4f(location, value.x, value.y, value.z, value.w); }
			void set_uniform_1d(GLint location, double value)        { glUniform1d(location, value); }
			void set_uniform_2d(GLint location, glm::f64vec2 value)  { glUniform2d(location, value.x, value.y); }
			void set_uniform_3d(GLint location, glm::f64vec3 value)  { glUniform3d(location, value.x, value.y, value.z); }
			void set_uniform_4d(GLint location, glm::f64vec4 value)  { glUniform4d(location, value.x, value.y, value.z, value.w); }

			void set_uniform_2x2f(GLint location, glm::f32mat2 value)   { set_uniform_data_2x2f(location, &value[0][0], 1); }
			void set_uniform_2x3f(GLint location, glm::f32mat2x3 value) { set_uniform_data_2x3f(location, &value[0][0], 1); }
			void set_uniform_2x4f(GLint location, glm::f32mat2x4 value) { set_uniform_data_2x4f(location, &value[0][0], 1); }
			void set_uniform_3x2f(GLint location, glm::f32mat3 value)   { set_uniform_data_3x2f(location, &value[0][0], 1); }
			void set_uniform_3x3f(GLint location, glm::f32mat3x3 value) { set_uniform_data_3x3f(location, &value[0][0], 1); }
			void set_uniform_3x4f(GLint location, glm::f32mat3x4 value) { set_uniform_data_3x4f(location, &value[0][0], 1); }
			void set_uniform_4x2f(GLint location, glm::f32mat4 value)   { set_uniform_data_4x2f(location, &value[0][0], 1); }
			void set_uniform_4x3f(GLint location, glm::f32mat4x3 value) { set_uniform_data_4x3f(location, &value[0][0], 1); }
			void set_uniform_4x4f(GLint location, glm::f32mat4x4 value) { set_uniform_data_4x4f(location, &value[0][0], 1); }
			void set_uniform_2x2d(GLint location, glm::f64mat2 value)   { set_uniform_data_2x2d(location, &value[0][0], 1); }
			void set_uniform_2x3d(GLint location, glm::f64mat2x3 value) { set_uniform_data_2x3d(location, &value[0][0], 1); }
			void set_uniform_2x4d(GLint location, glm::f64mat2x4 value) { set_uniform_data_2x4d(location, &value[0][0], 1); }
			void set_uniform_3x2d(GLint location, glm::f64mat3 value)   { set_uniform_data_3x2d(location, &value[0][0], 1); }
			void set_uniform_3x3d(GLint location, glm::f64mat3x3 value) { set_uniform_data_3x3d(location, &value[0][0], 1); }
			void set_uniform_3x4d(GLint location, glm::f64mat3x4 value) { set_uniform_data_3x4d(location, &value[0][0], 1); }
			void set_uniform_4x2d(GLint location, glm::f64mat4 value)   { set_uniform_data_4x2d(location, &value[0][0], 1); }
			void set_uniform_4x3d(GLint location, glm::f64mat4x3 value) { set_uniform_data_4x3d(location, &value[0][0], 1); }
			void set_uniform_4x4d(GLint location, glm::f64mat4x4 value) { set_uniform_data_4x4d(location, &value[0][0], 1); }

			// note: count is the number of elements (i.e. the number of vectors, not the total number of floats)
			void set_uniform_data_1i(GLint location, GLint* data, std::size_t count)    { glUniform1iv (location, narrow_cast<GLsizei>(count), data); }
			void set_uniform_data_2i(GLint location, GLint* data, std::size_t count)    { glUniform2iv (location, narrow_cast<GLsizei>(count), data); }
			void set_uniform_data_3i(GLint location, GLint* data, std::size_t count)    { glUniform3iv (location, narrow_cast<GLsizei>(count), data); }
			void set_uniform_data_4i(GLint location, GLint* data, std::size_t count)    { glUniform4iv (location, narrow_cast<GLsizei>(count), data); }
			void set_uniform_data_1u(GLint location, GLuint* data, std::size_t count)   { glUniform1uiv(location, narrow_cast<GLsizei>(count), data); }
			void set_uniform_data_2u(GLint location, GLuint* data, std::size_t count)   { glUniform2uiv(location, narrow_cast<GLsizei>(count), data); }
			void set_uniform_data_3u(GLint location, GLuint* data, std::size_t count)   { glUniform3uiv(location, narrow_cast<GLsizei>(count), data); }
			void set_uniform_data_4u(GLint location, GLuint* data, std::size_t count)   { glUniform4uiv(location, narrow_cast<GLsizei>(count), data); }
			void set_uniform_data_1f(GLint location, GLfloat* data, std::size_t count)  { glUniform1fv (location, narrow_cast<GLsizei>(count), data); }
			void set_uniform_data_2f(GLint location, GLfloat* data, std::size_t count)  { glUniform2fv (location, narrow_cast<GLsizei>(count), data); }
			void set_uniform_data_3f(GLint location, GLfloat* data, std::size_t count)  { glUniform3fv (location, narrow_cast<GLsizei>(count), data); }
			void set_uniform_data_4f(GLint location, GLfloat* data, std::size_t count)  { glUniform4fv (location, narrow_cast<GLsizei>(count), data); }
			void set_uniform_data_1d(GLint location, GLdouble* data, std::size_t count) { glUniform1dv (location, narrow_cast<GLsizei>(count), data); }
			void set_uniform_data_2d(GLint location, GLdouble* data, std::size_t count) { glUniform2dv (location, narrow_cast<GLsizei>(count), data); }
			void set_uniform_data_3d(GLint location, GLdouble* data, std::size_t count) { glUniform3dv (location, narrow_cast<GLsizei>(count), data); }
			void set_uniform_data_4d(GLint location, GLdouble* data, std::size_t count) { glUniform4dv (location, narrow_cast<GLsizei>(count), data); }
			
			// note: count is the number of elements (i.e. the number of matrices, not the total number of floats)
			void set_uniform_data_2x2f(GLint location, GLfloat* data, std::size_t count)  { glUniformMatrix2fv  (location, narrow_cast<GLsizei>(count), GL_FALSE, data); }
			void set_uniform_data_2x3f(GLint location, GLfloat* data, std::size_t count)  { glUniformMatrix2x3fv(location, narrow_cast<GLsizei>(count), GL_FALSE, data); }
			void set_uniform_data_2x4f(GLint location, GLfloat* data, std::size_t count)  { glUniformMatrix2x4fv(location, narrow_cast<GLsizei>(count), GL_FALSE, data); }
			void set_uniform_data_3x2f(GLint location, GLfloat* data, std::size_t count)  { glUniformMatrix3x2fv(location, narrow_cast<GLsizei>(count), GL_FALSE, data); }
			void set_uniform_data_3x3f(GLint location, GLfloat* data, std::size_t count)  { glUniformMatrix3fv  (location, narrow_cast<GLsizei>(count), GL_FALSE, data); }
			void set_uniform_data_3x4f(GLint location, GLfloat* data, std::size_t count)  { glUniformMatrix3x4fv(location, narrow_cast<GLsizei>(count), GL_FALSE, data); }
			void set_uniform_data_4x2f(GLint location, GLfloat* data, std::size_t count)  { glUniformMatrix4x2fv(location, narrow_cast<GLsizei>(count), GL_FALSE, data); }
			void set_uniform_data_4x3f(GLint location, GLfloat* data, std::size_t count)  { glUniformMatrix4x3fv(location, narrow_cast<GLsizei>(count), GL_FALSE, data); }
			void set_uniform_data_4x4f(GLint location, GLfloat* data, std::size_t count)  { glUniformMatrix4fv  (location, narrow_cast<GLsizei>(count), GL_FALSE, data); }
			void set_uniform_data_2x2d(GLint location, GLdouble* data, std::size_t count) { glUniformMatrix2dv  (location, narrow_cast<GLsizei>(count), GL_FALSE, data); }
			void set_uniform_data_2x3d(GLint location, GLdouble* data, std::size_t count) { glUniformMatrix2x3dv(location, narrow_cast<GLsizei>(count), GL_FALSE, data); }
			void set_uniform_data_2x4d(GLint location, GLdouble* data, std::size_t count) { glUniformMatrix2x4dv(location, narrow_cast<GLsizei>(count), GL_FALSE, data); }
			void set_uniform_data_3x2d(GLint location, GLdouble* data, std::size_t count) { glUniformMatrix3x2dv(location, narrow_cast<GLsizei>(count), GL_FALSE, data); }
			void set_uniform_data_3x3d(GLint location, GLdouble* data, std::size_t count) { glUniformMatrix3dv  (location, narrow_cast<GLsizei>(count), GL_FALSE, data); }
			void set_uniform_data_3x4d(GLint location, GLdouble* data, std::size_t count) { glUniformMatrix3x4dv(location, narrow_cast<GLsizei>(count), GL_FALSE, data); }
			void set_uniform_data_4x2d(GLint location, GLdouble* data, std::size_t count) { glUniformMatrix4x2dv(location, narrow_cast<GLsizei>(count), GL_FALSE, data); }
			void set_uniform_data_4x3d(GLint location, GLdouble* data, std::size_t count) { glUniformMatrix4x3dv(location, narrow_cast<GLsizei>(count), GL_FALSE, data); }
			void set_uniform_data_4x4d(GLint location, GLdouble* data, std::size_t count) { glUniformMatrix4dv  (location, narrow_cast<GLsizei>(count), GL_FALSE, data); }

			void draw_arrays(GLenum primitive_type, std::size_t primitive_count, std::size_t instance_count = 1);
			void draw_indexed(GLenum primitive_type, std::size_t index_count, GLenum index_type, std::size_t instance_count = 1);
		};

	} // gl
	
} // bump
