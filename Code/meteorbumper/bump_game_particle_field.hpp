#pragma once

#include "bump_gl.hpp"

#include <glm/glm.hpp>

#include <cstdint>

namespace bump
{

	class camera_matrices;
	
	namespace game
	{
		
		class particle_field
		{
		public:

			explicit particle_field(gl::shader_program const& shader, float radius, std::size_t grid_size);

			void set_position(glm::vec3 position) { m_position = position; }
			glm::vec3 get_position() const { return m_position; }

			void set_base_color_rgb(glm::vec3 base_color_rgb) { m_base_color_rgb = base_color_rgb; }
			glm::vec3 get_base_color_rgb() const { return m_base_color_rgb; }

			void set_color_variation_hsv(glm::vec3 color_variation_hsv) { m_color_variation_hsv = color_variation_hsv; }
			glm::vec3 get_color_variation_hsv() { return m_color_variation_hsv; }

			void render(gl::renderer& renderer, camera_matrices const& matrices);

		private:

			gl::shader_program const& m_shader;

			float m_radius;
			glm::vec3 m_position;
			glm::vec3 m_base_color_rgb;
			glm::vec3 m_color_variation_hsv;

			GLint m_in_VertexPosition;
			GLint m_u_MVP;
			GLint m_u_Radius;
			GLint m_u_Offset;
			GLint m_u_BaseColorRGB;
			GLint m_u_ColorVariationHSV;

			gl::buffer m_vertices;
			gl::vertex_array m_vertex_array;
		};
		
	} // game
	
} // bump