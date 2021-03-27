#pragma once

#include <glm/glm.hpp>

#include <cstdint>
#include <functional>
#include <optional>
#include <variant>

namespace bump
{
	
	namespace physics
	{

		class rigidbody;
	
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

		class collider
		{
		public:

			using shape_type = std::variant<sphere_shape, cuboid_shape, plane_shape>;
			using callback_type = std::function<void()>; // todo: pass collision information (other collider, point, normal, velocities, etc.)

			collider();

			void set_shape(shape_type shape) { m_shape = shape; }
			shape_type get_shape() const { return m_shape; }

			void set_restitution(float value) { m_restitution = glm::clamp(value, 0.f, 1.f); }
			float get_restitution() const { return m_restitution; }

			void set_collision_layer(std::uint32_t layer) { m_layer = layer; }
			std::uint32_t get_collision_layer() const { return m_layer; }

			void set_collision_mask(std::uint32_t layer_mask) { m_layer_mask = layer_mask; }
			std::uint32_t get_collision_mask() const { return m_layer_mask; }

			void set_callback(callback_type callback) { m_callback = std::move(callback); }
			void call_callback() const { if (m_callback) m_callback(); }

		private:

			float m_restitution;
			shape_type m_shape;
			std::uint32_t m_layer;      // bitmask of layers this object is on
			std::uint32_t m_layer_mask; // bitmask of layers this object collides with
			callback_type m_callback;
		};

		struct collision_data
		{
			glm::vec3 m_point;
			glm::vec3 m_normal;
			float m_penetration = 0.f;
		};

		std::optional<collision_data> dispatch_find_collision(rigidbody const& p1, collider const& c1, rigidbody const& p2, collider const& c2);
		void resolve_impulse(rigidbody& p1, rigidbody& p2, collider const& c1, collider const& c2, collision_data const& data);
		void resolve_projection(rigidbody& p1, rigidbody& p2, collision_data const& data);
		
	} // physics
	
} // bumpe