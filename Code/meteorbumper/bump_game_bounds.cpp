#include "bump_game_bounds.hpp"

#include "bump_lighting.hpp"
#include "bump_physics.hpp"

namespace bump
{
	
	namespace game
	{
		
		bounds::bounds(entt::registry& registry, float radius, gl::shader_program const& bouy_depth_shader, gl::shader_program const& bouy_shader, mbp_model const& bouy_model):
			m_registry(registry),
			m_id(entt::null),
			m_bouy(bouy_depth_shader, bouy_shader, bouy_model)
		{
			m_id = registry.create();
			m_registry.emplace<bounds_tag>(m_id);

			// set up physics
			auto& rigidbody = m_registry.emplace<physics::rigidbody>(m_id);
			rigidbody.set_infinite_mass();

			auto& collider = m_registry.emplace<physics::collider>(m_id);
			collider.set_shape(physics::inverse_sphere_shape{ radius });
			collider.set_collision_layer(physics::BOUNDS);
			collider.set_collision_mask(~physics::PLAYER_WEAPONS);

			// set up bouy transforms (circle around bounds radius)
			auto const num_bouys = std::size_t{ 100 };

			for (auto i = std::size_t{ 0 }; i != num_bouys; ++i)
			{
				auto angle = ((float)i / (float)num_bouys) * 2.f * glm::pi<float>();
				auto t = glm::mat4(1.f);
				rotate_around_world_axis(t, glm::vec3(0.f, 1.f, 0.f), angle);
				translate_in_local(t, glm::vec3(0.f, 0.f, -radius));

				m_bouy_transforms.push_back(t);
			}

			// add lights
			for (auto const& t : m_bouy_transforms)
			{
				auto id = m_registry.create();
				m_registry.emplace<bouy_light_tag>(id);

				auto& light = m_registry.emplace<lighting::point_light>(id);
				light.m_position = get_position(t);
				light.m_color = glm::vec3{ 1.f, 0.f, 0.f } * 20.f;
				light.m_radius = 15.f;
			}
		}

		void bounds::render_depth(gl::renderer& renderer, camera_matrices const& matrices)
		{
			m_bouy.render_depth(renderer, matrices, m_bouy_transforms);
		}

		void bounds::render_scene(gl::renderer& renderer, camera_matrices const& matrices)
		{
			m_bouy.render(renderer, matrices, m_bouy_transforms);
		}

		bounds::~bounds()
		{
			auto lights = m_registry.view<bouy_light_tag>();
			m_registry.destroy(lights.begin(), lights.end());
			m_registry.destroy(m_id);
		}

	} // game
	
} // bump