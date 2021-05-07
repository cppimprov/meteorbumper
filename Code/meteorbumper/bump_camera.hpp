#pragma once

#include <glm/glm.hpp>

namespace bump
{
	
	class orthographic_projection
	{
	public:

		orthographic_projection();

		float aspect_ratio() const;
		glm::mat4 projection_matrix() const;

		glm::vec2 m_position, m_size;
		float m_near, m_far;
	};

	class perspective_projection
	{
	public:

		perspective_projection();

		float aspect_ratio() const;
		glm::mat4 projection_matrix() const;

		glm::vec2 m_size;
		float m_fov_degrees;
		float m_near, m_far;
	};


	class viewport
	{
	public:

		viewport();

		glm::mat4 viewport_matrix() const;
		
		glm::vec2 m_position, m_size;
		float m_min_depth, m_max_depth;
	};

	class orthographic_camera
	{
	public:

		orthographic_camera();

		glm::mat4 m_transform;
		orthographic_projection m_projection;
		viewport m_viewport;
	};
	
	class perspective_camera
	{
	public:

		perspective_camera();

		glm::mat4 m_transform;
		perspective_projection m_projection;
		viewport m_viewport;
	};

	class camera_matrices
	{
	public:

		camera_matrices() = default;
		
		template<class CameraT>
		camera_matrices(CameraT const& camera);

		glm::mat4 model_view_matrix(glm::mat4 const& model) const;
		glm::mat4 model_view_projection_matrix(glm::mat4 const& model) const;
		glm::mat3 normal_matrix(glm::mat4 const& model) const;
		
		glm::mat4 m_view;
		glm::mat4 m_projection;
		glm::mat4 m_view_projection;
		glm::mat4 m_viewport;

		glm::mat4 m_inv_view;
		glm::mat4 m_inv_projection;
		glm::mat4 m_inv_view_projection;
		glm::mat4 m_inv_viewport;
	};

	template<class CameraT>
	camera_matrices::camera_matrices(CameraT const& camera):
		m_view(glm::inverse(camera.m_transform)),
		m_projection(camera.m_projection.projection_matrix()),
		m_view_projection(m_projection * m_view),
		m_viewport(camera.m_viewport.viewport_matrix()),
		m_inv_view(camera.m_transform),
		m_inv_projection(glm::inverse(m_projection)),
		m_inv_view_projection(glm::inverse(m_view_projection)),
		m_inv_viewport(glm::inverse(m_viewport)) { }

} // bump
