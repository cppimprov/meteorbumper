#pragma once

#include "bump_gl.hpp"

#include <entt.hpp>

namespace bump
{
	
	struct mbp_model;
	class perspective_camera;
	class camera_matrices;

	namespace game
	{
		
		class basic_renderable
		{
		public:

			explicit basic_renderable(gl::shader_program const& shader, mbp_model const& model);

			basic_renderable(basic_renderable const&) = delete;
			basic_renderable& operator=(basic_renderable const&) = delete;
			
			basic_renderable(basic_renderable&&) = default;
			basic_renderable& operator=(basic_renderable&&) = default;

			void render(gl::renderer& renderer, camera_matrices const& matrices);

			void set_transform(glm::mat4 const& transform) { m_transform = transform; }
			glm::mat4 get_transform() const { return m_transform; }

		private:

			gl::shader_program const* m_shader;
			GLint m_in_VertexPosition;
			GLint m_in_VertexNormal;
			GLint m_u_MVP;
			GLint m_u_NormalMatrix;
			GLint m_u_Color;
			GLint m_u_Metallic;
			GLint m_u_Roughness;
			GLint m_u_Emissive;

			struct submesh_data
			{
				glm::vec3 m_color;
				float m_metallic;
				float m_roughness;
				float m_emissive;

				gl::buffer m_vertices;
				gl::buffer m_normals;
				gl::buffer m_indices;
				gl::vertex_array m_vertex_array;
			};

			std::vector<submesh_data> m_submeshes;

			glm::mat4 m_transform;
		};

	} // game
	
} // bump