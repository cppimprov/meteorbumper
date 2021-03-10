#pragma once

#include <functional>

#include <GL/glew.h>

namespace bump
{
	
	namespace gl
	{
		
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

			using deleter_type = std::function<void(GLuint)>;

			object_handle(GLuint id, deleter_type deleter);

			void reset(GLuint id, deleter_type deleter);

		private:

			GLuint m_id;
			deleter_type m_deleter;
		};
		
	} // gl
	
} // bump
