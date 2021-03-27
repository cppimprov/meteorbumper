#pragma once

#include <glm/glm.hpp>

#include <cstdint>
#include <optional>
#include <variant>

namespace bump
{
	
	namespace physics
	{

		class physics_component;
	
		struct sphere_shape
		{
			float m_radius = 1.f;
		};

		struct cuboid_shape
		{
			glm::vec3 m_half_size;
		};

		struct plane_shape
		{
			glm::vec3 m_normal;
			float m_distance;
		};
		
		enum collision_layers : std::uint32_t
		{
			PLAYER =         1u << 0u,
			PLAYER_WEAPONS = 1u << 1u,
			ASTEROIDS =      1u << 2u,
		};

		class collision_component
		{
		public:

			using shape_type = std::variant<sphere_shape, cuboid_shape, plane_shape>;

			collision_component();

			void set_shape(shape_type shape) { m_shape = shape; }
			shape_type get_shape() const { return m_shape; }

			void set_restitution(float value) { m_restitution = glm::clamp(value, 0.f, 1.f); }
			float get_restitution() const { return m_restitution; }

			void set_collision_layer(std::uint32_t layer) { m_layer = layer; }
			std::uint32_t get_collision_layer() const { return m_layer; }

			void set_collision_mask(std::uint32_t layer_mask) { m_layer_mask = layer_mask; }
			std::uint32_t get_collision_mask() const { return m_layer_mask; }

		private:

			float m_restitution;
			shape_type m_shape;
			std::uint32_t m_layer;      // bitmask of layers this object is on
			std::uint32_t m_layer_mask; // bitmask of layers this object collides with
		};

		struct collision_data
		{
			glm::vec3 m_point;
			glm::vec3 m_normal;
			float m_penetration = 0.f;
		};

		std::optional<collision_data> dispatch_find_collision(physics_component const& p1, collision_component const& c1, physics_component const& p2, collision_component const& c2);
		void resolve_impulse(physics_component& p1, physics_component& p2, collision_component const& c1, collision_component const& c2, collision_data const& data);
		void resolve_projection(physics_component& p1, physics_component& p2, collision_data const& data);
		
	} // physics
	
} // bumpe