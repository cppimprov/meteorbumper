#pragma once

#include "bump_camera.hpp"
#include "bump_gl.hpp"

#include <entt.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/std_based_type.hpp>

namespace bump
{

	struct mbp_model;

	namespace lighting
	{
	
		class gbuffers
		{
		public:

			explicit gbuffers(glm::ivec2 screen_size);

			void recreate(glm::ivec2 screen_size);

			gl::framebuffer m_framebuffer;
			gl::texture_2d m_buffer_1; // diffuse.xyz, metallic
			gl::texture_2d m_buffer_2; // normal.xyz, roughness (note: 16 bit floats)
			gl::texture_2d m_buffer_3; // depth (24 bit float as vec3), emissive
			gl::renderbuffer m_depth_stencil;
		};

		class shadow_rendertarget
		{
		public:

			explicit shadow_rendertarget(glm::ivec2 size);
			
			gl::framebuffer m_framebuffer;
			gl::texture_2d m_texture;
		};

		class lighting_rendertarget
		{
		public:

			explicit lighting_rendertarget(glm::ivec2 screen_size, gl::renderbuffer const& depth_stencil_rt);

			void recreate(glm::ivec2 screen_size, gl::renderbuffer const& depth_stencil_rt);

			gl::framebuffer m_framebuffer;
			gl::texture_2d m_texture;
		};

		class tone_map_quad
		{
		public:

			explicit tone_map_quad(gl::shader_program const& shader);

			glm::vec2 m_position;
			glm::vec2 m_size;

			void render(gl::texture_2d const& texture, gl::renderer& renderer, camera_matrices const& ui_matrices);

		private:

			gl::shader_program const& m_shader;
			GLint m_in_VertexPosition;
			GLint m_u_MVP;
			GLint m_u_Position;
			GLint m_u_Size;
			GLint m_u_Texture;

			gl::buffer m_vertex_buffer;
			gl::vertex_array m_vertex_array;
		};
		
		struct directional_light
		{
			glm::vec3 m_direction = glm::vec3{ 0.f, -1.f, 0.f };
			glm::vec3 m_color = glm::vec3(1.f);
		};

		struct point_light
		{
			glm::vec3 m_position = glm::vec3(0.f);
			glm::vec3 m_color = glm::vec3(1.f);
			float m_radius = 1.f;
		};

		struct main_light_tag {};

		class lighting_system
		{
		public:

			explicit lighting_system(entt::registry& registry, gl::shader_program const& directional_light_shader, gl::shader_program const& point_light_shader, mbp_model const& point_light_model, gl::shader_program const& emissive_shader);

			void render(gl::renderer& renderer, glm::vec2 screen_size, camera_matrices const& light_matrices, camera_matrices const& scene_matrices, camera_matrices const& ui_matrices, gbuffers const& gbuf, gl::texture_2d const& shadow_map);

		private:

			entt::registry& m_registry;

			struct directional_light_renderable
			{
				explicit directional_light_renderable(entt::registry& registry, gl::shader_program const& shader);

				void render(gl::renderer& renderer, glm::vec2 screen_size, camera_matrices const& light_matrices, camera_matrices const& scene_matrices, camera_matrices const& ui_matrices, gbuffers const& gbuf, gl::texture_2d const& shadow_map);

			private:

				entt::registry& m_registry;

				gl::shader_program const& m_shader;
				GLint m_in_VertexPosition;
				GLint m_in_LightDirection;
				GLint m_in_LightColor;
				GLint m_in_LightShadows;
				GLint m_u_MVP;
				GLint m_u_Size;
				GLint m_g_buffer_1;
				GLint m_g_buffer_2;
				GLint m_g_buffer_3;
				GLint m_u_Shadows;
				GLint m_u_LightViewMatrix;
				GLint m_u_InvViewMatrix;
				GLint m_u_InvProjMatrix;

				gl::buffer m_buffer_vertices;
				gl::buffer m_buffer_light_directions;
				gl::buffer m_buffer_light_colors;
				gl::buffer m_buffer_light_shadows;
				gl::vertex_array m_vertex_array;

				std::vector<glm::vec3> m_frame_light_directions;
				std::vector<glm::vec3> m_frame_light_colors;
				std::vector<float> m_frame_light_shadows;
			};

			struct point_light_renderable
			{
			public:

				explicit point_light_renderable(entt::registry& registry, gl::shader_program const& shader, mbp_model const& model);

				void render(gl::renderer& renderer, camera_matrices const& scene_matrices, gbuffers const& gbuf);

			private:

				entt::registry& m_registry;

				gl::shader_program const& m_shader;
				GLint m_in_VertexPosition;
				GLint m_in_LightPosition;
				GLint m_in_LightPosition_vs;
				GLint m_in_LightColor;
				GLint m_in_LightRadius;
				GLint m_u_MVP;
				GLint m_g_buffer_1;
				GLint m_g_buffer_2;
				GLint m_g_buffer_3;
				GLint m_u_InvProjMatrix;

				gl::buffer m_buffer_vertices;
				gl::buffer m_buffer_indices;
				gl::buffer m_buffer_light_positions;
				gl::buffer m_buffer_light_positions_vs;
				gl::buffer m_buffer_light_colors;
				gl::buffer m_buffer_light_radii;
				gl::vertex_array m_vertex_array;

				std::vector<glm::vec3> m_frame_light_positions;
				std::vector<glm::vec3> m_frame_light_positions_vs;
				std::vector<glm::vec3> m_frame_light_colors;
				std::vector<float> m_frame_light_radii;
			};

			// ... spot lights

			struct emissive_renderable
			{
			public:

				explicit emissive_renderable(entt::registry& registry, gl::shader_program const& shader);

				void render(gl::renderer& renderer, glm::vec2 screen_size, camera_matrices const& ui_matrices, gbuffers const& gbuf);

			private:

				entt::registry& m_registry;

				gl::shader_program const& m_shader;
				GLint m_in_VertexPosition;
				GLint m_u_MVP;
				GLint m_u_Size;
				GLint m_g_buffer_1;
				GLint m_g_buffer_2;
				GLint m_g_buffer_3;

				gl::buffer m_buffer_vertices;
				gl::vertex_array m_vertex_array;
			};
			
			directional_light_renderable m_renderable_directional;
			point_light_renderable m_renderable_point;
			emissive_renderable m_renderable_emissive;
		};

		} // lighting
		
	} // bump
