#include "bump_gl_object_handle.hpp"

#include "bump_die.hpp"

namespace bump
{
	
	namespace gl
	{
		
		object_handle::object_handle():
			m_id(0),
			m_deleter() { }

		object_handle::object_handle(GLuint id, deleter_type deleter):
			m_id(id),
			m_deleter(std::move(deleter)) { }
		
		object_handle::object_handle(object_handle&& other):
			m_id(other.m_id),
			m_deleter(std::move(other.m_deleter))
		{
			other.m_id = 0;
			other.m_deleter = deleter_type();
		}

		object_handle& object_handle::operator=(object_handle&& other)
		{
			auto temp = std::move(other);

			using std::swap;
			swap(m_id, temp.m_id);
			swap(m_deleter, temp.m_deleter);

			return *this;
		}

		object_handle::~object_handle()
		{
			destroy();
		}

		bool object_handle::is_valid() const
		{
			return (m_id != GLuint{ 0 });
		}

		void object_handle::destroy()
		{
			if (!is_valid())
				return;
			
			die_if(!m_deleter);
			m_deleter(m_id);
			
			m_id = 0;
			m_deleter = deleter_type();
		}
		
		void object_handle::reset(GLuint id, deleter_type deleter)
		{
			destroy();

			m_id = id;
			m_deleter = std::move(deleter);
		}

		GLuint object_handle::get_id() const
		{
			return m_id;
		}
		
	} // gl
	
} // bump