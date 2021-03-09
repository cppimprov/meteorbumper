#pragma once

#include "bump_gl_object_handle.hpp"

namespace bump
{
	
	namespace gl
	{
		
		class buffer;

		class vertex_array : public object_handle<>
		{
		public:

			vertex_array();

			void set_array_buffer(GLuint location, buffer const& buffer, GLuint divisor = 0);
			void clear_array_buffer(GLuint location);

			void set_index_buffer(buffer const& buffer);
			void clear_index_buffer(); 
		};

	} // gl
	
} // bump