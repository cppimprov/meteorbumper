#pragma once

#include <functional>
#include <utility>

namespace bump
{

	template<class T>
	class ptr_handle
	{
	public:

		ptr_handle() = default;

		bool is_valid() const;
		void destroy();

		T* get_handle() const;

	protected:

		using deleter_type = std::function<void(T*)>;

		ptr_handle(T* object, deleter_type deleter);

		void reset(T* object, deleter_type deleter);

	private:

		std::unique_ptr<T, deleter_type> m_handle;
	};

	template<class T>
	ptr_handle<T>::ptr_handle(T* object, deleter_type deleter):
		m_handle(object, std::move(deleter)) { }
	
	template<class T>
	bool ptr_handle<T>::is_valid() const
	{
		return (m_handle != nullptr);
	}

	template<class T>
	T* ptr_handle<T>::get_handle() const
	{
		return m_handle.get();
	}

	template<class T>
	void ptr_handle<T>::destroy()
	{
		m_handle.reset();
	}
	
	template<class T>
	void ptr_handle<T>::reset(T* object, deleter_type deleter)
	{
		destroy();

		m_handle = { object, std::move(deleter) };
	}

} // bump
