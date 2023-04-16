#include "Bind.hpp"
#include "../Input.hpp"
#include "../State.hpp"

namespace python_bindings
{

void bindInput(py::module_& module)
{
	auto&& sub = module.def_submodule(
		"input",
		"Submodule for inputting into FTL\n\n"
		"All methods that send commands take a parameter called 'delay'\n"
		"which delays the input for that many seconds.\n"
		"The methods will return the following in a tuple:\n"
		"- the id of the commands in the queue\n"
		"- the time the command will be executed\n"
		"Time is in seconds and is relative to the start of PyFTL\n\n"
		"Many methods also take a parameter called 'suppress_exceptions'\n"
		"This stops PyFTL from throwing non-critical exceptions.\n"
		"This allows erroneous inputs to be attempted, if possible.\n"
		"An example of an erroneous input may be powering a system when you have no reactor power to spare."
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

	py::enum_<MouseButton>(sub, "Mouse", "A mouse button")
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
		"Check if no inputs are queued.\n"
		"This will always be the case if called in on_update.\n"
		"Because PyFTL will not run on_update until the input queue is empty."
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
		"dummy",
		&Input::dummy,
		"Returns the value you get when an input function does nothing."
	);

	sub.def(
		"wait",
		&Input::wait,
		py::arg("time"),
		"Queues a command to wait before subsequent inputs in the queue are executed."
	);

	sub.def(
		"mouse_move",
		&Input::mouseMove,
		py::arg("pos"),
		py::kw_only(),
		"Queue a command to move the mouse."
	);

	sub.def(
		"mouse_down",
		&Input::mouseDown,
		py::arg("button") = MouseButton::Left,
		py::arg("pos") = Point<int>(-1, -1),
		py::kw_only(),
		py::arg("shift") = false,
		"Queue a command to hold a mouse button down.\n"
		"Calls mouse_move before clicking. Default 'pos' argument makes mouse not move."
	);

	sub.def(
		"mouse_up",
		&Input::mouseUp,
		py::arg("button") = MouseButton::Left,
		py::arg("pos") = Point<int>(-1, -1),
		py::kw_only(),
		py::arg("shift") = false,
		"Queue a command to release a mouse button.\n"
		"Calls mouse_move before releasing. Default 'pos' argument makes mouse not move."
	);

	sub.def(
		"mouse_click",
		&Input::mouseClick,
		py::arg("button") = MouseButton::Left,
		py::arg("pos") = Point<int>(-1, -1),
		py::kw_only(),
		py::arg("shift") = false,
		"Calls mouse_move, mouse_down, and then mouse_up.\n"
		"Calls mouse_move before releasing. Default 'pos' argument makes mouse not move."
	);

	sub.def(
		"key_down",
		&Input::keyDown,
		py::arg("key"),
		py::kw_only(),
		py::arg("shift") = false,
		"Queue a command to hold a keyboard key down."
	);

	sub.def(
		"key_up",
		&Input::keyUp,
		py::arg("key"),
		py::kw_only(),
		py::arg("shift") = false,
		"Queue a command to release a keyboard key."
	);

	sub.def(
		"key_press",
		&Input::keyPress,
		py::arg("key"),
		py::kw_only(),
		py::arg("shift") = false,
		"Queue a command to press a keyboard key."
	);

	sub.def(
		"hotkey_down",
		&Input::hotkeyDown,
		py::arg("hotkey"),
		py::kw_only(),
		py::arg("shift") = false,
		"Queue a command to hold a named hotkey down."
	);

	sub.def(
		"hotkey_up",
		&Input::hotkeyUp,
		py::arg("hotkey"),
		py::kw_only(),
		py::arg("shift") = false,
		"Queue a command to release a named hotkey."
	);

	sub.def(
		"hotkey_press",
		&Input::hotkeyPress,
		py::arg("hotkey"),
		py::kw_only(),
		py::arg("shift") = false,
		"Queue a command to press a named hotkey."
	);

	sub.def(
		"hotkeys",
		&Input::hotkeys,
		py::return_value_policy::reference,
		"Returns the hotkey dictionary."
	);

	sub.def(
		"text",
		py::overload_cast<const std::string&>(&Input::text),
		py::arg("str"),
		"Inputs a string of text into a text input"
	);

	sub.def(
		"text_confirm",
		&Input::textConfirm,
		"Confirms the text entered into a text input"
	);

	sub.def(
		"text_clear",
		&Input::textClear,
		"Clears the text entered into a text input"
	);

	sub.def(
		"text_backspace",
		&Input::textBackspace,
		"Deletes the character before the cursor in a text input"
	);

	sub.def(
		"text_delete",
		&Input::textDelete,
		"Deletes the character after the cursor in a text input"
	);

	sub.def(
		"text_left",
		&Input::textLeft,
		"Moves the cursor one character to the left in a text input"
	);

	sub.def(
		"text_right",
		&Input::textLeft,
		"Moves the cursor one character to the right in a text input"
	);

	sub.def(
		"text_home",
		&Input::textHome,
		"Moves the cursor to the beginning of a text input"
	);

	sub.def(
		"text_end",
		&Input::textEnd,
		"Moves the cursor to the end of a text input"
	);

	sub.def(
		"cheat",
		&Input::cheat,
		py::arg("command"),
		"Are you a dirty cheater? Use this!\n"
		"This effectively lets you use the game's built-in console without enabling/opening it.\n"
		"This is not a queued input command and thus it will occur immediately.\n"
		"Do note the case-sensitivity of some parameters.\n"
		"Weapon/drone blueprints are generally ALL_CAPS, and crew are generally all_lower_case."
	);

	sub.def(
		"pause",
		&Input::pause,
		py::arg("on") = true,
		"Sets if the game is paused or not"
	);

	sub.def(
		"event_choice",
		&Input::choice,
		py::arg("which"),
		"Queue a command to select a choice at an event.\n"
		"Events are zero-indexed, so pass in 0 for choice #1."
	);

	sub.def(
		"power_system",
		&Input::powerSystem,
		py::arg("system"),
		py::arg("set") = 0,
		"Queue a command to change a system's power.\n\n"
		"Inputs will be tried in the below order:\n"
		"1) The direct unpower hotkey (if lowering power)\n"
		"2) The direct power hotkey (+shift if lowering power)\n"
		"3) The positional unpower hotkey (if lowering power)\n"
		"4) The positional power hotkey (+shift if lowering power)\n"
		"5) Clicking directly on the system"
	);

	sub.def(
		"power_weapon",
		&Input::powerWeapon,
		py::arg("weapon"),
		py::arg("on") = true,
		"Queue a command to toggle a weapon.\n"
		"By default it will turn the weapon on, set 'on' to false if you don't want this.\n\n"
		"Inputs will be tried in the below order:\n"
		"1) The slot hotkey (+shift if lowering power)\n"
		"2) Clicking directly on the slot"
	);

	sub.def(
		"power_drone",
		&Input::powerDrone,
		py::arg("drone"),
		py::arg("on") = true,
		"Queue a command to toggle a drone.\n"
		"By default it will turn the drone on, set 'on' to false if you don't want this.\n\n"
		"Inputs will be tried in the below order:\n"
		"1) The slot hotkey (+shift if lowering power)\n"
		"2) Clicking directly on the slot"
	);

	sub.def(
		"select_weapon",
		&Input::selectWeapon,
		py::arg("weapon"),
		"Queue a command to select a weapon."
	);

	sub.def(
		"select_crew",
		&Input::selectCrew,
		py::arg("crew"),
		"Queue a command to select a group of crew members.\n"
		"Note that the order of the list you pass in matters!"
	);

	sub.def(
		"swap_weapons",
		&Input::swapWeapons,
		py::arg("slot_a"),
		py::arg("slot_b"),
		"Queue a command to swap two equipped weapons."
	);

	sub.def(
		"swap_drones",
		&Input::swapDrones,
		py::arg("slot_a"),
		py::arg("slot_b"),
		"Queue a command to swap two equipped drones."
	);

	sub.def(
		"autofire",
		&Input::autofire,
		py::arg("on") = false,
		"Queue a command to toggle auto-firing.\n"
		"If not in the desired state, it tries the hotkey, then clicks the button."
	);

	sub.def(
		"teleport_send",
		&Input::teleportSend,
		"Queue a command to select the teleport send function.\n"
		"If not already selected, it tries the hotkey, then clicks the button.\n"
		"This does not activate the teleporter immediately. Use the aim function."
	);

	sub.def(
		"teleport_return",
		&Input::teleportReturn,
		"Queue a command to select the teleport return function.\n"
		"If not already selected, it tries the hotkey, then clicks the button.\n"
		"This does not activate the teleporter immediately. Use the aim function."
	);

	sub.def(
		"cloak",
		&Input::cloak,
		"Queue a command to activate cloaking.\n"
		"It tries the hotkey, then clicks the button."
	);

	sub.def(
		"mind_control",
		&Input::mindControl,
		"Queue a command to select the mind control function.\n"
		"If not already selected, it tries the hotkey, then clicks the button.\n"
		"This does not activate mind control immediately. Use the aim function."
	);

	sub.def(
		"setup_hack",
		&Input::setupHack,
		"Queue a command to begin picking a room to hack.\n"
		"If not already selected, it tries the hotkey, then clicks the button.\n"
		"This does not send the hacking drone immediately. Use the aim function."
	);

	sub.def(
		"hack",
		&Input::hack,
		"Queue a command to begin hacking.\n"
		"If not already selected, it tries the hotkey, then clicks the button."
	);

	sub.def(
		"door",
		&Input::door,
		py::arg("door"),
		py::arg("open"),
		"Queue a command to open/close a door.\n"
		"Clicks on the door, if it's not in the desired state.\n"
		"May raise an exception if the door is blocked"
	);

	sub.def(
		"door_all",
		&Input::doorAll,
		py::arg("open") = false,
		py::arg("airlocks") = false,
		"Queue a command to open/close all doors.\n"
		"Specify 'airlocks' to open airlocks too.\n"
		"Tries the hotkey, then clicks the button."
	);

	sub.def(
		"aim",
		py::overload_cast<int, bool, std::optional<bool>>(&Input::aim),
		py::arg("room"),
		py::arg("self") = false,
		py::arg("autofire") = std::nullopt,
		"Queue a command to aim a weapon/system at a room.\n\n"
		"This simply left clicks on a room, generally speaking.\n"
		"You can target your own ship by setting 'self' to True.\n\n"
		"If aiming a weapon, you can specify 'autofire' to override the overall autofire toggle.\n"
		"Otherwise, that toggle will be used to determine if the weapon autofires.\n"
		"Specifying 'autofire' when not using a weapon will throw an exception"
	);

	sub.def(
		"aim",
		py::overload_cast<int, const Point<int>&, const Point<int>&, std::optional<bool>>(&Input::aim),
		py::arg("room"),
		py::arg("start"),
		py::arg("end"),
		py::arg("autofire") = std::nullopt,
		"Queue a command to aim a beam weapon at a room.\n"
		"Aiming anything else with this overload will raise an exception.\n\n"
		"The start and end offsets are relative to the top left corner of the room.\n"
		"Note that rooms have a 1 pixel margin on all sides that you can't aim from.\n"
		"Trying to start the beam path from there will raise an exception.\n\n"
		"You can specify 'autofire' to override the overall autofire toggle.\n"
		"Otherwise, that toggle will be used to determine if the weapon autofires."
	);

	sub.def(
		"deselect",
		&Input::deselect,
		"Queue a command to deselect weapons/systems/crew.\n"
		"What it actually does is it left/right clicks in a spot that does nothing."
	);

	sub.def(
		"send_crew",
		&Input::sendCrew,
		py::arg("room"),
		py::arg("self") = true,
		"Queue a command to send the currently selected crew to a room.\n"
		"You can target the enemy ship by setting 'self' to False."
	);

	sub.def(
		"save_stations",
		&Input::saveStations,
		"Queue a command to save the stations of the player crew.\n"
		"Tries the hotkey first, then clicks the button."
	);

	sub.def(
		"load_stations",
		&Input::loadStations,
		"Queue a command to load the stations of the player crew.\n"
		"Tries the hotkey first, then clicks the button."
	);
}

}
