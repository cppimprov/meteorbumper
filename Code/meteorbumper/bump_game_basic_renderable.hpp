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

			explicit basic_renderable(gl::shader_program const& depth_shader, gl::shader_program const& shader, mbp_model const& model);

			basic_renderable(basic_renderable const&) = delete;
			basic_renderable& operator=(basic_renderable const&) = delete;
			
			basic_renderable(basic_renderable&&) = default;
			basic_renderable& operator=(basic_renderable&&) = default;

			void render_depth(gl::renderer& renderer, camera_matrices const& matrices);
			void render(gl::renderer& renderer, camera_matrices const& matrices);

			void set_transform(glm::mat4 const& transform) { m_transform = transform; }
			glm::mat4 get_transform() const { return m_transform; }

		private:

			gl::shader_program const* m_depth_shader;

			GLint m_depth_in_VertexPosition;
			GLint m_depth_u_MVP;
			std::vector<gl::vertex_array> m_depth_vertex_arrays;

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
		
		class basic_renderable_instanced
		{
		public:

			explicit basic_renderable_instanced(gl::shader_program const& depth_shader, gl::shader_program const& shader, mbp_model const& model);

			basic_renderable_instanced(basic_renderable_instanced const&) = delete;
			basic_renderable_instanced& operator=(basic_renderable_instanced const&) = delete;
			
			basic_renderable_instanced(basic_renderable_instanced&&) = default;
			basic_renderable_instanced& operator=(basic_renderable_instanced&&) = default;

			void render_depth(gl::renderer& renderer, camera_matrices const& matrices, std::vector<glm::mat4> const& transforms);
			void render(gl::renderer& renderer, camera_matrices const& matrices, std::vector<glm::mat4> const& transforms);

		private:

			gl::shader_program const* m_depth_shader;

			GLint m_depth_in_VertexPosition;
			GLint m_depth_in_MVP;
			std::vector<gl::vertex_array> m_depth_vertex_arrays;

			gl::shader_program const* m_shader;
			GLint m_in_VertexPosition;
			GLint m_in_VertexNormal;
			GLint m_in_MVP;
			GLint m_in_NormalMatrix;
			GLint m_u_Color;
			GLint m_u_Metallic;
			GLint m_u_Roughness;
			GLint m_u_Emissive;

			gl::buffer m_buffer_mvp_matrices;
			gl::buffer m_buffer_normal_matrices;

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

			std::vector<glm::mat4> m_frame_mvp_matrices;
			std::vector<glm::mat3> m_frame_normal_matrices;
		};

	} // game
	
} // bump