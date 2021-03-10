#pragma once

#include <cstddef>
#include <iterator>
#include <type_traits>

namespace bump
{
	
	template<class T>
	class span
	{
	public:

		using element_type = T;
		using value_type = std::remove_cv_t<T>;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		using pointer = T*;
		using const_pointer = T const*;
		using reference = T&;
		using const_reference = const T&;

		using iterator = pointer;
		using reverse_iterator = std::reverse_iterator<iterator>;

		span();
		span(pointer first, size_type size);

		span(span const&) = default;
		span& operator=(span const&) = default;
		
		pointer data() const;

		size_type size() const;
		size_type size_bytes() const;

		bool empty() const;
		
		reference front() const;
		reference back() const;

		reference at(size_type index) const;
		
		iterator begin() const;
		iterator end() const;
		reverse_iterator rbegin() const;
		reverse_iterator rend() const;

		span<T> first(size_type count) const;
		span<T> last(size_type count) const;
		span<T> subspan(size_type offset, size_type count) const;

	private:
		
		pointer m_data;
		size_type m_size;
	};

	template<class T>
	span<T>::span():
		m_data(nullptr),
		m_size(0)
	{

	}

	template<class T>
	span<T>::span(pointer first, size_type size):
		m_data(first),
		m_size(size)
	{
		
	}

	template<class T>
	typename span<T>::pointer span<T>::data() const
	{
		return m_data;
	}

	template<class T>
	typename span<T>::size_type span<T>::size() const
	{
		return m_size;
	}
	
	template<class T>
	typename span<T>::size_type span<T>::size_bytes() const
	{
		return size() * sizeof(element_type);
	}

	template<class T>
	bool span<T>::empty() const
	{
		return (size() == 0);
	}

	template<class T>
	typename span<T>::reference span<T>::front() const
	{
		return *begin();
	}

	template<class T>
	typename span<T>::reference span<T>::back() const
	{
		return *(std::prev(end()));
	}

	template<class T>
	typename span<T>::reference span<T>::at(size_type index) const
	{
		return data()[index];
	}

	template<class T>
	typename span<T>::iterator span<T>::begin() const
	{
		return data();
	}

	template<class T>
	typename span<T>::iterator span<T>::end() const
	{
		return data() + size();
	}

	template<class T>
	typename span<T>::reverse_iterator span<T>::rbegin() const
	{
		return { end() };
	}

	template<class T>
	typename span<T>::reverse_iterator span<T>::rend() const
	{
		return { begin() };
	}

	template<class T>
	span<T> span<T>::first(std::size_t count) const
	{
		return { data(), count };
	}
	
	template<class T>
	span<T> span<T>::last(std::size_t count) const
	{
		return { data() + (size()  - count), count };
	}
	
	template<class T>
	span<T> span<T>::subspan(std::size_t offset, std::size_t count) const
	{
		return { data() + offset, count };
	}
	
} // bump