#pragma once

#include "bump_gl_object_handle.hpp"

namespace bump
{
	
	namespace gl
	{
		
		class buffer;

		class vertex_array : public object_handle
		{
		public:

			vertex_array();

			void set_array_buffer(GLuint location, buffer const& buffer, GLuint divisor = 0);
			void set_array_buffer(GLuint location, buffer const& buffer, GLint components, GLint elements, GLuint divisor = 0);
			void clear_array_buffer(GLuint location);

			void set_index_buffer(buffer const& buffer);
			void clear_index_buffer();

		private:

			static void enable_vertex_attribute(GLuint location, GLint components, GLenum component_type, GLsizei stride, std::size_t offset, GLuint divisor);
		};

	} // gl
	
} // bump
