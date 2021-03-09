#pragma once

#include <GL/glew.h>

namespace bump
{
	
	namespace gl
	{
		
		namespace traits
		{
			
			template<class T> struct component_type;
			template<> struct component_type<bool> { static const GLenum value = GL_BOOL; };
			template<> struct component_type<char> { static const GLenum value = GL_BYTE; };
			template<> struct component_type<unsigned char> { static const GLenum value = GL_UNSIGNED_BYTE; };
			template<> struct component_type<short int> { static const GLenum value = GL_SHORT; };
			template<> struct component_type<unsigned short int> { static const GLenum value = GL_UNSIGNED_SHORT; };
			template<> struct component_type<int> { static const GLenum value = GL_INT; };
			template<> struct component_type<unsigned int> { static const GLenum value = GL_UNSIGNED_INT; };
			template<> struct component_type<float> { static const GLenum value = GL_FLOAT; };
			template<> struct component_type<double> { static const GLenum value = GL_DOUBLE; };
			
			template<class T>
			constexpr GLenum component_type_v = component_type<T>::value;
			
		} // traits
		
	} // gl
	
} // bump