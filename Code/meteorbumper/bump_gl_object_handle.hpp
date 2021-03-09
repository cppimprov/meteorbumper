#pragma once

#include <functional>

#include <GL/glew.h>

namespace bump
{
	
	namespace gl
	{
		
		template<class Deleter = std::function<void(GLuint)>>
		class object_handle
		{
		public:

			object_handle();

			object_handle(object_handle const&) = delete;
			object_handle& operator=(object_handle const&) = delete;
			
			object_handle(object_handle&&);
			object_handle& operator=(object_handle&&);

			~object_handle();

			bool is_valid() const;
			void destroy();

			GLuint get_id() const;

		protected:

			object_handle(GLuint id, Deleter deleter);

			void reset(GLuint id, Deleter deleter);

		private:

			GLuint m_id;
			Deleter m_deleter;
		};
		
		template<class D>
		object_handle<D>::object_handle():
			m_id(0),
			m_deleter() { }

		template<class D>
		object_handle<D>::object_handle(GLuint id, D deleter):
			m_id(id),
			m_deleter(std::move(deleter)) { }
		
		template<class D>
		object_handle<D>::object_handle(object_handle&& other):
			m_id(other.m_id),
			m_deleter(std::move(other.m_deleter))
		{
			other.m_id = 0;
			other.m_deleter = D();
		}

		template<class D>
		object_handle<D>& object_handle<D>::operator=(object_handle&& other)
		{
			auto temp = std::move(other);

			using std::swap;
			swap(m_id, temp.m_id);
			swap(m_deleter, temp.m_deleter);

			return *this;
		}

		template<class D>
		object_handle<D>::~object_handle()
		{
			destroy();
		}

		template<class D>
		bool object_handle<D>::is_valid() const
		{
			return (m_id != GLuint{ 0 });
		}

		template<class D>
		void object_handle<D>::destroy()
		{
			if (!is_valid())
				return;
			
			if (m_deleter)
				m_deleter(m_id);
			
			m_id = 0;
			m_deleter = D();
		}
		
		template<class D>
		void object_handle<D>::reset(GLuint id, D deleter)
		{
			destroy();

			m_id = id;
			m_deleter = std::move(deleter);
		}

		template<class D>
		GLuint object_handle<D>::get_id() const
		{
			return m_id;
		}
		
	} // gl
	
} // bump
