#pragma once

#include "bump_camera.hpp"
#include "bump_game_basic_renderable.hpp"
#include "bump_gl.hpp"
#include "bump_physics.hpp"

#include <entt.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <iostream>
#include <vector>

namespace bump
{
	
	namespace game
	{
		class assets;
		class crosshair;

		class player_controls
		{
		public:

			float m_boost_axis = 0.f;
			float m_vertical_axis = 0.f;
			float m_horizontal_axis = 0.f;

			bool m_mouse_update = false;
			bool m_controller_update = false;

			glm::vec2 m_mouse_motion = glm::vec2(0.f);
			glm::vec2 m_controller_position = glm::vec2(0.f);

			bool m_firing = false;
			
			void apply(physics::rigidbody& physics, crosshair& crosshair, glm::vec2 window_size, camera_matrices const& matrices);
		};

		struct player_weapon_damage
		{
			float m_damage;
		};

		class player_lasers
		{
		public:

			explicit player_lasers(entt::registry& registry, gl::shader_program const& shader);

			player_lasers(player_lasers const&) = delete;
			player_lasers& operator=(player_lasers const&) = delete;
			
			player_lasers(player_lasers&&) = delete;
			player_lasers& operator=(player_lasers&&) = delete;

			~player_lasers();

			void upgrade();
			
			void update(bool fire, glm::mat4 const& player_transform, high_res_duration_t dt);
			void render(gl::renderer& renderer, camera_matrices const& matrices);

		private:

			entt::registry& m_registry;
			gl::shader_program const& m_shader;
			GLint m_in_Color;
			GLint m_in_Position;
			GLint m_in_Direction;
			GLint m_in_BeamLength;
			GLint m_u_MVP;

			gl::buffer m_vertices;
			gl::buffer m_instance_color;
			gl::buffer m_instance_position;
			gl::buffer m_instance_direction;
			gl::buffer m_instance_beam_length;
			gl::vertex_array m_vertex_array;

			std::vector<glm::vec3> m_frame_instance_colors;
			std::vector<glm::vec3> m_frame_instance_positions;
			std::vector<glm::vec3> m_frame_instance_directions;
			std::vector<float> m_frame_instance_beam_lengths;

			struct beam_segment
			{
				glm::vec3 m_color;
				float m_beam_length;
				high_res_duration_t m_lifetime;
				bool m_collided;
			};

			struct emitter
			{
				float m_damage;
				glm::vec3 m_color;
				glm::vec3 m_origin; // in player model space
				high_res_duration_t m_max_lifetime;
				std::vector<entt::entity> m_beams;
			};

			high_res_duration_t m_firing_period;
			high_res_duration_t m_time_since_firing;

			float m_beam_speed_m_per_s;
			float m_beam_length_factor; // max_beam_length = beam_speed * firing_period * length_factor;

			std::vector<emitter> m_emitters;

			std::size_t m_upgrade_level;
		};

		class player_weapons
		{
		public:

			explicit player_weapons(entt::registry& registry, gl::shader_program const& laser_shader);

			void update(bool fire, glm::mat4 const& player_transform, high_res_duration_t dt);
			void render(gl::renderer& renderer, camera_matrices const& matrices);

			player_lasers m_lasers;
		};

		class player_health
		{
		public:

			explicit player_health();

			void update(high_res_duration_t dt);

			void take_damage(float damage);

			void reset_shield_hp() { m_shield_hp = m_shield_max_hp; }
			void reset_armor_hp() { m_armor_hp = m_armor_max_hp; }

			float get_shield_hp() const { return m_shield_hp; }
			float get_armor_hp() const { return m_armor_hp; }

			bool has_shield() const { return m_shield_hp > 0.f; }
			bool is_alive() const { return m_armor_hp > 0.f; }

		private:

			float m_shield_max_hp;
			float m_armor_max_hp;

			float m_shield_hp;
			float m_armor_hp;

			float m_shield_recharge_rate_hp_per_s;
			high_res_duration_t m_shield_recharge_delay;
			high_res_duration_t m_time_since_last_hit;
		};

		class player
		{
		public:

			explicit player(entt::registry& registry, assets& assets);

			void update(high_res_duration_t dt);
			void render(gl::renderer& renderer, camera_matrices const& matrices);

			entt::registry& m_registry;

			entt::entity m_entity;
			basic_renderable m_renderable;
			player_controls m_controls;
			player_weapons m_weapons;
			player_health m_health;

			float m_player_shield_restitution = 0.8f;
			float m_player_armor_restitution = 0.25f;
		};

	} // game
	
} // bump
