#pragma once

#include <glm/glm.hpp>

#include <functional>
#include <string>

namespace bump
{
	
	namespace input
	{

		enum class control_id
		{
			// KEYBOARD:
			KEYBOARDKEY_A, KEYBOARDKEY_B, KEYBOARDKEY_C, KEYBOARDKEY_D, KEYBOARDKEY_E, KEYBOARDKEY_F, KEYBOARDKEY_G, KEYBOARDKEY_H, KEYBOARDKEY_I, KEYBOARDKEY_J, KEYBOARDKEY_K, KEYBOARDKEY_L, KEYBOARDKEY_M, KEYBOARDKEY_N, KEYBOARDKEY_O, KEYBOARDKEY_P, KEYBOARDKEY_Q, KEYBOARDKEY_R, KEYBOARDKEY_S, KEYBOARDKEY_T, KEYBOARDKEY_U, KEYBOARDKEY_V, KEYBOARDKEY_W, KEYBOARDKEY_X, KEYBOARDKEY_Y, KEYBOARDKEY_Z,
			KEYBOARDKEY_1, KEYBOARDKEY_2, KEYBOARDKEY_3, KEYBOARDKEY_4, KEYBOARDKEY_5, KEYBOARDKEY_6, KEYBOARDKEY_7, KEYBOARDKEY_8, KEYBOARDKEY_9, KEYBOARDKEY_0,
			KEYBOARDKEY_F1, KEYBOARDKEY_F2, KEYBOARDKEY_F3, KEYBOARDKEY_F4, KEYBOARDKEY_F5, KEYBOARDKEY_F6, KEYBOARDKEY_F7, KEYBOARDKEY_F8, KEYBOARDKEY_F9, KEYBOARDKEY_F10, KEYBOARDKEY_F11, KEYBOARDKEY_F12,

			KEYBOARDKEY_ESCAPE,

			KEYBOARDKEY_PRINTSCREEN, KEYBOARDKEY_SCROLLLOCK, KEYBOARDKEY_PAUSE,

			KEYBOARDKEY_BACKTICK, KEYBOARDKEY_MINUS, KEYBOARDKEY_EQUALS,
			KEYBOARDKEY_LEFTSQUAREBRACKET, KEYBOARDKEY_RIGHTSQUAREBRACKET,
			KEYBOARDKEY_SEMICOLON, KEYBOARDKEY_SINGLEQUOTE, KEYBOARDKEY_HASH,
			KEYBOARDKEY_BACKSLASH, KEYBOARDKEY_COMMA, KEYBOARDKEY_DOT, KEYBOARDKEY_FORWARDSLASH,

			KEYBOARDKEY_BACKSPACE, KEYBOARDKEY_RETURN, KEYBOARDKEY_TAB, KEYBOARDKEY_CAPSLOCK,
			KEYBOARDKEY_LEFTSHIFT, KEYBOARDKEY_RIGHTSHIFT,
			KEYBOARDKEY_LEFTCTRL, KEYBOARDKEY_RIGHTCTRL,
			KEYBOARDKEY_LEFTALT, KEYBOARDKEY_RIGHTALT,
			KEYBOARDKEY_SPACE,

			KEYBOARDKEY_LEFTWINDOWS, KEYBOARDKEY_RIGHTWINDOWS,
			KEYBOARDKEY_CONTEXTMENU,

			KEYBOARDKEY_INSERT, KEYBOARDKEY_DELETE, KEYBOARDKEY_HOME, KEYBOARDKEY_END,
			KEYBOARDKEY_PAGEUP, KEYBOARDKEY_PAGEDOWN,

			KEYBOARDKEY_NUMLOCK, KEYBOARDKEY_NUMDIVIDE, KEYBOARDKEY_NUMMULTIPLY, KEYBOARDKEY_NUMMINUS, KEYBOARDKEY_NUMPLUS, KEYBOARDKEY_NUMENTER, KEYBOARDKEY_NUMDOT,
			KEYBOARDKEY_NUM1, KEYBOARDKEY_NUM2, KEYBOARDKEY_NUM3, KEYBOARDKEY_NUM4, KEYBOARDKEY_NUM5, KEYBOARDKEY_NUM6, KEYBOARDKEY_NUM7, KEYBOARDKEY_NUM8, KEYBOARDKEY_NUM9, KEYBOARDKEY_NUM0,

			KEYBOARDKEY_ARROWLEFT, KEYBOARDKEY_ARROWRIGHT, KEYBOARDKEY_ARROWUP, KEYBOARDKEY_ARROWDOWN,

			KEYBOARDKEY_UNRECOGNISED,

			// MOUSE:
			MOUSEBUTTON_LEFT, MOUSEBUTTON_MIDDLE, MOUSEBUTTON_RIGHT,
			MOUSEBUTTON_X1, MOUSEBUTTON_X2, MOUSEBUTTON_X3, MOUSEBUTTON_X4, MOUSEBUTTON_X5, MOUSEBUTTON_X6, MOUSEBUTTON_X7, MOUSEBUTTON_X8, MOUSEBUTTON_X9, MOUSEBUTTON_X10, MOUSEBUTTON_X11, MOUSEBUTTON_X12, MOUSEBUTTON_X13, MOUSEBUTTON_X14, MOUSEBUTTON_X15, MOUSEBUTTON_X16, MOUSEBUTTON_X17, MOUSEBUTTON_X18, MOUSEBUTTON_X19, MOUSEBUTTON_X20,

			MOUSESCROLLWHEEL_X, MOUSESCROLLWHEEL_Y,

			MOUSEPOSITION_X, MOUSEPOSITION_Y,
			MOUSEMOTION_X, MOUSEMOTION_Y,

			// GAMEPAD:
			GAMEPADBUTTON_X, GAMEPADBUTTON_Y, GAMEPADBUTTON_A, GAMEPADBUTTON_B,
			GAMEPADBUTTON_DPADLEFT, GAMEPADBUTTON_DPADRIGHT, GAMEPADBUTTON_DPADUP, GAMEPADBUTTON_DPADDOWN,
			GAMEPADBUTTON_LEFTBUMPER, GAMEPADBUTTON_RIGHTBUMPER,
			GAMEPADBUTTON_BACK, GAMEPADBUTTON_START, GAMEPADBUTTON_GUIDE,
			GAMEPADBUTTON_LEFTSTICK, GAMEPADBUTTON_RIGHTSTICK,

			GAMEPADSTICK_LEFTX, GAMEPADSTICK_LEFTY,
			GAMEPADSTICK_RIGHTX, GAMEPADSTICK_RIGHTY,

			GAMEPADTRIGGER_LEFT, GAMEPADTRIGGER_RIGHT,

			SIZE, // placeholder...
		};

		auto constexpr control_id_size = static_cast<std::size_t>(control_id::SIZE);

		std::string to_string(control_id id);
		control_id from_string(std::string const& str);
		
		struct raw_input
		{
			float m_value = 0.f;
			bool m_normalized = true;
		};

		struct input_callbacks
		{
			std::function<void()> m_quit;
			std::function<void(bool)> m_pause; // true is paused
			std::function<void(control_id, raw_input)> m_input;
			std::function<void(glm::i32vec2)> m_resize;
		};
		
	} // input
	
} // bump