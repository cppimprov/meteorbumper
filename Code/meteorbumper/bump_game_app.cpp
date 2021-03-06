#include "bump_game_app.hpp"

namespace bump
{

	namespace game
	{

		namespace
		{

			auto const fonts = std::vector<font_metadata>
			{
				{ "press_start", "Bungee-Regular.ttf", 64 },
				{ "fps_counter", "Roboto-Regular.ttf", 16 },
			};

			auto const sounds = std::vector<sound_metadata>{ };

			auto const music = std::vector<music_metadata>
			{
				{ "intro", "intro.wav" },
			};

			auto const shaders = std::vector<shader_metadata>
			{
				{ "text_quad", { "text_quad.vert", "text_quad.frag" } },
				{ "press_start", { "press_start_text.vert", "press_start_text.frag" } },
				{ "skybox", { "skybox.vert", "g_buffers_write.frag", "skybox.frag" } },
				{ "bouy_depth", { "default_material_instanced_depth.vert", "default_material_depth.frag" } },
				{ "bouy", { "default_material_instanced.vert", "g_buffers_write.frag", "default_material.frag" } },
				{ "player_ship_depth", { "default_material_depth.vert", "default_material_depth.frag" } },
				{ "player_ship", { "default_material.vert", "g_buffers_write.frag", "default_material.frag" } },
				{ "player_shield", { "default_material_alpha.vert", "lighting.frag", "default_material_alpha.frag" } },
				{ "asteroid", { "asteroid.vert", "g_buffers_write.frag", "asteroid.frag" } },
				{ "particle_field", { "particle_field.vert", "particle_field.frag" } },
				{ "particle_effect", { "particle_effect.vert", "lighting.frag", "particle_effect.frag" } },
				{ "indicator", { "indicator.vert", "indicator.frag" } },
				{ "crosshair", { "crosshair.vert", "crosshair.frag" } },
				{ "player_laser", { "player_laser.vert", "player_laser.geom", "g_buffers_write.frag", "player_laser.frag" } },
				{ "fps_counter", { "fps_counter.vert", "fps_counter.frag" } },
				{ "powerup_depth", { "default_material_depth.vert", "default_material_depth.frag" } },
				{ "powerup", { "default_material.vert", "g_buffers_write.frag", "default_material.frag" } },
				{ "tone_mapping", { "tone_mapping.vert", "tone_mapping.frag" } },
				{ "light_directional", { "light_directional.vert", "g_buffers_read.frag", "lighting.frag", "light_directional.frag" } },
				{ "light_point", { "light_point.vert", "g_buffers_read.frag", "lighting.frag", "light_point.frag" } },
				{ "light_emissive", { "light_emissive.vert", "g_buffers_read.frag", "light_emissive.frag" } },
				{ "asteroid_depth", { "asteroid_depth.vert", "asteroid_depth.frag" } },
				{ "start_asteroid", { "default_material.vert", "g_buffers_write.frag", "default_material.frag" } },
				{ "start_asteroid_depth", { "default_material_depth.vert", "default_material_depth.frag" } },
			};

			auto const models = std::vector<model_metadata>
			{
				{ "skybox", "skybox.mbp_model" },
				{ "bouy", "bouy.mbp_model" },
				{ "player_ship", "player_ship.mbp_model" },
				{ "player_shield_lower", "player_shield_lower.mbp_model" },
				{ "player_shield_upper", "player_shield_upper.mbp_model" },
				{ "asteroid", "asteroid.mbp_model" },
				{ "powerup_shield", "powerup_shield.mbp_model" },
				{ "powerup_armor", "powerup_armor.mbp_model" },
				{ "powerup_lasers", "powerup_lasers.mbp_model" },
				{ "point_light", "point_light.mbp_model" },
				{ "asteroid_fragment_0", "asteroid_fragment_0.mbp_model" },
				{ "asteroid_fragment_1", "asteroid_fragment_1.mbp_model" },
				{ "asteroid_fragment_2", "asteroid_fragment_2.mbp_model" },
				{ "asteroid_fragment_3", "asteroid_fragment_3.mbp_model" },
				{ "asteroid_fragment_4", "asteroid_fragment_4.mbp_model" },
				{ "asteroid_fragment_5", "asteroid_fragment_5.mbp_model" },
				{ "asteroid_fragment_6", "asteroid_fragment_6.mbp_model" },
				{ "asteroid_fragment_7", "asteroid_fragment_7.mbp_model" },
				{ "asteroid_fragment_8", "asteroid_fragment_8.mbp_model" },
				{ "asteroid_fragment_9", "asteroid_fragment_9.mbp_model" },
				{ "asteroid_fragment_10", "asteroid_fragment_10.mbp_model" },
				{ "asteroid_fragment_11", "asteroid_fragment_11.mbp_model" },
				{ "asteroid_fragment_12", "asteroid_fragment_12.mbp_model" },
				{ "asteroid_fragment_13", "asteroid_fragment_13.mbp_model" },
				{ "asteroid_fragment_14", "asteroid_fragment_14.mbp_model" },
				{ "player_ship_fragment_0", "player_ship_fragment_0.mbp_model" },
				{ "player_ship_fragment_1", "player_ship_fragment_1.mbp_model" },
				{ "player_ship_fragment_2", "player_ship_fragment_2.mbp_model" },
				{ "player_ship_fragment_3", "player_ship_fragment_3.mbp_model" },
				{ "player_ship_fragment_4", "player_ship_fragment_4.mbp_model" },
				{ "player_ship_fragment_5", "player_ship_fragment_5.mbp_model" },
			};

			auto const cubemaps = std::vector<cubemap_metadata>
			{
				{ "skybox", { "skybox_x_pos.png", "skybox_x_neg.png", "skybox_y_pos.png", "skybox_y_neg.png", "skybox_z_pos.png", "skybox_z_neg.png" } },
			};

		} // unnamed

		app::app():
			m_sdl_context(),
			m_mixer_context(),
			m_ft_context(),
			m_window({ 1280, 720 }, "bump!", sdl::window::display_mode::WINDOWED),
			m_input_handler(m_window),
			m_gl_context(m_window),
			m_glew_context(),
			m_renderer(),
			m_assets(load_assets(*this, fonts, sounds, music, shaders, models, cubemaps))
		{
			m_window.set_min_size({ 640, 360 });
			m_window.set_cursor_mode(sdl::window::cursor_mode::RELATIVE);
		}

		app::~app()
		{
			m_window.set_cursor_mode(sdl::window::cursor_mode::FREE);
		}
	
	} // game

} // bump
