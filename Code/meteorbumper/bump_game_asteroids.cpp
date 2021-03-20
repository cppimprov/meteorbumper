#include "bump_game_asteroids.hpp"

#include "bump_camera.hpp"
#include "bump_die.hpp"
#include "bump_game_ecs_physics.hpp"
#include "bump_mbp_model.hpp"
#include "bump_transform.hpp"

#include <random>

namespace bump
{
	
	namespace game
	{
		
		asteroid_field::asteroid_field(entt::registry& registry, mbp_model const& model, gl::shader_program const& shader):
			m_shader(&shader),
			m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
			m_in_MVP(shader.get_attribute_location("in_MVP")),
			m_in_Color(shader.get_attribute_location("in_Color"))
		{
			// setup mesh buffers
			die_if(model.m_submeshes.size() != 1);

			auto const& mesh = model.m_submeshes.front();

			m_vertices.set_data(GL_ARRAY_BUFFER, mesh.m_mesh.m_vertices.data(), 3, mesh.m_mesh.m_vertices.size() / 3, GL_STATIC_DRAW);
			m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertices);

			m_indices.set_data(GL_ELEMENT_ARRAY_BUFFER, mesh.m_mesh.m_indices.data(), 1, mesh.m_mesh.m_indices.size(), GL_STATIC_DRAW);
			m_vertex_array.set_index_buffer(m_indices);

			// set up instance buffers
			m_transforms.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 16, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_MVP, m_transforms, 1);
			m_colors.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 3, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_Color, m_colors, 1);

			// create asteroids
			auto spacing = 50.f;
			auto grid_size = glm::ivec3(4);

			auto rng = std::mt19937(std::random_device()());
			auto base_color = glm::vec3(0.8f);
			auto max_color_offset = glm::vec3(0.2f);
			auto dist = std::uniform_real_distribution<float>(-1.f, 1.f);

			for (auto z = 0; z != grid_size.z; ++z)
			{
				for (auto y = 0; y != grid_size.y; ++y)
				{
					for (auto x = 0; x != grid_size.x; ++x)
					{
						auto transform = glm::mat4(1.f);
						set_position(transform, glm::vec3(x, y, -(z + 1)) * spacing);
						
						auto color = base_color + glm::vec3(dist(rng), dist(rng), dist(rng)) * max_color_offset;

						auto id = registry.create();
						registry.emplace<ecs::asteroid_data>(id, transform, color);

						// TODO:
						// update asteroid transform from physics component!
						// set physics component position at start!
						// add random velocity to asteroids at start

						auto& physics = registry.emplace<ecs::physics_component>(id);
						physics.set_mass(200000.f);
						physics.set_local_inertia_tensor(ecs::make_sphere_inertia_tensor(200000.f, 10.f));
						
						auto& collision = registry.emplace<ecs::collision_component>(id);
						collision.set_shape({ ecs::sphere_shape{ 10.f } });
					}
				}
			}
		}

		void asteroid_field::render(entt::registry& registry, gl::renderer& renderer, camera_matrices const& matrices)
		{
			// get instance data from components
			auto view = registry.view<ecs::asteroid_data>();

			if (view.empty())
				return;

			for (auto id : view)
			{
				auto const& data = view.get<ecs::asteroid_data>(id);
				m_instance_transforms.push_back(matrices.model_view_projection_matrix(data.m_transform));
				m_instance_colors.push_back(data.m_color);
			}

			// upload instance data to buffers
			static_assert(sizeof(glm::mat4) == sizeof(float) * 16, "Unexpected matrix size.");
			m_transforms.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_instance_transforms.front()), 16, m_instance_transforms.size(), GL_STREAM_DRAW);
			
			static_assert(sizeof(glm::vec3) == sizeof(float) * 3, "Unexpected vector size.");
			m_colors.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_instance_colors.front()), 3, m_instance_colors.size(), GL_STREAM_DRAW);

			// render
			renderer.set_program(*m_shader);
			renderer.set_vertex_array(m_vertex_array);

			renderer.draw_indexed(GL_TRIANGLES, m_indices.get_element_count(), m_indices.get_component_type(), view.size());

			renderer.clear_vertex_array();
			renderer.clear_program();

			// clear instance data
			m_instance_transforms.clear();
			m_instance_colors.clear();
		}
		
	} // game
	
} // bump