#pragma once

#include "bump_die.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/std_based_type.hpp>

#include <cstdint>
#include <vector>

namespace bump
{
	
	template<class T>
	class image
	{
	public:

		using component_t = T;

		image();
		image(std::size_t channels, glm::size2 size, T value = 0);
		image(std::size_t channels, glm::size2 size, std::vector<T> pixels);

		std::size_t channels() const { return m_channels; }
		glm::size2 size() const { return m_size; }

		std::vector<T>& pixels() { return m_pixels; }
		std::vector<T> const& pixels() const { return m_pixels; }

		component_t* data() { return m_pixels.data(); }
		component_t const* data() const { return m_pixels.data(); }

	private:

		std::size_t m_channels;
		glm::size2 m_size;
		std::vector<T> m_pixels;
	};
	
	template<class T>
	image<T>::image():
		m_channels(0), m_size{ 0, 0 }, m_pixels() { }

	template<class T>
	image<T>::image(std::size_t channels, glm::size2 size, T value):
		m_channels(channels), m_size(size), m_pixels(channels * size.x * size.y, value) { }

	template<class T>
	image<T>::image(std::size_t channels, glm::size2 size, std::vector<T> pixels):
		m_channels(channels), m_size(size), m_pixels(std::move(pixels)) { }

} // bump
