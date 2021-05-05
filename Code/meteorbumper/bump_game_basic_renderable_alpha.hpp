#pragma once

#include "bump_gl.hpp"
#include "bump_lighting.hpp"

#include <entt.hpp>

namespace bump
{
	
	struct mbp_model;
	class perspective_camera;
	class camera_matrices;

	namespace game
	{
		
		class basic_renderable_alpha
		{
		public:

			explicit basic_renderable_alpha(gl::shader_program const& shader, mbp_model const& model);

			basic_renderable_alpha(basic_renderable_alpha const&) = delete;
			basic_renderable_alpha& operator=(basic_renderable_alpha const&) = delete;
			
			basic_renderable_alpha(basic_renderable_alpha&&) = default;
			basic_renderable_alpha& operator=(basic_renderable_alpha&&) = default;

			void render(gl::renderer& renderer, camera_matrices const& matrices);

			void set_transform(glm::mat4 const& transform) { m_transform = transform; }
			glm::mat4 get_transform() const { return m_transform; }

			void set_directional_lights(std::vector<lighting::directional_light> lights) { m_lights_dir = lights; }
			void set_point_lights(std::vector<lighting::point_light> lights) { m_lights_point = lights; }

		private:
			
			gl::shader_program const* m_shader;
			GLint m_in_VertexPosition;
			GLint m_in_VertexNormal;
			GLint m_u_MV;
			GLint m_u_MVP;
			GLint m_u_NormalMatrix;
			GLint m_u_Color;
			GLint m_u_Metallic;
			GLint m_u_Roughness;
			GLint m_u_Emissive;
			GLint m_u_Opacity;
			GLint m_u_DirLightDirection;
			GLint m_u_DirLightColor;
			GLint m_u_PointLightPosition;
			GLint m_u_PointLightColor;
			GLint m_u_PointLightRadius;

			struct submesh_data
			{
				glm::vec3 m_color;
				float m_metallic;
				float m_roughness;
				float m_emissive;
				float m_opacity;

				gl::buffer m_vertices;
				gl::buffer m_normals;
				gl::buffer m_indices;
				gl::vertex_array m_vertex_array;
			};

			std::vector<submesh_data> m_submeshes;
			
			std::vector<lighting::directional_light> m_lights_dir;
			std::vector<lighting::point_light> m_lights_point;

			glm::mat4 m_transform;
		};

	} // game
	
} // bump
