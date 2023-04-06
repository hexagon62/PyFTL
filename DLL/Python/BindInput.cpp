#include "Bind.hpp"
#include "../Player/Input.hpp"
#include "../State/State.hpp"

namespace python_bindings
{

void bindInput(py::module_& module)
{
	auto&& sub = module.def_submodule("input", "Module for input into FTL");

	py::enum_<Key>(sub, "Key", "A keyboard key")
		.value("Unknown", Key::Unknown)
		.value("Backspace", Key::Backspace)
		.value("Tab", Key::Tab)
		.value("Clear", Key::Clear)
		.value("Enter", Key::Return)
		.value("Pause", Key::Pause)
		.value("Escape", Key::Escape)
		.value("Space", Key::Space)
		.value("!", Key::Exclaim)
		.value("\"", Key::DoubleQuote)
		.value("#", Key::Hash)
		.value("$", Key::Dollar)
		.value("&", Key::Ampersand)
		.value("'", Key::Quote)
		.value("(", Key::LeftParenthesis)
		.value(")", Key::RightParenthesis)
		.value("*", Key::Asterisk)
		.value("+", Key::Plus)
		.value(",", Key::Comma)
		.value("-", Key::Minus)
		.value(".", Key::Period)
		.value("/", Key::Slash)
		.value("0", Key::Num0)
		.value("1", Key::Num1)
		.value("2", Key::Num2)
		.value("3", Key::Num3)
		.value("4", Key::Num4)
		.value("5", Key::Num5)
		.value("6", Key::Num6)
		.value("7", Key::Num7)
		.value("8", Key::Num8)
		.value("9", Key::Num9)
		.value(":", Key::Colon)
		.value(";", Key::Semicolon)
		.value("<", Key::Less)
		.value("=", Key::Equals)
		.value(">", Key::Greater)
		.value("?", Key::Question)
		.value("@", Key::At)
		.value("[", Key::LeftBracket)
		.value("\\", Key::BackSlash)
		.value("]", Key::RightBracket)
		.value("^", Key::Caret)
		.value("_", Key::Underscore)
		.value("`", Key::Backquote)
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
		.value("Pad.", Key::NumpadPeriod)
		.value("Pad/", Key::NumpadDivide)
		.value("Pad*", Key::NumpadMultiply)
		.value("Pad-", Key::NumpadMinus)
		.value("Pad+", Key::NumpadPlus)
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
		;

	py::enum_<MouseButton>(sub, "MouseButton", "A mouse button")
		.value("Left", MouseButton::Left)
		.value("Middle", MouseButton::Middle)
		.value("Right", MouseButton::Right)
		;

	sub.attr("GAME_WIDTH") = Input::GAME_WIDTH;
	sub.attr("GAME_HEIGHT") = Input::GAME_HEIGHT;
	//sub.attr("GAME_WIDTH").doc() = "The game's base width (1280 pixels)";
	//sub.attr("GAME_HEIGHT").doc() = "The game's base height (720 pixels)";

	sub.def("inputs", &Input::inputs, "Check if any inputs are queued");
	sub.def("allow_human_mouse", &Input::allowHumanMouse, py::arg("allow") = true, "Set if human mouse input is allowed");
	sub.def("allow_human_keyboard", &Input::allowHumanKeyboard, py::arg("allow") = true, "Set if human keyboard input is allowed");
	sub.def("human_mouse_allowed", &Input::humanMouseAllowed, "Check if human mouse input is allowed");
	sub.def("human_keyboard_allowed", &Input::humanKeyboardAllowed, "Check if human keyboard input is allowed");
		
	sub.def(
		"mouse_move",
		py::overload_cast<const Point<int>&, Duration>(& Input::mouseMove),
		py::arg("position"),
		py::arg("delay") = ZERO_DURATION,
		"Send a command to move the mouse"
	);

	sub.def(
		"mouse_move",
		py::overload_cast<int, int, Duration>(&Input::mouseMove),
		py::arg("x"), py::arg("y"),
		py::arg("delay") = ZERO_DURATION,
		"Send a command to move the mouse"
	);

	sub.def(
		"mouse_down",
		py::overload_cast<const Point<int>&, MouseButton, bool, bool, Duration>(&Input::mouseDown),
		py::arg("position"),
		py::arg("button") = MouseButton::Left,
		py::arg("shift") = false,
		py::arg("ctrl") = false,
		py::arg("delay") = ZERO_DURATION,
		"Send a command to hold the mouse button down"
	);

	sub.def(
		"mouse_down",
		py::overload_cast<int, int, MouseButton, bool, bool, Duration>(&Input::mouseDown),
		py::arg("x"), py::arg("y"),
		py::arg("button") = MouseButton::Left,
		py::arg("shift") = false,
		py::arg("ctrl") = false,
		py::arg("delay") = ZERO_DURATION,
		"Send a command to hold the mouse button down"
	);

	sub.def(
		"mouse_up",
		py::overload_cast<const Point<int>&, MouseButton, bool, bool, Duration>(&Input::mouseUp),
		py::arg("position"),
		py::arg("button") = MouseButton::Left,
		py::arg("shift") = false,
		py::arg("ctrl") = false,
		py::arg("delay") = ZERO_DURATION,
		"Send a command to release the mouse button"
	);

	sub.def(
		"mouse_up",
		py::overload_cast<int, int, MouseButton, bool, bool, Duration>(&Input::mouseUp),
		py::arg("x"), py::arg("y"),
		py::arg("button") = MouseButton::Left,
		py::arg("shift") = false,
		py::arg("ctrl") = false,
		py::arg("delay") = ZERO_DURATION,
		"Send a command to release the mouse button"
	);

	sub.def(
		"mouse_click",
		py::overload_cast<const Point<int>&, MouseButton, bool, bool, Duration>(&Input::mouseClick),
		py::arg("position"),
		py::arg("button") = MouseButton::Left,
		py::arg("shift") = false,
		py::arg("ctrl") = false,
		py::arg("delay") = ZERO_DURATION,
		"Send a command to click with the mouse button"
	);

	sub.def(
		"mouse_click",
		py::overload_cast<int, int, MouseButton, bool, bool, Duration>(&Input::mouseClick),
		py::arg("x"), py::arg("y"),
		py::arg("button") = MouseButton::Left,
		py::arg("shift") = false,
		py::arg("ctrl") = false,
		py::arg("delay") = ZERO_DURATION,
		"Send a command to click with the mouse button"
	);
}

}
