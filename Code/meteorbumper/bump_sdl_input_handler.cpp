#include "bump_sdl_input_handler.hpp"

#include "bump_pair_map.hpp"
#include "bump_sdl_window.hpp"

#include <algorithm>

namespace bump
{
	
	namespace sdl
	{

		namespace
		{

			using sdl_scancode_pair_type = std::pair<SDL_Scancode, input::control_id>;

			auto const sdl_scancode_control_ids = pair_map<SDL_Scancode, input::control_id>
			{
				sdl_scancode_pair_type{ SDL_SCANCODE_A, input::control_id::KEYBOARDKEY_A },
				sdl_scancode_pair_type{ SDL_SCANCODE_B, input::control_id::KEYBOARDKEY_B },
				sdl_scancode_pair_type{ SDL_SCANCODE_C, input::control_id::KEYBOARDKEY_C },
				sdl_scancode_pair_type{ SDL_SCANCODE_D, input::control_id::KEYBOARDKEY_D },
				sdl_scancode_pair_type{ SDL_SCANCODE_E, input::control_id::KEYBOARDKEY_E },
				sdl_scancode_pair_type{ SDL_SCANCODE_F, input::control_id::KEYBOARDKEY_F },
				sdl_scancode_pair_type{ SDL_SCANCODE_G, input::control_id::KEYBOARDKEY_G },
				sdl_scancode_pair_type{ SDL_SCANCODE_H, input::control_id::KEYBOARDKEY_H },
				sdl_scancode_pair_type{ SDL_SCANCODE_I, input::control_id::KEYBOARDKEY_I },
				sdl_scancode_pair_type{ SDL_SCANCODE_J, input::control_id::KEYBOARDKEY_J },
				sdl_scancode_pair_type{ SDL_SCANCODE_K, input::control_id::KEYBOARDKEY_K },
				sdl_scancode_pair_type{ SDL_SCANCODE_L, input::control_id::KEYBOARDKEY_L },
				sdl_scancode_pair_type{ SDL_SCANCODE_M, input::control_id::KEYBOARDKEY_M },
				sdl_scancode_pair_type{ SDL_SCANCODE_N, input::control_id::KEYBOARDKEY_N },
				sdl_scancode_pair_type{ SDL_SCANCODE_O, input::control_id::KEYBOARDKEY_O },
				sdl_scancode_pair_type{ SDL_SCANCODE_P, input::control_id::KEYBOARDKEY_P },
				sdl_scancode_pair_type{ SDL_SCANCODE_Q, input::control_id::KEYBOARDKEY_Q },
				sdl_scancode_pair_type{ SDL_SCANCODE_R, input::control_id::KEYBOARDKEY_R },
				sdl_scancode_pair_type{ SDL_SCANCODE_S, input::control_id::KEYBOARDKEY_S },
				sdl_scancode_pair_type{ SDL_SCANCODE_T, input::control_id::KEYBOARDKEY_T },
				sdl_scancode_pair_type{ SDL_SCANCODE_U, input::control_id::KEYBOARDKEY_U },
				sdl_scancode_pair_type{ SDL_SCANCODE_V, input::control_id::KEYBOARDKEY_V },
				sdl_scancode_pair_type{ SDL_SCANCODE_W, input::control_id::KEYBOARDKEY_W },
				sdl_scancode_pair_type{ SDL_SCANCODE_X, input::control_id::KEYBOARDKEY_X },
				sdl_scancode_pair_type{ SDL_SCANCODE_Y, input::control_id::KEYBOARDKEY_Y },
				sdl_scancode_pair_type{ SDL_SCANCODE_Z, input::control_id::KEYBOARDKEY_Z },

				sdl_scancode_pair_type{ SDL_SCANCODE_1, input::control_id::KEYBOARDKEY_1 },
				sdl_scancode_pair_type{ SDL_SCANCODE_2, input::control_id::KEYBOARDKEY_2 },
				sdl_scancode_pair_type{ SDL_SCANCODE_3, input::control_id::KEYBOARDKEY_3 },
				sdl_scancode_pair_type{ SDL_SCANCODE_4, input::control_id::KEYBOARDKEY_4 },
				sdl_scancode_pair_type{ SDL_SCANCODE_5, input::control_id::KEYBOARDKEY_5 },
				sdl_scancode_pair_type{ SDL_SCANCODE_6, input::control_id::KEYBOARDKEY_6 },
				sdl_scancode_pair_type{ SDL_SCANCODE_7, input::control_id::KEYBOARDKEY_7 },
				sdl_scancode_pair_type{ SDL_SCANCODE_8, input::control_id::KEYBOARDKEY_8 },
				sdl_scancode_pair_type{ SDL_SCANCODE_9, input::control_id::KEYBOARDKEY_9 },
				sdl_scancode_pair_type{ SDL_SCANCODE_0, input::control_id::KEYBOARDKEY_0 },

				sdl_scancode_pair_type{ SDL_SCANCODE_F1, input::control_id::KEYBOARDKEY_F1 },
				sdl_scancode_pair_type{ SDL_SCANCODE_F2, input::control_id::KEYBOARDKEY_F2 },
				sdl_scancode_pair_type{ SDL_SCANCODE_F3, input::control_id::KEYBOARDKEY_F3 },
				sdl_scancode_pair_type{ SDL_SCANCODE_F4, input::control_id::KEYBOARDKEY_F4 },
				sdl_scancode_pair_type{ SDL_SCANCODE_F5, input::control_id::KEYBOARDKEY_F5 },
				sdl_scancode_pair_type{ SDL_SCANCODE_F6, input::control_id::KEYBOARDKEY_F6 },
				sdl_scancode_pair_type{ SDL_SCANCODE_F7, input::control_id::KEYBOARDKEY_F7 },
				sdl_scancode_pair_type{ SDL_SCANCODE_F8, input::control_id::KEYBOARDKEY_F8 },
				sdl_scancode_pair_type{ SDL_SCANCODE_F9, input::control_id::KEYBOARDKEY_F9 },
				sdl_scancode_pair_type{ SDL_SCANCODE_F10, input::control_id::KEYBOARDKEY_F10 },
				sdl_scancode_pair_type{ SDL_SCANCODE_F11, input::control_id::KEYBOARDKEY_F11 },
				sdl_scancode_pair_type{ SDL_SCANCODE_F12, input::control_id::KEYBOARDKEY_F12 },

				sdl_scancode_pair_type{ SDL_SCANCODE_ESCAPE, input::control_id::KEYBOARDKEY_ESCAPE },

				sdl_scancode_pair_type{ SDL_SCANCODE_PRINTSCREEN, input::control_id::KEYBOARDKEY_PRINTSCREEN },
				sdl_scancode_pair_type{ SDL_SCANCODE_SCROLLLOCK, input::control_id::KEYBOARDKEY_SCROLLLOCK },
				sdl_scancode_pair_type{ SDL_SCANCODE_PAUSE, input::control_id::KEYBOARDKEY_PAUSE },

				sdl_scancode_pair_type{ SDL_SCANCODE_GRAVE, input::control_id::KEYBOARDKEY_BACKTICK },
				sdl_scancode_pair_type{ SDL_SCANCODE_MINUS, input::control_id::KEYBOARDKEY_MINUS },
				sdl_scancode_pair_type{ SDL_SCANCODE_EQUALS, input::control_id::KEYBOARDKEY_EQUALS },
				sdl_scancode_pair_type{ SDL_SCANCODE_LEFTBRACKET, input::control_id::KEYBOARDKEY_LEFTSQUAREBRACKET },
				sdl_scancode_pair_type{ SDL_SCANCODE_RIGHTBRACKET, input::control_id::KEYBOARDKEY_RIGHTSQUAREBRACKET },
				sdl_scancode_pair_type{ SDL_SCANCODE_SEMICOLON, input::control_id::KEYBOARDKEY_SEMICOLON },
				sdl_scancode_pair_type{ SDL_SCANCODE_APOSTROPHE, input::control_id::KEYBOARDKEY_SINGLEQUOTE },
				sdl_scancode_pair_type{ SDL_SCANCODE_BACKSLASH, input::control_id::KEYBOARDKEY_HASH },
				sdl_scancode_pair_type{ SDL_SCANCODE_NONUSBACKSLASH, input::control_id::KEYBOARDKEY_BACKSLASH },
				sdl_scancode_pair_type{ SDL_SCANCODE_COMMA, input::control_id::KEYBOARDKEY_COMMA },
				sdl_scancode_pair_type{ SDL_SCANCODE_PERIOD, input::control_id::KEYBOARDKEY_DOT },
				sdl_scancode_pair_type{ SDL_SCANCODE_SLASH, input::control_id::KEYBOARDKEY_FORWARDSLASH },

				sdl_scancode_pair_type{ SDL_SCANCODE_BACKSPACE, input::control_id::KEYBOARDKEY_BACKSPACE },
				sdl_scancode_pair_type{ SDL_SCANCODE_RETURN, input::control_id::KEYBOARDKEY_RETURN },
				sdl_scancode_pair_type{ SDL_SCANCODE_TAB, input::control_id::KEYBOARDKEY_TAB },
				sdl_scancode_pair_type{ SDL_SCANCODE_CAPSLOCK, input::control_id::KEYBOARDKEY_CAPSLOCK },
				sdl_scancode_pair_type{ SDL_SCANCODE_LSHIFT, input::control_id::KEYBOARDKEY_LEFTSHIFT },
				sdl_scancode_pair_type{ SDL_SCANCODE_RSHIFT, input::control_id::KEYBOARDKEY_RIGHTSHIFT },
				sdl_scancode_pair_type{ SDL_SCANCODE_LCTRL, input::control_id::KEYBOARDKEY_LEFTCTRL },
				sdl_scancode_pair_type{ SDL_SCANCODE_RCTRL, input::control_id::KEYBOARDKEY_RIGHTCTRL },
				sdl_scancode_pair_type{ SDL_SCANCODE_LALT, input::control_id::KEYBOARDKEY_LEFTALT },
				sdl_scancode_pair_type{ SDL_SCANCODE_RALT, input::control_id::KEYBOARDKEY_RIGHTALT },
				sdl_scancode_pair_type{ SDL_SCANCODE_SPACE, input::control_id::KEYBOARDKEY_SPACE },

				sdl_scancode_pair_type{ SDL_SCANCODE_LGUI, input::control_id::KEYBOARDKEY_LEFTWINDOWS },
				sdl_scancode_pair_type{ SDL_SCANCODE_RGUI, input::control_id::KEYBOARDKEY_RIGHTWINDOWS },
				sdl_scancode_pair_type{ SDL_SCANCODE_MENU, input::control_id::KEYBOARDKEY_CONTEXTMENU },

				sdl_scancode_pair_type{ SDL_SCANCODE_INSERT, input::control_id::KEYBOARDKEY_INSERT },
				sdl_scancode_pair_type{ SDL_SCANCODE_DELETE, input::control_id::KEYBOARDKEY_DELETE },
				sdl_scancode_pair_type{ SDL_SCANCODE_HOME, input::control_id::KEYBOARDKEY_HOME },
				sdl_scancode_pair_type{ SDL_SCANCODE_END, input::control_id::KEYBOARDKEY_END },
				sdl_scancode_pair_type{ SDL_SCANCODE_PAGEUP, input::control_id::KEYBOARDKEY_PAGEUP },
				sdl_scancode_pair_type{ SDL_SCANCODE_PAGEDOWN, input::control_id::KEYBOARDKEY_PAGEDOWN },

				sdl_scancode_pair_type{ SDL_SCANCODE_NUMLOCKCLEAR, input::control_id::KEYBOARDKEY_NUMLOCK },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_DIVIDE, input::control_id::KEYBOARDKEY_NUMDIVIDE },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_MULTIPLY, input::control_id::KEYBOARDKEY_NUMMULTIPLY },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_MINUS, input::control_id::KEYBOARDKEY_NUMMINUS },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_PLUS, input::control_id::KEYBOARDKEY_NUMPLUS },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_ENTER, input::control_id::KEYBOARDKEY_NUMENTER },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_PERIOD, input::control_id::KEYBOARDKEY_NUMDOT },

				sdl_scancode_pair_type{ SDL_SCANCODE_KP_0, input::control_id::KEYBOARDKEY_NUM0 },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_1, input::control_id::KEYBOARDKEY_NUM1 },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_2, input::control_id::KEYBOARDKEY_NUM2 },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_3, input::control_id::KEYBOARDKEY_NUM3 },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_4, input::control_id::KEYBOARDKEY_NUM4 },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_5, input::control_id::KEYBOARDKEY_NUM5 },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_6, input::control_id::KEYBOARDKEY_NUM6 },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_7, input::control_id::KEYBOARDKEY_NUM7 },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_8, input::control_id::KEYBOARDKEY_NUM8 },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_9, input::control_id::KEYBOARDKEY_NUM9 },

				sdl_scancode_pair_type{ SDL_SCANCODE_LEFT, input::control_id::KEYBOARDKEY_ARROWLEFT },
				sdl_scancode_pair_type{ SDL_SCANCODE_RIGHT, input::control_id::KEYBOARDKEY_ARROWRIGHT },
				sdl_scancode_pair_type{ SDL_SCANCODE_UP, input::control_id::KEYBOARDKEY_ARROWUP },
				sdl_scancode_pair_type{ SDL_SCANCODE_DOWN, input::control_id::KEYBOARDKEY_ARROWDOWN },
			};

			input::control_id sdl_scancode_to_control_id(SDL_Scancode scancode)
			{
				auto entry = sdl_scancode_control_ids.find_second(scancode);

				if (!entry)
					return input::control_id::KEYBOARDKEY_UNRECOGNISED;
				
				return entry.value();
			}

			using sdl_mousebutton_pair_type = std::pair<Uint8, input::control_id>;

			auto const sdl_mousebutton_control_ids = pair_map<Uint8, input::control_id>
			{
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_LEFT }, input::control_id::MOUSEBUTTON_LEFT },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_MIDDLE }, input::control_id::MOUSEBUTTON_MIDDLE },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_RIGHT }, input::control_id::MOUSEBUTTON_RIGHT },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_X1 }, input::control_id::MOUSEBUTTON_X1 },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_X2 }, input::control_id::MOUSEBUTTON_X2 },
			};

			input::control_id sdl_mousebutton_to_control_id(Uint8 button)
			{
				return sdl_mousebutton_control_ids.get_second(button);
			}

			using sdl_controllerbutton_pair_type = std::pair<Uint8, input::control_id>;
			
			auto const sdl_controllerbutton_control_ids = pair_map<Uint8, input::control_id>
			{
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_A }, input::control_id::GAMEPADBUTTON_A },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_B }, input::control_id::GAMEPADBUTTON_B },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_X }, input::control_id::GAMEPADBUTTON_X },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_Y }, input::control_id::GAMEPADBUTTON_Y },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_BACK }, input::control_id::GAMEPADBUTTON_BACK },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_GUIDE }, input::control_id::GAMEPADBUTTON_GUIDE },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_START }, input::control_id::GAMEPADBUTTON_START },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_LEFTSTICK }, input::control_id::GAMEPADBUTTON_LEFTSTICK },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_RIGHTSTICK }, input::control_id::GAMEPADBUTTON_RIGHTSTICK },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_LEFTSHOULDER }, input::control_id::GAMEPADBUTTON_LEFTBUMPER },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_RIGHTSHOULDER }, input::control_id::GAMEPADBUTTON_RIGHTBUMPER },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_DPAD_UP }, input::control_id::GAMEPADBUTTON_DPADUP },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_DPAD_DOWN }, input::control_id::GAMEPADBUTTON_DPADDOWN },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_DPAD_LEFT }, input::control_id::GAMEPADBUTTON_DPADLEFT },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_DPAD_RIGHT }, input::control_id::GAMEPADBUTTON_DPADRIGHT },
			};

			input::control_id sdl_controllerbutton_to_control_id(Uint8 button)
			{
				return sdl_controllerbutton_control_ids.get_second(button);
			}

			using sdl_controlleraxis_pair_type = std::pair<Uint8, input::control_id>;
			
			auto const sdl_controlleraxis_control_ids = pair_map<Uint8, input::control_id>
			{
				sdl_controlleraxis_pair_type{ Uint8{ SDL_CONTROLLER_AXIS_LEFTX }, input::control_id::GAMEPADSTICK_LEFTX },
				sdl_controlleraxis_pair_type{ Uint8{ SDL_CONTROLLER_AXIS_LEFTY }, input::control_id::GAMEPADSTICK_LEFTY },
				sdl_controlleraxis_pair_type{ Uint8{ SDL_CONTROLLER_AXIS_RIGHTX }, input::control_id::GAMEPADSTICK_RIGHTX },
				sdl_controlleraxis_pair_type{ Uint8{ SDL_CONTROLLER_AXIS_RIGHTY }, input::control_id::GAMEPADSTICK_RIGHTY },
				sdl_controlleraxis_pair_type{ Uint8{ SDL_CONTROLLER_AXIS_TRIGGERLEFT }, input::control_id::GAMEPADTRIGGER_LEFT },
				sdl_controlleraxis_pair_type{ Uint8{ SDL_CONTROLLER_AXIS_TRIGGERRIGHT }, input::control_id::GAMEPADTRIGGER_RIGHT },
			};
			
			input::control_id sdl_controlleraxis_to_control_id(Uint8 button)
			{
				return sdl_controlleraxis_control_ids.get_second(button);
			}

			float sdl_controller_axis_value_to_float(Sint16 value)
			{
				return (value < 0 ? static_cast<float>(value) / 32768 : static_cast<float>(value) / 32767);
			}

		} // unnamed


		input_handler::input_handler(window& window):
			m_window(window)
		{

		}
		
		void input_handler::poll_input(input::input_callbacks const& callbacks)
		{
			SDL_Event e;
			while (SDL_PollEvent(&e))
			{
				// quit
				if (e.type == SDL_QUIT)
				{
					if (callbacks.m_quit)
						callbacks.m_quit();
					
					continue;
				}

				// toggle fullscreen
				if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN && (e.key.keysym.mod & KMOD_LALT) != 0)
				{
					auto mode = m_window.get_display_mode();
					using display_mode = sdl::window::display_mode;
					
					if (mode != display_mode::FULLSCREEN)
						m_window.set_display_mode(mode == display_mode::BORDERLESS_WINDOWED ? display_mode::WINDOWED : display_mode::BORDERLESS_WINDOWED);
					
					continue;
				}
				
				// window focus
				if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
				{
					m_window.set_cursor_mode(sdl::window::cursor_mode::FREE);

					if (callbacks.m_pause)
						callbacks.m_pause(true);

					continue;
				}
				if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
				{
					m_window.set_cursor_mode(sdl::window::cursor_mode::RELATIVE);

					if (callbacks.m_pause)
						callbacks.m_pause(false);
					
					continue;
				}

				// controller connection / disconnection
				// todo: handle SDL_CONTROLLERDEVICEREMAPPED?
				if (e.type == SDL_CONTROLLERDEVICEADDED)
				{
					m_gamepads.push_back(sdl::gamepad(e.cdevice.which));

					continue;
				}
				if (e.type == SDL_CONTROLLERDEVICEREMOVED)
				{
					auto entry = std::find_if(m_gamepads.begin(), m_gamepads.end(),
						[&] (sdl::gamepad const& p) { return p.get_joystick_id() == e.cdevice.which; });
					
					die_if(entry == m_gamepads.end());

					m_gamepads.erase(entry);
					
					continue;
				}

				// mouse
				if (e.type == SDL_MOUSEMOTION)
				{
					auto position = glm::vec2{ e.motion.x, (m_window.get_size().y - 1) - e.motion.y };
					auto motion = glm::vec2{ e.motion.xrel, -e.motion.yrel };

					if (callbacks.m_input)
					{
						callbacks.m_input(input::control_id::MOUSEPOSITION_X, { position.x, false });
						callbacks.m_input(input::control_id::MOUSEPOSITION_Y, { position.y, false });

						if (motion.x != 0.f)
							callbacks.m_input(input::control_id::MOUSEMOTION_X, { motion.x, false });
						
						if (motion.y != 0.f)
							callbacks.m_input(input::control_id::MOUSEMOTION_Y, { motion.y, false });
					}

					continue;
				}
				if (e.type == SDL_MOUSEBUTTONUP || e.type == SDL_MOUSEBUTTONDOWN)
				{
					auto id = sdl_mousebutton_to_control_id(e.button.button);
					auto value = (e.button.state == SDL_PRESSED ? 1.f : 0.f);

					if (callbacks.m_input)
						callbacks.m_input(id, { value, true });
					
					continue;
				}

				// keyboard
				if (e.type == SDL_KEYUP || e.type == SDL_KEYDOWN)
				{
					auto id = sdl_scancode_to_control_id(e.key.keysym.scancode);
					auto value = (e.key.state == SDL_PRESSED ? 1.f : 0.f);

					if (callbacks.m_input)
						callbacks.m_input(id, { value, true });
						
					continue;
				}

				// controller
				if (e.type == SDL_CONTROLLERAXISMOTION)
				{
					auto id = sdl_controlleraxis_to_control_id(e.caxis.axis);
					auto value = sdl_controller_axis_value_to_float(e.caxis.value);

					if (callbacks.m_input)
						callbacks.m_input(id, { value, true });
					
					continue;
				}
				if (e.type == SDL_CONTROLLERBUTTONUP || e.type == SDL_CONTROLLERBUTTONDOWN)
				{
					auto id = sdl_controllerbutton_to_control_id(e.cbutton.button);
					auto value = (e.cbutton.state == SDL_PRESSED ? 1.f : 0.f);

					if (callbacks.m_input)
						callbacks.m_input(id, { value, true });
					
					continue;
				}
			}
		}
		
	} // sdl
	
} // bump