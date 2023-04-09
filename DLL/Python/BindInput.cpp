#include "Bind.hpp"
#include "../Input.hpp"
#include "../State.hpp"

namespace python_bindings
{

void bindInput(py::module_& module)
{
	auto&& sub = module.def_submodule(
		"input",
		"Submodule for inputting into FTL\n"
		"All methods that send commands take a parameter called 'delay'\n"
		"which delays the input for that many seconds.\n"
		"The methods will return the following in a tuple:\n"
		"- the id of the commands in the queue\n"
		"- the time the command will be executed\n"
		"Time is in seconds and is relative to the start of PyFTL"
	);

	py::enum_<Key>(sub, "Key", "A keyboard key")
		.value("Unknown", Key::Unknown)
		.value("Backspace", Key::Backspace)
		.value("Tab", Key::Tab)
		.value("Clear", Key::Clear)
		.value("Enter", Key::Return)
		.value("Pause", Key::Pause)
		.value("Escape", Key::Escape)
		.value("Space", Key::Space)
		.value("Exclaim", Key::Exclaim)
		.value("DoubleQuote", Key::DoubleQuote)
		.value("Hash", Key::Hash)
		.value("Dollar", Key::Dollar)
		.value("Ampersand", Key::Ampersand)
		.value("Quote", Key::Quote)
		.value("LeftParen", Key::LeftParenthesis)
		.value("RightParen", Key::RightParenthesis)
		.value("Asterisk", Key::Asterisk)
		.value("Plus", Key::Plus)
		.value("Comma", Key::Comma)
		.value("Minus", Key::Minus)
		.value("Period", Key::Period)
		.value("Slash", Key::Slash)
		.value("Num0", Key::Num0)
		.value("Num1", Key::Num1)
		.value("Num2", Key::Num2)
		.value("Num3", Key::Num3)
		.value("Num4", Key::Num4)
		.value("Num5", Key::Num5)
		.value("Num6", Key::Num6)
		.value("Num7", Key::Num7)
		.value("Num8", Key::Num8)
		.value("Num9", Key::Num9)
		.value("Colon", Key::Colon)
		.value("Semicolon", Key::Semicolon)
		.value("Less", Key::Less)
		.value("Equals", Key::Equals)
		.value("Greater", Key::Greater)
		.value("Question", Key::Question)
		.value("At", Key::At)
		.value("LeftBracket", Key::LeftBracket)
		.value("BackSlash", Key::BackSlash)
		.value("RightBracket", Key::RightBracket)
		.value("Caret", Key::Caret)
		.value("_", Key::Underscore)
		.value("Backquote", Key::Backquote)
		.value("A", Key::A)
		.value("B", Key::B)
		.value("C", Key::C)
		.value("D", Key::D)
		.value("E", Key::E)
		.value("F", Key::F)
		.value("G", Key::G)
		.value("H", Key::H)
		.value("I", Key::I)
		.value("J", Key::J)
		.value("K", Key::K)
		.value("L", Key::L)
		.value("M", Key::M)
		.value("N", Key::N)
		.value("O", Key::O)
		.value("P", Key::P)
		.value("Q", Key::Q)
		.value("R", Key::R)
		.value("S", Key::S)
		.value("T", Key::T)
		.value("U", Key::U)
		.value("V", Key::V)
		.value("W", Key::W)
		.value("X", Key::X)
		.value("Y", Key::Y)
		.value("Z", Key::Z)
		.value("Delete", Key::Delete)
		.value("Pad0", Key::Numpad0)
		.value("Pad1", Key::Numpad1)
		.value("Pad2", Key::Numpad2)
		.value("Pad3", Key::Numpad3)
		.value("Pad4", Key::Numpad4)
		.value("Pad5", Key::Numpad5)
		.value("Pad6", Key::Numpad6)
		.value("Pad7", Key::Numpad7)
		.value("Pad8", Key::Numpad8)
		.value("Pad9", Key::Numpad9)
		.value("PadPeriod", Key::NumpadPeriod)
		.value("PadDivide", Key::NumpadDivide)
		.value("PadMultiply", Key::NumpadMultiply)
		.value("PadMinus", Key::NumpadMinus)
		.value("PadPlus", Key::NumpadPlus)
		.value("PadEnter", Key::NumpadEnter)
		.value("Up", Key::Up)
		.value("Down", Key::Down)
		.value("Right", Key::Right)
		.value("Left", Key::Left)
		.value("Insert", Key::Insert)
		.value("Home", Key::Home)
		.value("End", Key::End)
		.value("PageUp", Key::PageUp)
		.value("PageDown", Key::PageDown)
		.value("F1", Key::F1)
		.value("F2", Key::F2)
		.value("F3", Key::F3)
		.value("F4", Key::F4)
		.value("F5", Key::F5)
		.value("F6", Key::F6)
		.value("F7", Key::F7)
		.value("F8", Key::F8)
		.value("F9", Key::F9)
		.value("F10", Key::F10)
		.value("F11", Key::F11)
		.value("F12", Key::F12)
		.value("F13", Key::F13)
		.value("F14", Key::F14)
		.value("F15", Key::F15)
		.value("NumLock", Key::NumLock)
		.value("CapsLock", Key::CapsLock)
		.value("ScrollLock", Key::ScrollLock)
		.value("RShift", Key::RShift)
		.value("LShift", Key::LShift)
		.value("RCtrl", Key::RCtrl)
		.value("LCtrl", Key::LCtrl)
		.value("RAlt", Key::RAlt)
		.value("LAlt", Key::LAlt)
		.value("RMeta", Key::RMeta)
		.value("LMeta", Key::LMeta)
		.value("LSuper", Key::LSuper)
		.value("RSuper", Key::RSuper)
		.value("AltGr", Key::AltGr)
		.value("Compose", Key::Compose)
		.value("Help", Key::Help)
		.value("Print", Key::Print)
		.value("SysReq", Key::SysReq)
		.value("Break", Key::Break)
		.value("Menu", Key::Menu)
		.value("Power", Key::Power)
		.value("Euro", Key::Euro)
		.value("Undo", Key::Undo)
		;

	py::enum_<MouseButton>(sub, "MouseButton", "A mouse button")
		.value("None", MouseButton::None)
		.value("Left", MouseButton::Left)
		.value("Middle", MouseButton::Middle)
		.value("Right", MouseButton::Right)
		;

	sub.attr("GAME_WIDTH") = Input::GAME_WIDTH;
	sub.attr("GAME_HEIGHT") = Input::GAME_HEIGHT;
	//sub.attr("GAME_WIDTH").doc() = "The game's base width (1280 pixels)";
	//sub.attr("GAME_HEIGHT").doc() = "The game's base height (720 pixels)";

	sub.def(
		"empty",
		&Input::empty,
		"Check if no inputs are queued."
	);

	sub.def(
		"queued",
		&Input::queued,
		"Check if an input with this id is still queued."
	);

	sub.def(
		"pop",
		&Input::pop,
		"Dequeues the input with this id; returns false if the input wasn't in the queue."
	);

	sub.def(
		"allow_human_mouse",
		&Input::allowHumanMouse,
		py::arg("allow") = true,
		"Set if human mouse input is allowed."
	);

	sub.def(
		"allow_human_keyboard",
		&Input::allowHumanKeyboard,
		py::arg("allow") = true,
		"Set if human keyboard input is allowed."
	);

	sub.def(
		"human_mouse_allowed",
		&Input::humanMouseAllowed,
		"Check if human mouse input is allowed."
	);

	sub.def(
		"human_keyboard_allowed",
		&Input::humanKeyboardAllowed,
		"Check if human keyboard input is allowed."
	);

	sub.def(
		"clear_queue",
		&Input::clear,
		"Clears the input queue."
	);

	sub.def(
		"mouse_move",
		&Input::mouseMove,
		py::arg("position"),
		py::arg("delay") = 0.0,
		"Queue a command to move the mouse"
	);

	sub.def(
		"mouse_down",
		&Input::mouseDown,
		py::arg("button") = MouseButton::Left,
		py::kw_only(),
		py::arg("position") = Point<int>(-1, -1),
		py::arg("shift") = false,
		py::arg("delay") = 0.0,
		"Queue a command to hold a mouse button down."
	);

	sub.def(
		"mouse_up",
		&Input::mouseUp,
		py::arg("button") = MouseButton::Left,
		py::kw_only(),
		py::arg("position") = Point<int>(-1, -1),
		py::arg("shift") = false,
		py::arg("delay") = 0.0,
		"Queue a command to release a mouse button."
	);

	sub.def(
		"mouse_click",
		&Input::mouseClick,
		py::arg("position") = Point<int>(-1, -1),
		py::arg("button") = MouseButton::Left,
		py::kw_only(),
		py::arg("shift") = false,
		py::arg("delay") = 0.0,
		"Queue a command to click a mouse button."
	);

	sub.def(
		"key_down",
		&Input::keyDown,
		py::arg("key"),
		py::kw_only(),
		py::arg("shift") = false,
		py::arg("delay") = 0.0,
		"Queue a command to hold a keyboard key down."
	);

	sub.def(
		"key_up",
		&Input::keyUp,
		py::arg("key"),
		py::kw_only(),
		py::arg("shift") = false,
		py::arg("delay") = 0.0,
		"Queue a command to release a keyboard key."
	);

	sub.def(
		"key_press",
		&Input::keyPress,
		py::arg("key"),
		py::kw_only(),
		py::arg("shift") = false,
		py::arg("delay") = 0.0,
		"Queue a command to press a keyboard key."
	);

	sub.def(
		"hotkey_down",
		&Input::hotkeyDown,
		py::arg("hotkey"),
		py::kw_only(),
		py::arg("shift") = false,
		py::arg("delay") = 0.0,
		"Queue a command to hold a named hotkey down."
	);

	sub.def(
		"hotkey_up",
		&Input::hotkeyUp,
		py::arg("hotkey"),
		py::kw_only(),
		py::arg("shift") = false,
		py::arg("delay") = 0.0,
		"Queue a command to release a named hotkey."
	);

	sub.def(
		"hotkey_press",
		&Input::hotkeyPress,
		py::arg("hotkey"),
		py::kw_only(),
		py::arg("shift") = false,
		py::arg("delay") = 0.0,
		"Queue a command to press a named hotkey."
	);

	sub.def(
		"hotkeys",
		&Input::hotkeys,
		py::return_value_policy::reference,
		"Returns the hotkey dictionary."
	);

	sub.def(
		"event_choice",
		&Input::eventChoice,
		py::arg("which"),
		py::kw_only(),
		py::arg("suppress") = false,
		py::arg("delay") = 0.0,
		"Queue a command to select a choice at an event.\n"
		"Events are zero-indexed, so pass in 0 for choice #1.\n"
		"Set 'suppress' to True to surpress non-critical exceptions.\n"
		"This will then instead input nothing instead of throwing an exception."
	);

	sub.def(
		"system_power",
		&Input::systemPower,
		py::arg("system"),
		py::arg("set") = 0,
		py::kw_only(),
		py::arg("delta") = 0,
		py::arg("suppress") = false,
		py::arg("delay") = 0.0,
		"Queue a command to change a system's power.\n"
		"Specify 'set' to change to an amount. Specify 'delta' to change by an amount.\n"
		"Setting both is invalid and will result in an exception.\n"
		"Set 'suppress' to True to surpress non-critical exceptions.\n"
		"This will input commands as long as it's possible.\n"
		"This allows you to click on a system even if you can't actually change its power state."
	);
}

}
