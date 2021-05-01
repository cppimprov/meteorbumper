#include "bump_camera.hpp"

#include <glm/ext.hpp>

namespace bump
{
	
	orthographic_projection::orthographic_projection():
		m_position(0.f), m_size(1.f),
		m_near(-1.f), m_far(1.f) { }
	
	float orthographic_projection::aspect_ratio() const
	{
		return m_size.x / m_size.y;
	}
	
	glm::mat4 orthographic_projection::projection_matrix() const
	{
		return glm::ortho(m_position.x, m_position.x + m_size.x, m_position.y, m_position.y + m_size.y, m_near, m_far);
	}

	perspective_projection::perspective_projection():
		m_size(1.f),
		m_fov_degrees(45.f),
		m_near(0.5f), m_far(200.f) { }

	float perspective_projection::aspect_ratio() const
	{
		return m_size.x / m_size.y;
	}

	glm::mat4 perspective_projection::projection_matrix() const
	{
		return glm::perspective(glm::radians(m_fov_degrees), aspect_ratio(), m_near, m_far);
	}
	
	viewport::viewport():
		m_position(0.f), m_size(1.f),
		m_min_depth(-1.f), m_max_depth(1.f) { }
	
	glm::mat4 viewport::viewport_matrix() const
	{
		return glm::inverse(
			glm::ortho(m_position.x, m_position.x + m_size.x, m_position.y, m_position.y + m_size.y, m_min_depth, m_max_depth));
	}


	orthographic_camera::orthographic_camera():
		m_transform(1.f), m_projection(), m_viewport() { }

	perspective_camera::perspective_camera():
		m_transform(1.f), m_projection(), m_viewport() { }

	
	glm::mat4 camera_matrices::model_view_matrix(glm::mat4 const& model) const
	{
		return m_view * model;
	}
	
	glm::mat4 camera_matrices::model_view_projection_matrix(glm::mat4 const& model) const
	{
		return m_view_projection * model;
	}

	glm::mat3 camera_matrices::normal_matrix(glm::mat4 const& model) const
	{
		return glm::transpose(glm::inverse(glm::mat3(model_view_matrix(model))));
	}
	
} // bump
