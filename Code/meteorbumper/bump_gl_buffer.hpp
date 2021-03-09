#pragma once

#include "bump_die.hpp"
#include "bump_gl_error.hpp"
#include "bump_gl_object_handle.hpp"
#include "bump_gl_traits.hpp"

#include <GL/glew.h>

namespace bump
{
	
	namespace gl
	{
		
		class buffer : public object_handle<>
		{
		public:
		
			buffer();

			template<class ComponentT>
			void set_data(GLenum target, ComponentT const* data, std::size_t component_count, std::size_t element_count, GLenum usage);

			template<class ComponentT>
			void set_sub_data(GLenum target, ComponentT const* data, std::size_t element_offset, std::size_t element_count);

			GLenum get_component_type() const;

			std::size_t get_component_size_bytes() const;
			std::size_t get_element_size_bytes() const;
			std::size_t get_buffer_size_bytes() const;
			
			std::size_t get_component_count() const;
			std::size_t get_element_count() const;

		private:

			GLenum m_component_type;
			std::size_t m_component_size_bytes;
			std::size_t m_component_count;
			std::size_t m_element_count;
		};
		
		template<class ComponentT>
		void buffer::set_data(GLenum target, ComponentT const* data, std::size_t component_count, std::size_t element_count, GLenum usage)
		{
			die_if(!is_valid());

			m_component_type = traits::component_type_v<ComponentT>;
			m_component_size_bytes = sizeof(ComponentT);
			m_component_count = component_count;
			m_element_count = element_count;

			glBindBuffer(target, get_id());
			glBufferData(target, get_buffer_size_bytes(), data, usage);
			glBindBuffer(target, 0);

			die_if_error();
		}
		
		template<class ComponentT>
		void buffer::set_sub_data(GLenum target, ComponentT const* data, std::size_t element_offset, std::size_t element_count)
		{
			die_if(!is_valid());
			
			die_if(traits::component_type_v<ComponentT> != get_component_type());
			die_if(element_offset + element_count > get_element_count());

			glBindBuffer(target, get_id());
			glBufferSubData(target, get_element_size() * element_offset, get_element_size() * element_count, data);
			glBindBuffer(target, 0);
			
			die_if_error();
		}

	} // gl
	
} // bump