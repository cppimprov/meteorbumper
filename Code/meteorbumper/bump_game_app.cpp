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
				{ "skybox", { "skybox.vert", "skybox.frag" } },
				{ "player_ship", { "default_material.vert", "default_material.frag" } },
				{ "asteroid", { "asteroid.vert", "asteroid.frag" } },
				{ "particle_field", { "particle_field.vert", "particle_field.frag" } },
				{ "particle_effect", { "particle_effect.vert", "particle_effect.frag" } },
				{ "indicator", { "indicator.vert", "indicator.frag" } },
				{ "crosshair", { "crosshair.vert", "crosshair.frag" } },
				{ "player_laser", { "player_laser.vert", "player_laser.geom", "player_laser.frag" } },
				{ "fps_counter", { "fps_counter.vert", "fps_counter.frag" } },
				{ "powerup", { "default_material.vert", "default_material.frag" } },
			};

			auto const models = std::vector<model_metadata>
			{
				{ "skybox", "skybox.mbp_model" },
				{ "player_ship", "player_ship.mbp_model" },
				{ "asteroid", "asteroid.mbp_model" },
				{ "powerup_shield", "powerup_shield.mbp_model" },
				{ "powerup_armor", "powerup_armor.mbp_model" },
				{ "powerup_lasers", "powerup_lasers.mbp_model" },
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
