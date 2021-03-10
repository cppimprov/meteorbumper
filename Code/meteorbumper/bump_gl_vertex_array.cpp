#include "bump_gl_vertex_array.hpp"

#include "bump_die.hpp"
#include "bump_gl_buffer.hpp"
#include "bump_narrow_cast.hpp"

namespace bump
{
	
	namespace gl
	{
		
		vertex_array::vertex_array()
		{
			auto id = GLuint{ 0 };
			glGenVertexArrays(1, &id);
			die_if(!id);

			reset(id, [] (GLuint id) { glDeleteVertexArrays(1, &id); });
		}

		// todo:
		// max 4 floats components (floats???) per location
		// so for 4x4matrix buffers, we need to do this 4 times!
		void vertex_array::set_array_buffer(GLuint location, buffer const& buffer, GLuint divisor)
		{
			die_if(!is_valid());

			glBindVertexArray(get_id());
			glBindBuffer(GL_ARRAY_BUFFER, buffer.get_id());

			auto const component_type = buffer.get_component_type();
			auto const component_count = narrow_cast<GLint>(buffer.get_component_count());

			auto const integers = std::initializer_list<GLenum>{ GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_INT, GL_UNSIGNED_INT };

			if (component_type == GL_FLOAT)
				glVertexAttribPointer(location, component_count, component_type, GL_FALSE, 0, nullptr);

			else if (component_type == GL_DOUBLE)
				glVertexAttribLPointer(location, component_count, component_type, 0, nullptr);

			else if (std::find(integers.begin(), integers.end(), component_type) != integers.end())
				glVertexAttribIPointer(location, component_count, component_type, 0, nullptr);

			else
				die();

			glVertexAttribDivisor(location, divisor);
			glEnableVertexAttribArray(location);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
			
			die_if_error();
		}

		void vertex_array::clear_array_buffer(GLuint location)
		{
			die_if(!is_valid());

			glBindVertexArray(get_id());
			glDisableVertexAttribArray(location);
			glBindVertexArray(0);
			
			die_if_error();
		}

		void vertex_array::set_index_buffer(buffer const& buffer)
		{
			die_if(!is_valid());

			glBindVertexArray(get_id());
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.get_id());
			glBindVertexArray(0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			
			die_if_error();
		}

		void vertex_array::clear_index_buffer()
		{
			die_if(!is_valid());
			
			glBindVertexArray(get_id());
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
			
			die_if_error();
		}
		
	} // gl
	
} // bump