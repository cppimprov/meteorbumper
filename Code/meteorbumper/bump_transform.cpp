#include "bump_transform.hpp"

#include <glm/ext.hpp>

namespace bump
{
	
	glm::vec3 get_position(glm::mat4 const& matrix)
	{
		return glm::vec3(matrix[3]);
	}

	void set_position(glm::mat4& matrix, glm::vec3 position)
	{
		matrix[3].x = position.x;
		matrix[3].y = position.y;
		matrix[3].z = position.z;
	}

	glm::quat get_rotation(glm::mat4 const& matrix)
	{
		return glm::quat_cast(glm::mat3(matrix));
	}

	void set_rotation(glm::mat4& matrix, glm::quat const& rotation)
	{
		auto const m = glm::mat3_cast(rotation);

		for (auto c = 0; c != 3; ++c)
			for (auto r = 0; r != 3; ++r)
				matrix[c][r] = m[c][r];
	}

	glm::vec3 up(glm::mat4 const& matrix)
	{
		return transform_vector_to_world(matrix, { 0.f, 1.f, 0.f });
	}

	glm::vec3 right(glm::mat4 const& matrix)
	{
		return transform_vector_to_world(matrix, { 1.f, 0.f, 0.f });
	}

	glm::vec3 forwards(glm::mat4 const& matrix)
	{
		return transform_vector_to_world(matrix, { 0.f, 0.f, -1.f });
	}

	glm::vec3 transform_vector_to_world(glm::mat4 const& matrix, glm::vec3 vector)
	{
		return glm::vec3(matrix * glm::vec4(vector, 0.f));
	}

	glm::vec3 transform_vector_to_local(glm::mat4 const& matrix, glm::vec3 vector)
	{
		return glm::vec3(glm::inverse(matrix) * glm::vec4(vector, 0.f));
	}
	
	glm::vec3 transform_point_to_world(glm::mat4 const& matrix, glm::vec3 point)
	{
		return glm::vec3(matrix * glm::vec4(point, 1.f));
	}

	glm::vec3 transform_point_to_local(glm::mat4 const& matrix, glm::vec3 point)
	{
		return glm::vec3(matrix * glm::vec4(point, 1.f));
	}

	void translate_in_local(glm::mat4& matrix, glm::vec3 vector)
	{
		translate_in_world(matrix, transform_vector_to_world(matrix, vector));
	}

	void translate_in_world(glm::mat4& matrix, glm::vec3 vector)
	{
		set_position(matrix, get_position(matrix) + vector);
	}

	void rotate_around_local_axis(glm::mat4& matrix, glm::vec3 axis, float angle_radians)
	{
		matrix = glm::rotate(matrix, angle_radians, axis);
	}

	void rotate_around_world_axis(glm::mat4& matrix, glm::vec3 axis, float angle_radians)
	{
		rotate_around_local_axis(matrix, transform_vector_to_local(matrix, axis), angle_radians);
	}
	
} // bump