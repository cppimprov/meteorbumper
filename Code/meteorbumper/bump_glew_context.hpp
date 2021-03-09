#pragma once

namespace bump
{
	
	class glew_context
	{
	public:

		glew_context();

		glew_context(glew_context const&) = delete;
		glew_context& operator=(glew_context const&) = delete;

		glew_context(glew_context&&) = delete;
		glew_context& operator=(glew_context&&) = delete;
	};
	
} // bump