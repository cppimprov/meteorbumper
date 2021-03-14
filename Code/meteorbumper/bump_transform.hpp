#pragma once

#include <glm/glm.hpp>

namespace bump
{
	
	glm::vec3 get_position(glm::mat4 const& matrix);
	void set_position(glm::mat4& matrix, glm::vec3 position);

	glm::quat get_rotation(glm::mat4 const& matrix);
	void set_rotation(glm::mat4& matrix, glm::quat const& rotation);

	glm::vec3 get_scale(glm::mat4 const& matrix); // todo: implement me!
	void set_scale(glm::mat4& matrix, glm::vec3 scale); // todo: implement me!

	glm::vec3 up(glm::mat4 const& matrix);
	glm::vec3 right(glm::mat4 const& matrix);
	glm::vec3 forwards(glm::mat4 const& matrix);

	glm::vec3 transform_vector_to_world(glm::mat4 const& matrix, glm::vec3 vector);
	glm::vec3 transform_vector_to_local(glm::mat4 const& matrix, glm::vec3 vector);
	
	glm::vec3 transform_point_to_world(glm::mat4 const& matrix, glm::vec3 point);
	glm::vec3 transform_point_to_local(glm::mat4 const& matrix, glm::vec3 point);

	void translate_in_local(glm::mat4& matrix, glm::vec3 vector);
	void translate_in_world(glm::mat4& matrix, glm::vec3 vector);

	void rotate_around_local_axis(glm::mat4& matrix, glm::vec3 axis, float angle_radians);
	void rotate_around_world_axis(glm::mat4& matrix, glm::vec3 axis, float angle_radians);
	
} // bump