#include "bump_input.hpp"

#include "bump_pair_map.hpp"

namespace bump
{
	
	namespace input
	{
		
		namespace
		{

			using pair_type = std::pair<control_id, std::string>;

			auto control_id_strings = pair_map<control_id, std::string>
			{
#pragma region keyboard

				pair_type{ control_id::KEYBOARDKEY_A, "KEYBOARDKEY_A" },
				pair_type{ control_id::KEYBOARDKEY_B, "KEYBOARDKEY_B" },
				pair_type{ control_id::KEYBOARDKEY_C, "KEYBOARDKEY_C" },
				pair_type{ control_id::KEYBOARDKEY_D, "KEYBOARDKEY_D" },
				pair_type{ control_id::KEYBOARDKEY_E, "KEYBOARDKEY_E" },
				pair_type{ control_id::KEYBOARDKEY_F, "KEYBOARDKEY_F" },
				pair_type{ control_id::KEYBOARDKEY_G, "KEYBOARDKEY_G" },
				pair_type{ control_id::KEYBOARDKEY_H, "KEYBOARDKEY_H" },
				pair_type{ control_id::KEYBOARDKEY_I, "KEYBOARDKEY_I" },
				pair_type{ control_id::KEYBOARDKEY_J, "KEYBOARDKEY_J" },
				pair_type{ control_id::KEYBOARDKEY_K, "KEYBOARDKEY_K" },
				pair_type{ control_id::KEYBOARDKEY_L, "KEYBOARDKEY_L" },
				pair_type{ control_id::KEYBOARDKEY_M, "KEYBOARDKEY_M" },
				pair_type{ control_id::KEYBOARDKEY_N, "KEYBOARDKEY_N" },
				pair_type{ control_id::KEYBOARDKEY_O, "KEYBOARDKEY_O" },
				pair_type{ control_id::KEYBOARDKEY_P, "KEYBOARDKEY_P" },
				pair_type{ control_id::KEYBOARDKEY_Q, "KEYBOARDKEY_Q" },
				pair_type{ control_id::KEYBOARDKEY_R, "KEYBOARDKEY_R" },
				pair_type{ control_id::KEYBOARDKEY_S, "KEYBOARDKEY_S" },
				pair_type{ control_id::KEYBOARDKEY_T, "KEYBOARDKEY_T" },
				pair_type{ control_id::KEYBOARDKEY_U, "KEYBOARDKEY_U" },
				pair_type{ control_id::KEYBOARDKEY_V, "KEYBOARDKEY_V" },
				pair_type{ control_id::KEYBOARDKEY_W, "KEYBOARDKEY_W" },
				pair_type{ control_id::KEYBOARDKEY_X, "KEYBOARDKEY_X" },
				pair_type{ control_id::KEYBOARDKEY_Y, "KEYBOARDKEY_Y" },
				pair_type{ control_id::KEYBOARDKEY_Z, "KEYBOARDKEY_Z" },

				pair_type{ control_id::KEYBOARDKEY_1, "KEYBOARDKEY_1" },
				pair_type{ control_id::KEYBOARDKEY_2, "KEYBOARDKEY_2" },
				pair_type{ control_id::KEYBOARDKEY_3, "KEYBOARDKEY_3" },
				pair_type{ control_id::KEYBOARDKEY_4, "KEYBOARDKEY_4" },
				pair_type{ control_id::KEYBOARDKEY_5, "KEYBOARDKEY_5" },
				pair_type{ control_id::KEYBOARDKEY_6, "KEYBOARDKEY_6" },
				pair_type{ control_id::KEYBOARDKEY_7, "KEYBOARDKEY_7" },
				pair_type{ control_id::KEYBOARDKEY_8, "KEYBOARDKEY_8" },
				pair_type{ control_id::KEYBOARDKEY_9, "KEYBOARDKEY_9" },
				pair_type{ control_id::KEYBOARDKEY_0, "KEYBOARDKEY_0" },

				pair_type{ control_id::KEYBOARDKEY_F1, "KEYBOARDKEY_F1" },
				pair_type{ control_id::KEYBOARDKEY_F2, "KEYBOARDKEY_F2" },
				pair_type{ control_id::KEYBOARDKEY_F3, "KEYBOARDKEY_F3" },
				pair_type{ control_id::KEYBOARDKEY_F4, "KEYBOARDKEY_F4" },
				pair_type{ control_id::KEYBOARDKEY_F5, "KEYBOARDKEY_F5" },
				pair_type{ control_id::KEYBOARDKEY_F6, "KEYBOARDKEY_F6" },
				pair_type{ control_id::KEYBOARDKEY_F7, "KEYBOARDKEY_F7" },
				pair_type{ control_id::KEYBOARDKEY_F8, "KEYBOARDKEY_F8" },
				pair_type{ control_id::KEYBOARDKEY_F9, "KEYBOARDKEY_F9" },
				pair_type{ control_id::KEYBOARDKEY_F10, "KEYBOARDKEY_F10" },
				pair_type{ control_id::KEYBOARDKEY_F11, "KEYBOARDKEY_F11" },
				pair_type{ control_id::KEYBOARDKEY_F12, "KEYBOARDKEY_F12" },

				pair_type{ control_id::KEYBOARDKEY_ESCAPE, "KEYBOARDKEY_ESCAPE" },

				pair_type{ control_id::KEYBOARDKEY_PRINTSCREEN, "KEYBOARDKEY_PRINTSCREEN" },
				pair_type{ control_id::KEYBOARDKEY_SCROLLLOCK, "KEYBOARDKEY_SCROLLLOCK" },
				pair_type{ control_id::KEYBOARDKEY_PAUSE, "KEYBOARDKEY_PAUSE" },

				pair_type{ control_id::KEYBOARDKEY_BACKTICK, "KEYBOARDKEY_BACKTICK" },
				pair_type{ control_id::KEYBOARDKEY_MINUS, "KEYBOARDKEY_MINUS" },
				pair_type{ control_id::KEYBOARDKEY_EQUALS, "KEYBOARDKEY_EQUALS" },

				pair_type{ control_id::KEYBOARDKEY_LEFTSQUAREBRACKET, "KEYBOARDKEY_LEFTSQUAREBRACKET" },
				pair_type{ control_id::KEYBOARDKEY_RIGHTSQUAREBRACKET, "KEYBOARDKEY_RIGHTSQUAREBRACKET" },

				pair_type{ control_id::KEYBOARDKEY_SEMICOLON, "KEYBOARDKEY_SEMICOLON" },
				pair_type{ control_id::KEYBOARDKEY_SINGLEQUOTE, "KEYBOARDKEY_SINGLEQUOTE" },
				pair_type{ control_id::KEYBOARDKEY_HASH, "KEYBOARDKEY_HASH" },

				pair_type{ control_id::KEYBOARDKEY_BACKSLASH, "KEYBOARDKEY_BACKSLASH" },
				pair_type{ control_id::KEYBOARDKEY_COMMA, "KEYBOARDKEY_COMMA" },
				pair_type{ control_id::KEYBOARDKEY_DOT, "KEYBOARDKEY_DOT" },
				pair_type{ control_id::KEYBOARDKEY_FORWARDSLASH, "KEYBOARDKEY_FORWARDSLASH" },

				pair_type{ control_id::KEYBOARDKEY_BACKSPACE, "KEYBOARDKEY_BACKSPACE" },
				pair_type{ control_id::KEYBOARDKEY_RETURN, "KEYBOARDKEY_RETURN" },
				pair_type{ control_id::KEYBOARDKEY_TAB, "KEYBOARDKEY_TAB" },
				pair_type{ control_id::KEYBOARDKEY_CAPSLOCK, "KEYBOARDKEY_CAPSLOCK" },

				pair_type{ control_id::KEYBOARDKEY_LEFTSHIFT, "KEYBOARDKEY_LEFTSHIFT" },
				pair_type{ control_id::KEYBOARDKEY_RIGHTSHIFT, "KEYBOARDKEY_RIGHTSHIFT" },

				pair_type{ control_id::KEYBOARDKEY_LEFTCTRL, "KEYBOARDKEY_LEFTCTRL" },
				pair_type{ control_id::KEYBOARDKEY_RIGHTCTRL, "KEYBOARDKEY_RIGHTCTRL" },

				pair_type{ control_id::KEYBOARDKEY_LEFTALT, "KEYBOARDKEY_LEFTALT" },
				pair_type{ control_id::KEYBOARDKEY_RIGHTALT, "KEYBOARDKEY_RIGHTALT" },

				pair_type{ control_id::KEYBOARDKEY_SPACE, "KEYBOARDKEY_SPACE" },

				pair_type{ control_id::KEYBOARDKEY_LEFTWINDOWS, "KEYBOARDKEY_LEFTWINDOWS" },
				pair_type{ control_id::KEYBOARDKEY_RIGHTWINDOWS, "KEYBOARDKEY_RIGHTWINDOWS" },

				pair_type{ control_id::KEYBOARDKEY_CONTEXTMENU, "KEYBOARDKEY_CONTEXTMENU" },

				pair_type{ control_id::KEYBOARDKEY_INSERT, "KEYBOARDKEY_INSERT" },
				pair_type{ control_id::KEYBOARDKEY_DELETE, "KEYBOARDKEY_DELETE" },
				pair_type{ control_id::KEYBOARDKEY_HOME, "KEYBOARDKEY_HOME" },
				pair_type{ control_id::KEYBOARDKEY_END, "KEYBOARDKEY_END" },

				pair_type{ control_id::KEYBOARDKEY_PAGEUP, "KEYBOARDKEY_PAGEUP" },
				pair_type{ control_id::KEYBOARDKEY_PAGEDOWN, "KEYBOARDKEY_PAGEDOWN" },

				pair_type{ control_id::KEYBOARDKEY_NUMLOCK, "KEYBOARDKEY_NUMLOCK" },
				pair_type{ control_id::KEYBOARDKEY_NUMDIVIDE, "KEYBOARDKEY_NUMDIVIDE" },
				pair_type{ control_id::KEYBOARDKEY_NUMMULTIPLY, "KEYBOARDKEY_NUMMULTIPLY" },
				pair_type{ control_id::KEYBOARDKEY_NUMMINUS, "KEYBOARDKEY_NUMMINUS" },
				pair_type{ control_id::KEYBOARDKEY_NUMPLUS, "KEYBOARDKEY_NUMPLUS" },
				pair_type{ control_id::KEYBOARDKEY_NUMENTER, "KEYBOARDKEY_NUMENTER" },
				pair_type{ control_id::KEYBOARDKEY_NUMDOT, "KEYBOARDKEY_NUMDOT" },

				pair_type{ control_id::KEYBOARDKEY_NUM1, "KEYBOARDKEY_NUM1" },
				pair_type{ control_id::KEYBOARDKEY_NUM2, "KEYBOARDKEY_NUM2" },
				pair_type{ control_id::KEYBOARDKEY_NUM3, "KEYBOARDKEY_NUM3" },
				pair_type{ control_id::KEYBOARDKEY_NUM4, "KEYBOARDKEY_NUM4" },
				pair_type{ control_id::KEYBOARDKEY_NUM5, "KEYBOARDKEY_NUM5" },
				pair_type{ control_id::KEYBOARDKEY_NUM6, "KEYBOARDKEY_NUM6" },
				pair_type{ control_id::KEYBOARDKEY_NUM7, "KEYBOARDKEY_NUM7" },
				pair_type{ control_id::KEYBOARDKEY_NUM8, "KEYBOARDKEY_NUM8" },
				pair_type{ control_id::KEYBOARDKEY_NUM9, "KEYBOARDKEY_NUM9" },
				pair_type{ control_id::KEYBOARDKEY_NUM0, "KEYBOARDKEY_NUM0" },

				pair_type{ control_id::KEYBOARDKEY_ARROWLEFT, "KEYBOARDKEY_ARROWLEFT" },
				pair_type{ control_id::KEYBOARDKEY_ARROWRIGHT, "KEYBOARDKEY_ARROWRIGHT" },
				pair_type{ control_id::KEYBOARDKEY_ARROWUP, "KEYBOARDKEY_ARROWUP" },
				pair_type{ control_id::KEYBOARDKEY_ARROWDOWN, "KEYBOARDKEY_ARROWDOWN" },

				pair_type{ control_id::KEYBOARDKEY_UNRECOGNISED, "KEYBOARDKEY_UNRECOGNISED" },

#pragma endregion

#pragma region mouse

				pair_type{ control_id::MOUSEBUTTON_LEFT, "MOUSEBUTTON_LEFT" },
				pair_type{ control_id::MOUSEBUTTON_MIDDLE, "MOUSEBUTTON_MIDDLE" },
				pair_type{ control_id::MOUSEBUTTON_RIGHT, "MOUSEBUTTON_RIGHT" },

				pair_type{ control_id::MOUSEBUTTON_X1, "MOUSEBUTTON_X1" },
				pair_type{ control_id::MOUSEBUTTON_X2, "MOUSEBUTTON_X2" },
				pair_type{ control_id::MOUSEBUTTON_X3, "MOUSEBUTTON_X3" },
				pair_type{ control_id::MOUSEBUTTON_X4, "MOUSEBUTTON_X4" },
				pair_type{ control_id::MOUSEBUTTON_X5, "MOUSEBUTTON_X5" },
				pair_type{ control_id::MOUSEBUTTON_X6, "MOUSEBUTTON_X6" },
				pair_type{ control_id::MOUSEBUTTON_X7, "MOUSEBUTTON_X7" },
				pair_type{ control_id::MOUSEBUTTON_X8, "MOUSEBUTTON_X8" },
				pair_type{ control_id::MOUSEBUTTON_X9, "MOUSEBUTTON_X9" },
				pair_type{ control_id::MOUSEBUTTON_X10, "MOUSEBUTTON_X10" },
				pair_type{ control_id::MOUSEBUTTON_X11, "MOUSEBUTTON_X11" },
				pair_type{ control_id::MOUSEBUTTON_X12, "MOUSEBUTTON_X12" },
				pair_type{ control_id::MOUSEBUTTON_X13, "MOUSEBUTTON_X13" },
				pair_type{ control_id::MOUSEBUTTON_X14, "MOUSEBUTTON_X14" },
				pair_type{ control_id::MOUSEBUTTON_X15, "MOUSEBUTTON_X15" },
				pair_type{ control_id::MOUSEBUTTON_X16, "MOUSEBUTTON_X16" },
				pair_type{ control_id::MOUSEBUTTON_X17, "MOUSEBUTTON_X17" },
				pair_type{ control_id::MOUSEBUTTON_X18, "MOUSEBUTTON_X18" },
				pair_type{ control_id::MOUSEBUTTON_X19, "MOUSEBUTTON_X19" },
				pair_type{ control_id::MOUSEBUTTON_X20, "MOUSEBUTTON_X20" },

				pair_type{ control_id::MOUSESCROLLWHEEL_Y, "MOUSESCROLLWHEEL_Y" },
				pair_type{ control_id::MOUSESCROLLWHEEL_X, "MOUSESCROLLWHEEL_X" },

				pair_type{ control_id::MOUSEPOSITION_X, "MOUSEPOSITION_X" },
				pair_type{ control_id::MOUSEPOSITION_Y, "MOUSEPOSITION_Y" },
				pair_type{ control_id::MOUSEMOTION_X, "MOUSEMOTION_X" },
				pair_type{ control_id::MOUSEMOTION_Y, "MOUSEMOTION_Y" },

#pragma endregion

#pragma region gamepad

				pair_type{ control_id::GAMEPADBUTTON_X, "GAMEPADBUTTON_X" },
				pair_type{ control_id::GAMEPADBUTTON_Y, "GAMEPADBUTTON_Y" },
				pair_type{ control_id::GAMEPADBUTTON_A, "GAMEPADBUTTON_A" },
				pair_type{ control_id::GAMEPADBUTTON_B, "GAMEPADBUTTON_B" },

				pair_type{ control_id::GAMEPADBUTTON_DPADLEFT, "GAMEPADBUTTON_DPADLEFT" },
				pair_type{ control_id::GAMEPADBUTTON_DPADRIGHT, "GAMEPADBUTTON_DPADRIGHT" },
				pair_type{ control_id::GAMEPADBUTTON_DPADUP, "GAMEPADBUTTON_DPADUP" },
				pair_type{ control_id::GAMEPADBUTTON_DPADDOWN, "GAMEPADBUTTON_DPADDOWN" },

				pair_type{ control_id::GAMEPADBUTTON_LEFTBUMPER, "GAMEPADBUTTON_LEFTBUMPER" },
				pair_type{ control_id::GAMEPADBUTTON_RIGHTBUMPER, "GAMEPADBUTTON_RIGHTBUMPER" },

				pair_type{ control_id::GAMEPADBUTTON_LEFTSTICK, "GAMEPADBUTTON_LEFTSTICK" },
				pair_type{ control_id::GAMEPADBUTTON_RIGHTSTICK, "GAMEPADBUTTON_RIGHTSTICK" },

				pair_type{ control_id::GAMEPADBUTTON_BACK, "GAMEPADBUTTON_BACK" },
				pair_type{ control_id::GAMEPADBUTTON_START, "GAMEPADBUTTON_START" },
				pair_type{ control_id::GAMEPADBUTTON_GUIDE, "GAMEPADBUTTON_GUIDE" },

				pair_type{ control_id::GAMEPADSTICK_LEFTX, "GAMEPADSTICK_LEFTX" },
				pair_type{ control_id::GAMEPADSTICK_LEFTY, "GAMEPADSTICK_LEFTY" },
				pair_type{ control_id::GAMEPADSTICK_RIGHTX, "GAMEPADSTICK_RIGHTX" },
				pair_type{ control_id::GAMEPADSTICK_RIGHTY, "GAMEPADSTICK_RIGHTY" },

				pair_type{ control_id::GAMEPADTRIGGER_LEFT, "GAMEPADTRIGGER_LEFT" },
				pair_type{ control_id::GAMEPADTRIGGER_RIGHT, "GAMEPADTRIGGER_RIGHT" },

#pragma endregion

			};

		} // unnamed
	
		std::string to_string(control_id id)
		{
			return control_id_strings.get_second(id);
		}

		control_id from_string(std::string const& str)
		{
			return control_id_strings.get_first(str);
		}
		
	} // input
	
} // bump