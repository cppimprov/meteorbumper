#include "bump_gl_buffer.hpp"

#include "bump_die.hpp"

namespace bump
{
	
	namespace gl
	{
		
		buffer::buffer():
			m_component_type(0),
			m_component_size_bytes(0),
			m_component_count(0),
			m_element_count(0)
		{
			auto id = GLuint{ 0 };
			glGenBuffers(1, &id);
			die_if(!id);

			reset(id, [] (GLuint id) { glDeleteBuffers(1, &id); });
		}

		GLenum buffer::get_component_type() const
		{
			die_if(!is_valid());

			return m_component_type;
		}

		std::size_t buffer::get_component_size_bytes() const
		{
			die_if(!is_valid());

			return m_component_size_bytes;
		}
		
		std::size_t buffer::get_element_size_bytes() const
		{
			die_if(!is_valid());

			return get_component_size_bytes() * get_component_count();
		}
		
		std::size_t buffer::get_buffer_size_bytes() const
		{
			die_if(!is_valid());

			return get_element_size_bytes() * get_element_count();
		}

		std::size_t buffer::get_component_count() const
		{
			die_if(!is_valid());

			return m_component_count;
		}
		
		std::size_t buffer::get_element_count() const
		{
			die_if(!is_valid());
			
			return m_element_count;
		}
		
	} // gl
	
} // bump