#include "bump_game_particle_field.hpp"

#include "bump_camera.hpp"

#include <random>

namespace bump
{
	
	namespace game
	{
		
		particle_field::particle_field(gl::shader_program const& shader, float radius, std::size_t grid_size):
			m_shader(shader),
			m_radius(radius),
			m_position(0.f),
			m_base_color_rgb(1.f),
			m_color_variation_hsv(0.f),
			m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
			m_u_MVP(shader.get_uniform_location("u_MVP")),
			m_u_Radius(shader.get_uniform_location("u_Radius")),
			m_u_Offset(shader.get_uniform_location("u_Offset")),
			m_u_BaseColorRGB(shader.get_uniform_location("u_BaseColorRGB")),
			m_u_ColorVariationHSV(shader.get_uniform_location("u_ColorVariationHSV"))
		{
			die_if(grid_size == 0);

			auto particle_count = grid_size * grid_size * grid_size;
			auto cell_size = 1.f / static_cast<float>(grid_size);
			auto vertices = std::vector<glm::vec3>();
			vertices.reserve(particle_count);

			auto rng = std::mt19937(std::random_device()());
			auto dist = std::uniform_real_distribution(0.f, 1.f);

			for (auto z = std::size_t{ 0 }; z != grid_size; ++z)
			{
				for (auto y = std::size_t{ 0 }; y != grid_size; ++y)
				{
					for (auto x = std::size_t{ 0 }; x != grid_size; ++x)
					{
						auto origin = glm::vec3{ x, y, z } * cell_size;
						auto pos = glm::vec3{ dist(rng), dist(rng), dist(rng) } * cell_size;
						vertices.push_back(origin + pos);
					}
				}
			}
			
			static_assert(sizeof(glm::vec3) == sizeof(float) * 3, "Array of vec3 will not be a flat array.");
			m_vertices.set_data(GL_ARRAY_BUFFER, glm::value_ptr(vertices.front()), 3, vertices.size(), GL_STATIC_DRAW);

			m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertices);
		}

		void particle_field::render(gl::renderer& renderer, camera_matrices const& matrices)
		{
			// transparency / sorting?
			
			float radius = m_radius;
			float diameter = 2.f * radius;
			glm::vec3 origin = glm::floor(m_position / diameter) * diameter;
			glm::vec3 offset = glm::mod(m_position, diameter) / diameter;

			auto m = glm::translate(glm::mat4(1.f), origin);
			auto mvp = matrices.model_view_projection_matrix(m);

			renderer.set_blending(gl::renderer::blending::BLEND);

			renderer.set_program(m_shader);

			renderer.set_uniform_4x4f(m_u_MVP, mvp);
			renderer.set_uniform_1f(m_u_Radius, m_radius);
			renderer.set_uniform_3f(m_u_Offset, offset);
			renderer.set_uniform_3f(m_u_BaseColorRGB, m_base_color_rgb);
			renderer.set_uniform_3f(m_u_ColorVariationHSV, m_color_variation_hsv);

			renderer.set_vertex_array(m_vertex_array);

			renderer.draw_arrays(GL_POINTS, m_vertices.get_element_count());

			renderer.clear_vertex_array();
			renderer.clear_program();
			renderer.set_blending(gl::renderer::blending::NONE);
		}
		
	} // game
	
} // bump