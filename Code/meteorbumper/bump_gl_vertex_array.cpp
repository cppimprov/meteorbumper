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

		void vertex_array::set_array_buffer(GLuint location, buffer const& buffer, GLuint divisor)
		{
			die_if(!is_valid());
			die_if(location == (GLuint)-1);
			die_if(!buffer.is_valid());

			// this won't catch small matrices, or arrays with fewer than 4 elements :(
			// but those should use the other function too!!!
			die_if(buffer.get_component_count() > 4); // use the other function for multi-slot attributes!

			glBindVertexArray(get_id());
			glBindBuffer(GL_ARRAY_BUFFER, buffer.get_id());

			auto const component_type = buffer.get_component_type();
			auto const component_count = narrow_cast<GLint>(buffer.get_component_count());
			auto const stride = (GLsizei)buffer.get_element_size_bytes();

			enable_vertex_attribute(location, component_count, component_type, stride, 0, divisor);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
			
			die_if_error();
		}

		void vertex_array::set_array_buffer(GLuint location, buffer const& buffer, GLint components, GLint elements, GLuint divisor)
		{
			die_if(!is_valid());
			die_if(location == (GLuint)-1);
			die_if(!buffer.is_valid());

			die_if(components < 1 || components > 4);
			
			glBindVertexArray(get_id());
			glBindBuffer(GL_ARRAY_BUFFER, buffer.get_id());

			auto const component_type = buffer.get_component_type();
			auto const component_count = (GLint)buffer.get_component_count();
			auto const stride = (GLsizei)buffer.get_element_size_bytes();

			for (auto i = 0; i != elements; ++i)
			{
				auto index = location + i;
				auto offset = i * components * buffer.get_component_size_bytes();

				enable_vertex_attribute(index, components, component_type, stride, offset, divisor);
			}

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
			
			die_if_error();
		}

		void vertex_array::enable_vertex_attribute(GLuint location, GLint components, GLenum component_type, GLsizei stride, std::size_t offset, GLuint divisor)
		{
			auto pointer = (void*)offset;
			auto const integers = std::initializer_list<GLenum>{ GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_INT, GL_UNSIGNED_INT };

			if (component_type == GL_FLOAT)
				glVertexAttribPointer(location, components, component_type, GL_FALSE, stride, pointer);

			else if (component_type == GL_DOUBLE)
				glVertexAttribLPointer(location, components, component_type, stride, pointer);

			else if (std::find(integers.begin(), integers.end(), component_type) != integers.end())
				glVertexAttribIPointer(location, components, component_type, stride, pointer);

			else
				die();

			glVertexAttribDivisor(location, divisor);
			glEnableVertexAttribArray(location);
		}

		void vertex_array::clear_array_buffer(GLuint location)
		{
			die_if(!is_valid());

			glBindVertexArray(get_id());
			glDisableVertexAttribArray(location); // todo: this isn't enough is it? we might need to disable more attributes...
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