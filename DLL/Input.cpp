#include "Input.hpp"
#include "Utility/Memory.hpp"
#include "Utility/Exceptions.hpp"

#include <array>
#include <vector>
#include <algorithm>

extern HWND g_hGameWindow;
extern WNDPROC g_hGameWindowProc;

class Input::Impl
{
public:
	static constexpr int INPUT_CAP = 1;

	struct MouseCommand
	{
		MouseButton button;
		Point<int> position{ -1, -1 };
		bool shift = false;
		InputDirection direction = InputDirection::Unchanged;
	};

	struct KeyboardCommand
	{
		Key key;
		bool shift = false;
		InputDirection direction = InputDirection::Unchanged;
	};

	struct Command
	{
		enum class Type
		{
			Mouse, Keyboard, TextInput, TextEvent
		};

		Type type;
		double time;
		uintmax_t id = uintmax_t(-1);

		union
		{
			MouseCommand mouse;
			KeyboardCommand keyboard;
			char textInput;
			raw::TextEvent textEvent;
		};

		// Commands with higher times are sorted first
		// That way the ones that come sooner are popped off a max heap first
		bool operator<(const Command& other)
		{
			return this->time > other.time;
		}
	};

	Impl()
	{
		auto&& mrs = Reader::getRawState(MutableRawState{});
	}

	~Impl() = default;

	void iterate()
	{
		int inputsMade = 0;

		while (inputsMade < INPUT_CAP && !this->empty())
		{
			auto lowerBound = Reader::now() - 500.0;
			auto upperBound = Reader::now();

			if (this->top().time > upperBound) break;

			// stale/old inputs get ignored
			//if (this->top().time >= lowerBound)
			{
				auto&& cmd = this->top();

				switch (cmd.type)
				{
				case Command::Type::Mouse:
					this->mouseInput(cmd.mouse);
					break;
				case Command::Type::Keyboard:
					this->keyboardInput(cmd.keyboard);
					break;
				case Command::Type::TextInput:
					this->textInput(cmd.textInput);
					break;
				case Command::Type::TextEvent:
					this->textEvent(cmd.textEvent);
					break;
				}

				inputsMade++;
			}

			this->pop();
		}
	}

	Input::Ret push(const Command& cmd)
	{
		this->queue.push_back(cmd);
		this->queue.back().id = ++this->idCounter;
		std::push_heap(this->queue.begin(), this->queue.end());
		return { this->idCounter, cmd.time };
	}

	bool empty() const
	{
		return this->queue.empty();
	}

	bool emptyWithin(double time) const
	{
		double now = Reader::now();

		return 
			this->queue.end() == std::find_if(
				this->queue.begin(), this->queue.end(),
				[&](auto& cmd) {
					return cmd.time-now <= time;
				}
			);
	}

	bool queued(uintmax_t id) const
	{
		return this->search(id) != this->queue.end();
	}

	bool remove(uintmax_t id)
	{
		auto it = this->search(id);

		// Not in queue
		if (it == this->queue.end())
			return false;

		this->queue.erase(it);
		std::make_heap(this->queue.begin(), this->queue.end());

		return true;
	}

	void clear()
	{
		this->queue.clear();
	}

	void unhookAll()
	{
		this->shiftStateHook.unhook();
	}

	static bool offScreen(const Point<int>& p)
	{
		return
			p.x < 0 || p.x >= Input::GAME_WIDTH ||
			p.y < 0 || p.y >= Input::GAME_HEIGHT;
	}

	void cheat(const std::string& cmd)
	{
		static constexpr uintptr_t CONSOLE_ADDR = 0x0011B4A0;
		auto* gui = Reader::getRawState({}).app->gui;
		uintptr_t funcAddr = Reader::getRealAddress(CONSOLE_ADDR, {});
		auto* func = reinterpret_cast<void(__thiscall*)(raw::CommandGui*, raw::gcc::string&)>(funcAddr);

		// The one time we will ever have to do this conversion
		raw::gcc::string str;
		str.len = cmd.size();

		if (str.len <= raw::gcc::string::SMALL_STRING_OPTIMIZATION_LEN)
		{
			// small string optimization, copy it like this
			strcpy_s(str.local_data, cmd.data());
			str.local_data[raw::gcc::string::SMALL_STRING_OPTIMIZATION_LEN] = '\0';
			str.str = str.local_data;
			func(gui, str);
		}
		else
		{
			// not using const_cast just to be safe
			std::string copy = cmd;
			str.str = copy.data();
			func(gui, str);
		}
	}

private:
	friend class Input;
	using Queue = std::vector<Command>;

	static constexpr uintptr_t SHIFT_STATE_ADDR = 0x178BE0;

	// Hookable "function" that simply returns 1
	static constexpr std::array<BYTE, 10> RET_1{

		0x55, // push ebp
		0x89, 0xE5, // mov ebp,esp
		0xB8, 0x01, 0x00, 0x00, 0x00, // mov eax,1
		0xC9, // leave
		0xC3  // ret
	};

	// We hook into the functions that grab if these keys are down
	// and replace them with the above if we want to force them down
	mem::Hook shiftStateHook;

	// Search for input with this id
	Queue::iterator search(uintmax_t id)
	{
		return std::find_if(
			this->queue.begin(), this->queue.end(),
			[&id](auto&& cmd) {
				return cmd.id == id;
			}
		);
	}

	Queue::const_iterator search(uintmax_t id) const
	{
		return std::find_if(
			this->queue.cbegin(), this->queue.cend(),
			[&id](auto&& cmd) {
				return cmd.id == id;
			}
		);
	}

	// Forces shift to be held, regardless of player input
	void setShiftHeld(bool held)
	{
		auto&& mrs = Reader::getRawState({});

		if (held && !this->shiftStateHook.hooked())
		{
			auto addr = (BYTE*)Reader::getRealAddress(SHIFT_STATE_ADDR, {});
			this->shiftStateHook.hook(addr, RET_1.data(), RET_1.size(), true);
			mrs.app->shift_held = true;
		}
		else if(!held && this->shiftStateHook.hooked())
		{
			this->shiftStateHook.unhook();
			mrs.app->shift_held = false;
		}
	}

	void mouseInput(const MouseCommand& mouse)
	{
		auto&& mrs = Reader::getRawState({});
		Point<int> pos = mouse.position;
		Point<int> old = Reader::getState().ui.mouse.position;

		// Position is invalid (intentionally or not), don't move
		if (this->offScreen(pos))
		{
			pos = old;
		}

		// not clicking
		if (mouse.direction == InputDirection::Unchanged || mouse.button == MouseButton::None)
		{
			Point<int> offset = pos - old;
			mrs.app->OnMouseMove(pos.x, pos.y, offset.x, offset.y, false, false, false);
			mrs.mouseControl->lastPosition = mrs.mouseControl->position;
			mrs.mouseControl->position.x = pos.x;
			mrs.mouseControl->position.y = pos.y;

			return; 
		}

		this->setShiftHeld(mouse.shift);

		if (mouse.direction == InputDirection::Down)
		{
			switch (mouse.button)
			{
			case MouseButton::Left: mrs.app->OnLButtonDown(pos.x, pos.y); break;
			case MouseButton::Middle: mrs.app->OnMButtonDown(pos.x, pos.y); break;
			case MouseButton::Right: mrs.app->OnRButtonDown(pos.x, pos.y); break;
			}
		}
		else if (mouse.direction == InputDirection::Up)
		{
			switch (mouse.button)
			{
			case MouseButton::Left: mrs.app->OnLButtonUp(pos.x, pos.y); break;
			case MouseButton::Middle: mrs.app->OnMButtonUp(pos.x, pos.y); break;
			case MouseButton::Right: mrs.app->OnRButtonUp(pos.x, pos.y); break;
			}
		}

		this->setShiftHeld(false);
	}

	void keyboardInput(const KeyboardCommand& keyboard)
	{
		auto&& mrs = Reader::getRawState({});

		if (keyboard.direction == InputDirection::Unchanged)
		{
			return; // not changing
		}

		bool keyIsShift =
			keyboard.key == Key::LShift ||
			keyboard.key == Key::RShift;

		// Ignore shift parameter for shift key
		if(!keyIsShift) this->setShiftHeld(keyboard.shift);

		if (keyboard.direction == InputDirection::Down)
		{
			if (keyIsShift) this->setShiftHeld(true);
			else mrs.app->OnKeyDown(int(keyboard.key));
		}
		else if (keyboard.direction == InputDirection::Up)
		{
			if (keyIsShift) this->setShiftHeld(false);
			else mrs.app->OnKeyUp(int(keyboard.key));
		}

		// Ignore shift parameter for shift key
		if (!keyIsShift) this->setShiftHeld(false);
	}

	void textInput(char character)
	{
		auto&& mrs = Reader::getRawState({});
		mrs.app->OnTextInput(character);
	}

	void textEvent(raw::TextEvent event)
	{
		auto&& mrs = Reader::getRawState({});
		mrs.app->OnTextEvent(event);
	}

	Command& top()
	{
		return this->queue.front();
	}

	void pop()
	{
		std::pop_heap(this->queue.begin(), this->queue.end());
		this->queue.pop_back();
	}

	Queue queue;

	uintmax_t idCounter = 0;
};

Input::Impl Input::impl;
bool Input::good = false;
bool Input::humanMouse = true;
bool Input::humanKeyboard = true;

namespace
{

bool validateInGame(const State& s, bool nothrow = false)
{
	if (!s.game || !s.game->playerShip)
	{
		if (!nothrow) throw GameNotRunning();
		else return false;
	}

	return true;
}

void validateDelay(double d)
{
	if (d < 0.0)
	{
		throw InvalidTime("we can't send commands to the past to undo our mistakes");
	}

	if (d > 8640.0)
	{
		throw InvalidTime("this would send a command over a day from now; are you marathoning?");
	}
}

Key getHotkey(
	const decltype(Settings::hotkeys)& map,
	const std::string& k)
{
	try
	{
		Key key = map.at(k);

		return key;
	}
	catch (const std::out_of_range&)
	{
		throw InvalidHotkey(k);
	}
}

struct PowerHotkey
{
	Key key = Key::Unknown;
	bool shift = false;
};

// Find any working keyboard combo to (un)power the system
PowerHotkey workingPowerHotkey(
	const System& sys,
	const decltype(Settings::hotkeys)& hotkeys,
	bool unpower)
{
	Key k;

	if (unpower)
	{
		// Try to get the direct unpower key
		k = getHotkey(hotkeys, systemUnpowerHotkey(sys.type));
		if (k != Key::Unknown) return { k, false };

		// Then, fallback to direct power key + shift
		k = getHotkey(hotkeys, systemPowerHotkey(sys.type));
		if (k != Key::Unknown) return { k, true };

		// Then, fallback to positional unpower key
		int id = sys.uiBox + 1;
		k = getHotkey(hotkeys, "un_power_" + std::to_string(id));
		if (k != Key::Unknown) return { k, false };

		// Then, fallback to positional power key + shift
		k = getHotkey(hotkeys, "power_" + std::to_string(id));
		if (k != Key::Unknown) return { k, true };

		// Otherwise, there's no hotkey for it
		return { k, false };
	}

	// Try to get direct power key
	k = getHotkey(hotkeys, systemPowerHotkey(sys.type));
	if (k != Key::Unknown) return { k, false };

	// Then, fallback to positional power 
	int id = sys.uiBox + 1;
	k = getHotkey(hotkeys, "power_" + std::to_string(id));
	if (k != Key::Unknown) return { k, false };

	// Otherwise, there's no hotkey for it
	return { k, false };
}

// Some arbitrary spot that should have no UI elements
Point<int> nopSpot()
{
	return { Input::GAME_WIDTH - 1, Input::GAME_HEIGHT - 1 };
}

// Assumes a bunch of other checks have already been made
Input::Ret useWeapon(
	const Weapon& weapon,
	const std::map<std::string, Key>& hotkeys,
	bool powerOff = false,
	double delay = 0.0)
{
	// Try to use a hotkey
	auto hotkey = getHotkey(hotkeys, "weapon" + std::to_string(weapon.slot + 1));

	// If there is none, use the mouse
	if (hotkey == Key::Unknown)
	{
		auto&& weaponBoxes = Reader::getState().ui.game->weaponBoxes;
		auto pos = weaponBoxes[weapon.slot].center();
		auto button = powerOff ? MouseButton::Right : MouseButton::Left;

		if (powerOff)
		{
			return Input::mouseClick(button, pos, false, delay);
		}

		return Input::mouseUp(button, pos, false, delay);
	}

	return Input::keyPress(hotkey, powerOff, delay);
}

const Weapon* weaponAt(int which, bool suppress = false)
{
	auto&& opt = Reader::getState().game->playerShip->weapons;

	if (!opt)
	{
		if (!suppress) throw SystemNotInstalled(SystemType::Weapons);
		else return nullptr;
	}

	auto&& weapons = *opt;
	auto size = int(weapons.list.size());

	if (which < 0 || which >= size)
	{
		if (!suppress) throw InvalidSlotChoice("weapon", which, size);
		else return nullptr;
	}

	return &weapons.list.at(which);
}

// Assumes a bunch of other checks have already been made
Input::Ret useDrone(
	const Drone& drone,
	const std::map<std::string, Key>& hotkeys,
	bool powerOff = false,
	double delay = 0.0)
{
	// Try to use a hotkey
	auto hotkey = getHotkey(hotkeys, "drone" + std::to_string(drone.slot + 1));

	// If there is none, use the mouse
	if (hotkey == Key::Unknown)
	{
		auto&& droneBoxes = Reader::getState().ui.game->droneBoxes;
		auto pos = droneBoxes[drone.slot].center();
		auto button = powerOff ? MouseButton::Right : MouseButton::Left;

		if (powerOff)
		{
			return Input::mouseClick(button, pos, false, delay);
		}

		return Input::mouseUp(button, pos, false, delay);
	}

	return Input::keyPress(hotkey, powerOff, delay);
}

const Drone* droneAt(int which, bool suppress = false)
{
	auto&& opt = Reader::getState().game->playerShip->drones;

	if (!opt)
	{
		if (!suppress) throw SystemNotInstalled(SystemType::Drones);
		else return nullptr;
	}

	auto&& drones = *opt;
	auto size = int(drones.list.size());

	if (which < 0 || which >= size)
	{
		if (!suppress) throw InvalidSlotChoice("drone", which, size);
		else return nullptr;
	}

	return &drones.list.at(which);
}

void validateCrew(const Crew& crew, const std::vector<Crew>& list)
{
	if (crew.dead)
	{
		throw InvalidCrewChoice(crew, "they're dead");
	}

	if (crew.drone)
	{
		throw InvalidCrewChoice(crew, "they're a drone");
	}

	if (!crew.player)
	{
		throw InvalidCrewChoice(crew, "they're an enemy");
	}

	int count = int(list.size());

	if (crew.uiBox < 0)
	{
		throw InvalidCrewBoxChoice(crew.uiBox);
	}

	if (crew.uiBox > count)
	{
		throw InvalidCrewBoxChoice(crew.uiBox, count);
	}
}

Key crewHotkey(int which, const std::map<std::string, Key>& hotkeys)
{
	return getHotkey(hotkeys, "crew" + std::to_string(which + 1));
}

// Assumes a bunch of other checks have already been made
Input::Ret useCrewSingle(
	const Crew& crew,
	const std::map<std::string, Key>& hotkeys,
	bool exclusive = true,
	double delay = 0.0)
{
	// Try to use a hotkey
	auto hotkey = crewHotkey(crew.uiBox, hotkeys);

	if (hotkey == Key::Unknown)
	{
		// If there is none, use the mouse
		auto&& crewBoxes = Reader::getState().ui.game->crewBoxes;
		auto pos = crewBoxes[crew.uiBox].box.center();

		return Input::mouseClick(MouseButton::Left, pos, !exclusive, delay);
	}

	return Input::keyPress(hotkey, !exclusive, delay);
}

// Assumes a bunch of other checks have already been made
// Assumes crew list is sorted by ui box order already
// Always uses mouse
Input::Ret useCrewMany(
	const Input::CrewRefList& crew,
	bool exclusive = true,
	double delay = 0.0)
{
	auto&& crewBoxes = Reader::getState().ui.game->crewBoxes;

	Point<int> first = crewBoxes[crew.front().get().uiBox].box.center();
	Point<int> last = crewBoxes[crew.back().get().uiBox].box.center();

	Input::mouseDown(MouseButton::Left, first, !exclusive, delay);
	return Input::mouseUp(MouseButton::Left, last, !exclusive, delay);
}

// Generic function to try a hotkey and fallback to clicking
Input::Ret hotkeyOr(
	const char* what,
	const char* hotkey,
	const std::map<std::string, Key>& hotkeys,
	Point<int> fallback,
	bool suppress,
	double delay)
{
	auto&& state = Reader::getState();
	if (!validateInGame(state, suppress)) return {};

	if (state.game->pause.menu)
	{
		if (!suppress) throw WrongMenu(what);
		else return {};
	}

	// Use if hotkey possible
	auto k = getHotkey(hotkeys, hotkey);

	if (k != Key::Unknown)
	{
		return Input::keyPress(k, false, delay);
	}

	return Input::mouseClick(MouseButton::Left, fallback, false, delay);

}

}

void Input::iterate()
{
	impl.iterate();
}

bool Input::empty()
{
	return impl.empty();
}

bool Input::emptyWithin(double time)
{
	return impl.emptyWithin(time);
}

bool Input::queued(uintmax_t id)
{
	return impl.queued(id);
}

bool Input::pop(uintmax_t id)
{
	return impl.remove(id);
}

bool Input::ready()
{
	return good;
}

void Input::setReady(bool ready)
{
	good = ready;

	if (!ready)
	{
		impl.unhookAll();
	}
}

void Input::allowHumanMouse(bool allow)
{
	humanMouse = allow;
}

void Input::allowHumanKeyboard(bool allow)
{
	humanKeyboard = allow;
}

bool Input::humanMouseAllowed()
{
	return humanMouse;
}

bool Input::humanKeyboardAllowed()
{
	return humanKeyboard;
}

void Input::clear()
{
	impl.clear();
}

Input::Ret Input::mouseMove(
	const Point<int>& position,
	double delay)
{
	validateDelay(delay);
	return impl.push(Impl::Command{
		.type = Impl::Command::Type::Mouse,
		.time = Reader::now() + delay,
		.mouse = {
			.position = position
		},
	});
}

Input::Ret Input::mouseDown(
	MouseButton button,
	const Point<int>& position,
	bool shift,
	double delay)
{
	validateDelay(delay);
	mouseMove(position, delay);
	return impl.push(Impl::Command{
		.type = Impl::Command::Type::Mouse,
		.time = Reader::now() + delay,
		.mouse = {
			.button = button,
			.position = position,
			.shift = shift,
			.direction = InputDirection::Down
		}
	});
}

Input::Ret Input::mouseUp(
	MouseButton button,
	const Point<int>& position,
	bool shift,
	double delay)
{
	validateDelay(delay);
	mouseMove(position, delay);
	return impl.push(Impl::Command{
		.type = Impl::Command::Type::Mouse,
		.time = Reader::now() + delay,
		.mouse = {
			.button = button,
			.position = position,
			.shift = shift,
			.direction = InputDirection::Up
		}
	});
}

Input::Ret Input::mouseClick(
	MouseButton button,
	const Point<int>& position,
	bool shift,
	double delay)
{
	mouseMove(position, delay);

	impl.push(Impl::Command{
		.type = Impl::Command::Type::Mouse,
		.time = Reader::now() + delay,
		.mouse = {
			.button = button,
			.position = position,
			.shift = shift,
			.direction = InputDirection::Down
		}
	});

	return impl.push(Impl::Command{
		.type = Impl::Command::Type::Mouse,
		.time = Reader::now() + delay,
		.mouse = {
			.button = button,
			.position = position,
			.shift = shift,
			.direction = InputDirection::Up
		}
	});
}

Input::Ret Input::keyDown(
	Key key,
	bool shift,
	double delay)
{
	validateDelay(delay);
	return impl.push(Impl::Command{
		.type = Impl::Command::Type::Keyboard,
		.time = Reader::now() + delay,
		.keyboard = {
			.key = key,
			.shift = shift,
			.direction = InputDirection::Down
		}
	});
}

Input::Ret Input::keyUp(
	Key key,
	bool shift,
	double delay)
{
	validateDelay(delay);
	return impl.push(Impl::Command{
		.type = Impl::Command::Type::Keyboard,
		.time = Reader::now() + delay,
		.keyboard = {
			.key = key,
			.shift = shift,
			.direction = InputDirection::Up
		}
	});
}

Input::Ret Input::keyPress(
	Key key,
	bool shift,
	double delay)
{
	keyDown(key, shift, delay);
	return keyUp(key, false, Reader::now() + delay);
}

Input::Ret Input::hotkeyDown(
	const std::string& hotkey,
	bool shift,
	double delay)
{
	Key key = getHotkey(hotkeys(), hotkey);
	return keyDown(key, shift, delay);
}

Input::Ret Input::hotkeyUp(
	const std::string& hotkey,
	bool shift,
	double delay)
{
	Key key = getHotkey(hotkeys(), hotkey);
	return keyUp(key, shift, delay);
}

Input::Ret Input::hotkeyPress(
	const std::string& hotkey,
	bool shift,
	double delay)
{
	Key key = getHotkey(hotkeys(), hotkey);
	return keyPress(key, shift, delay);
}

const decltype(Settings::hotkeys)& Input::hotkeys()
{
	return Reader::getState().settings.hotkeys;
}

Input::Ret Input::text(char ch, double delay)
{
	validateDelay(delay);
	return impl.push(Impl::Command{
		.type = Impl::Command::Type::TextInput,
		.time = Reader::now() + delay,
		.textInput = ch
	});
}

Input::Ret Input::text(const std::string& str, double delay)
{
	Input::Ret last;
	for (auto&& c : str) last = text(c, delay);
	return last;
}

Input::Ret Input::textConfirm(double delay)
{
	validateDelay(delay);
	return impl.push(Impl::Command{
		.type = Impl::Command::Type::TextEvent,
		.time = Reader::now() + delay,
		.textEvent = raw::TEXT_CONFIRM
	});
}

Input::Ret Input::textClear(double delay)
{
	validateDelay(delay);
	return impl.push(Impl::Command{
		.type = Impl::Command::Type::TextEvent,
		.time = Reader::now() + delay,
		.textEvent = raw::TEXT_CLEAR
	});
}

Input::Ret Input::textBackspace(double delay)
{
	validateDelay(delay);
	return impl.push(Impl::Command{
		.type = Impl::Command::Type::TextEvent,
		.time = Reader::now() + delay,
		.textEvent = raw::TEXT_BACKSPACE
	});
}

Input::Ret Input::textDelete(double delay)
{
	validateDelay(delay);
	return impl.push(Impl::Command{
		.type = Impl::Command::Type::TextEvent,
		.time = Reader::now() + delay,
		.textEvent = raw::TEXT_DELETE
	});
}

Input::Ret Input::textLeft(double delay)
{
	validateDelay(delay);
	return impl.push(Impl::Command{
		.type = Impl::Command::Type::TextEvent,
		.time = Reader::now() + delay,
		.textEvent = raw::TEXT_LEFT
	});
}

Input::Ret Input::textRight(double delay)
{
	validateDelay(delay);
	return impl.push(Impl::Command{
		.type = Impl::Command::Type::TextEvent,
		.time = Reader::now() + delay,
		.textEvent = raw::TEXT_RIGHT
	});
}

Input::Ret Input::textHome(double delay)
{
	validateDelay(delay);
	return impl.push(Impl::Command{
		.type = Impl::Command::Type::TextEvent,
		.time = Reader::now() + delay,
		.textEvent = raw::TEXT_HOME
	});
}

Input::Ret Input::textEnd(double delay)
{
	validateDelay(delay);
	return impl.push(Impl::Command{
		.type = Impl::Command::Type::TextEvent,
		.time = Reader::now() + delay,
		.textEvent = raw::TEXT_END
	});
}

void Input::cheat(const std::string& command, bool suppress)
{
	auto&& state = Reader::getState();
	if (!validateInGame(state, suppress)) return;

	impl.cheat(command);
}

Input::Ret Input::pause(
	bool on,
	bool suppress,
	double delay)
{
	auto&& state = Reader::getState();
	if (!validateInGame(state, suppress)) return {};
	auto&& pause = state.game->pause;
	
	if (pause.menu && !pause.event)
	{
		if (!suppress) throw WrongMenu("pausing");
		else return {};
	}

	// nothing to do
	if (pause.any == on) return {};

	// middle mouse is always available, no need to check for hotkey
	return mouseClick(MouseButton::Left, { -1, -1 }, false, delay);
}

Input::Ret Input::eventChoice(
	int which,
	bool suppress,
	double delay)
{
	auto&& state = Reader::getState();
	if (!validateInGame(state, suppress)) return {};

	auto&& game = state.game;

	if(!game->event || !game->pause.event)
	{
		if (!suppress) throw NoEvent();
		else return {};
	}

	auto&& event = state.ui.game->event;
	auto&& choices = event->choices;
		
	if (size_t(which) >= choices.size())
	{
		if (!suppress) throw InvalidEventChoice(which, int(choices.size()));
		else return {};
	}

	// Hotkeys disabled, must click...
	if (state.settings.eventChoiceSelection == EventChoiceSelection::DisableHotkeys)
	{
		auto&& choice = choices.at(which);
		bool topCenter = !impl.offScreen(choice.box.topCenter());

		if (!topCenter)
		{
			if (!suppress) throw InvalidEventChoice(which, "hotkeys are disabled and the choice is off-screen");
			else return {};
		}

		bool center = !impl.offScreen(choice.box.center());

		return mouseClick(
			MouseButton::Left,
			center ? choice.box.center() : choice.box.topCenter(),
			false, delay);
	}

	// Make sure delay is long enough so input doesn't get ate
	double diff = event->openTime.second - event->openTime.first;
	if (delay < diff) delay = diff;

	return keyPress(Key(int(Key::Num1) + which), false, delay);
}

// Setting the system power is quite complicated and has many edge cases
// Mostly due to shields and zoltan power...
// So yeah, I'll be real glad this helper function exists if it works
Input::Ret Input::systemPower(
	const System& system,
	int set, int delta,
	bool suppress,
	double delay)
{
	auto&& state = Reader::getState();
	if (!validateInGame(state, suppress)) return {};

	if (state.game->pause.menu)
	{
		if (!suppress) throw WrongMenu("powering a system");
		else return {};
	}

	if (delta != 0 && set > 0)
	{
		if (!suppress) throw InvalidPowerRequest(system, "delta and set cannot both be non-zero");
		else return {};
	}

	if (!system.player)
	{
		if (!suppress) throw InvalidPowerRequest(system, "it does not belong to player");
		else return {};
	}

	int required = system.power.required;

	if (required <= 0 || required > 2)
	{
		if (!suppress) throw InvalidPowerRequest(system, "this function can't handle that system");
		else return {};
	}

	if (system.subsystem())
	{
		if (!suppress) throw InvalidPowerRequest(system, "subsystem power cannot be controlled");
		else return {};
	}

	int current = system.power.total.first;
	int desired = delta != 0 ? current + delta : set;

	// Nothing to do
	if (current == desired) return {};

	if (delta == 0) delta = desired - current;

	auto range = system.powerRange();
	if (!suppress && (desired < range.first || desired > range.second))
	{
		throw InvalidPowerRequest(system, desired, range);
	}

	int mod = desired % required;
	int zoltan = system.power.zoltan;

	// Power must be between a multiple and a multiple + zoltan power present
	if (!suppress && mod > zoltan)
	{
		throw InvalidPowerRequest(system, desired, required);
	}

	int upInputs = 0;
	int downInputs = 0;

	if (desired < current)
	{
		downInputs = -delta;

		if (!zoltan)
		{
			// assuming we're already set to a multiple
			downInputs /= system.power.required;
		}
	}
	else
	{
		int start = required > 1 ? (current / required) * required : current;
		int end = required > 1 ? ((desired + 1) / required) * required : desired;
		upInputs = (end - start) / required;
		if (mod != 0) downInputs = 1;

		if (end - current > state.game->playerShip->reactor.total.first)
		{
			if (!suppress) throw InvalidPowerRequest(system, "there isn't enough power in the reactor");
			else downInputs = 0; // just input upwards for the "not enough power" message
		}
	}

	Ret last{ 0, Reader::now() + delay };

	auto doInput = [&](const PowerHotkey& k, bool depower) {
		auto&& [key, shift] = k;

		if (key != Key::Unknown) // hotkey found
		{
			last = keyDown(key, shift, delay);
		}
		else // must click instead
		{
			auto&& ship = *state.game->playerShip;
			auto&& point = state.ui.game->getSystem(system.type, system.discriminator).bottomCenter();
			last = mouseClick(
				depower ? MouseButton::Right : MouseButton::Left,
				point, false, delay
			);
		}
	};

	if (upInputs > 0)
	{
		auto k = workingPowerHotkey(system, hotkeys(), false);
		for (int i = 0; i < upInputs; i++) doInput(k, false);
	}

	if (downInputs > 0)
	{
		auto k = workingPowerHotkey(system, hotkeys(), true);
		for (int i = 0; i < downInputs; i++) doInput(k, true);
	}

	return last;
}

Input::Ret Input::systemPower(
	SystemType which,
	int set, int delta,
	bool suppress,
	double delay)
{
	auto&& state = Reader::getState();
	if (!validateInGame(state, suppress)) return {};

	try
	{
		auto&& system = state.game->playerShip->getSystem(which, suppress);
		return systemPower(system, set, delta, suppress, delay);
	}
	catch (const std::exception& e)
	{
		if (!suppress) throw e;
	}

	return {};
}

Input::Ret Input::weaponPower(
	const Weapon& weapon,
	bool on,
	bool suppress,
	double delay)
{
	auto&& state = Reader::getState();
	if (!validateInGame(state, suppress)) return {};

	if (state.game->pause.menu)
	{
		if (!suppress) throw WrongMenu("powering a weapon");
		else return {};
	}

	if (!weapon.player)
	{
		if (!suppress) throw InvalidPowerRequest(weapon, "it does not belong to the player");
		else return {};
	}

	if (!state.game->playerShip->weapons)
	{
		if (!suppress) throw SystemNotInstalled(SystemType::Weapons);
		else return {};
	}

	auto&& weapSys = *state.game->playerShip->weapons;
	auto slots = int(weapSys.list.size());

	if (weapon.slot < 0)
	{
		if (!suppress) throw InvalidSlotChoice("weapon", weapon.slot);
		else return {};
	}

	if (weapon.slot >= slots)
	{
		if (!suppress) throw InvalidSlotChoice("weapon", weapon.slot, slots);
		else return {};
	}

	auto range = weapSys.powerRange();
	int current = weapon.power.total.first;
	if (range.first == range.second)
	{
		int desired = current + weapon.power.required * (on ? 1 : -1);
		if (!suppress) throw InvalidPowerRequest(weapSys, desired, range);
	}

	if (on)
	{
		int needs = weapon.power.required - current;

		// Nothing needs to be done
		if (needs == 0) return {};

		// Need missiles
		int missiles = state.game->playerShip->cargo.missiles;
		int neededMissiles = weapon.blueprint.missiles;
		if (!suppress && missiles < neededMissiles)
		{
			throw InvalidSlotChoice("weapon", weapon.slot, "there isn't enough missiles");
		}

		if (!suppress && needs > state.game->playerShip->reactor.total.first)
		{
			throw InvalidPowerRequest(weapon, "there isn't enough power in the reactor");
		}

		int remaining = weapSys.power.total.second - weapSys.power.total.first;
		if (!suppress && needs > remaining)
		{
			throw InvalidPowerRequest(weapon, "there isn't enough power in the weapon system");
		}
	}
	else
	{
		int del = current - weapon.power.zoltan;

		// Nothing needs to be done
		if (del == 0) return {};

		if (!suppress && weapon.power.zoltan >= weapon.power.required)
		{
			throw InvalidPowerRequest(weapon, "it is fully powered by zoltan crew");
		}
	}

	return useWeapon(weapon, hotkeys(), !on, delay);
}

Input::Ret Input::weaponPower(
	int which,
	bool on,
	bool suppress,
	double delay)
{
	auto&& state = Reader::getState();
	if (!validateInGame(state, suppress)) return {};
	auto&& weapon = weaponAt(which, suppress);
	if (!weapon) return {}; // only happens if suppressed
	return weaponPower(*weapon, on, suppress, delay);
}

Input::Ret Input::weaponSelect(
	const Weapon& weapon,
	bool suppress,
	double delay)
{
	auto&& state = Reader::getState();
	if (!validateInGame(state, suppress)) return {};

	if (state.game->pause.menu)
	{
		if (!suppress) throw WrongMenu("selecting a weapon");
		else return {};
	}

	if (!weapon.player)
	{
		if (!suppress) throw InvalidSlotChoice("weapon", weapon.slot, "it does not belong to the player");
		else return {};
	}

	if (!weapon.powered()) weaponPower(weapon, true, suppress, delay);
	return useWeapon(weapon, hotkeys(), false, delay);
}

Input::Ret Input::weaponSelect(
	int which,
	bool suppress,
	double delay)
{
	auto&& state = Reader::getState();
	if (!validateInGame(state, suppress)) return {};
	auto&& weapon = weaponAt(which, suppress);
	if (!weapon) return {}; // only happens if suppressed
	return weaponSelect(*weapon, suppress, delay);
}

Input::Ret Input::dronePower(
	const Drone& drone,
	bool on,
	bool suppress,
	double delay)
{
	auto&& state = Reader::getState();
	if (!validateInGame(state, suppress)) return {};

	if (state.game->pause.menu)
	{
		if (!suppress) throw WrongMenu("powering a drone");
		else return {};
	}

	if (!drone.player)
	{
		if (!suppress) throw InvalidPowerRequest(drone, "it does not belong to the player");
		else return {};
	}

	if (!state.game->playerShip->drones)
	{
		if (!suppress) throw SystemNotInstalled(SystemType::Drones);
		else return {};
	}

	auto&& droneSys = *state.game->playerShip->drones;
	auto slots = int(droneSys.list.size());

	if (drone.slot < 0)
	{
		if (!suppress) throw InvalidSlotChoice("drone", drone.slot);
		else return {};
	}

	if (drone.slot >= slots)
	{
		if (!suppress) throw InvalidSlotChoice("drone", drone.slot, slots);
		else return {};
	}

	auto range = droneSys.powerRange();
	int current = drone.power.total.first;
	if (range.first == range.second)
	{
		int desired = current + drone.power.required * (on ? 1 : -1);
		if (!suppress) throw InvalidPowerRequest(droneSys, desired, range);
	}

	if (on)
	{
		int needs = drone.power.required - current;

		// Nothing needs to be done
		if (needs == 0) return {};

		// Need drone parts
		int droneParts = state.game->playerShip->cargo.droneParts;
		if (!suppress && droneParts < 1)
		{
			throw InvalidSlotChoice("drone", drone.slot, "there isn't enough drone parts");
		}

		// Need to not be destroyed
		if (!suppress && drone.dead)
		{
			throw InvalidSlotChoice("drone", drone.slot, "it is destroyed or on cooldown");
		}

		// Various drone types need an enemy ship
		bool enemy = state.game->enemyShip.has_value();
		if (!suppress && drone.needsEnemy() && !enemy)
		{
			throw InvalidSlotChoice("drone", drone.slot, "it needs an enemy to function");
		}

		if (!suppress && needs > state.game->playerShip->reactor.total.first)
		{
			throw InvalidPowerRequest(drone, "there isn't enough power in the reactor");
		}

		int remaining = droneSys.power.total.second - droneSys.power.total.first;
		if (!suppress && needs > remaining)
		{
			throw InvalidPowerRequest(drone, "there isn't enough power in the drone system");
		}
	}
	else
	{
		int del = current - drone.power.zoltan;

		// Nothing needs to be done
		if (del == 0) return {};

		if (!suppress && drone.power.zoltan >= drone.power.required)
		{
			throw InvalidPowerRequest(drone, "it is fully powered by zoltan crew");
		}
	}

	return useDrone(drone, hotkeys(), !on, delay);
}


Input::Ret Input::dronePower(
	int which,
	bool on,
	bool suppress,
	double delay)
{
	auto&& state = Reader::getState();
	if (!validateInGame(state, suppress)) return {};
	auto&& drone = droneAt(which, suppress);
	if (!drone) return {}; // only happens if suppressed
	return dronePower(*drone, on, suppress, delay);
}

Input::Ret Input::autofire(bool on, bool suppress, double delay)
{
	auto&& state = Reader::getState();
	if (!validateInGame(state, suppress)) return {};

	if (!state.game->playerShip->weapons)
	{
		if (!suppress) throw SystemNotInstalled(SystemType::Weapons);
		else return {};
	}

	if (state.game->playerShip->weapons->autoFire == on)
	{
		// Nothing to do
		return {};
	}

	return hotkeyOr(
		"toggling auto-fire", "autofire", hotkeys(),
		state.ui.game->autofire->center(), suppress, delay
	);
}

Input::Ret Input::openAllDoors(bool airlocks, bool suppress, double delay)
{
	auto&& state = Reader::getState();
	if (!validateInGame(state, suppress)) return {};

	if (!state.game->playerShip->doorControl)
	{
		if (!suppress) throw SystemNotInstalled(SystemType::Doors);
		else return {};
	}

	bool onlyAirlocksClosed = true;

	for (auto&& door : state.game->playerShip->doors)
	{
		if (!door.airlock && !door.open)
		{
			onlyAirlocksClosed = false;
			break;
		}
	}

	if (!airlocks && onlyAirlocksClosed)
	{
		// Nothing to do
		return {};
	}

	auto l = [&] {
		return hotkeyOr(
			"opening all doors", "open", hotkeys(),
			state.ui.game->openAllDoors->center(), suppress, delay
		);
	};

	if (airlocks && !onlyAirlocksClosed)
	{
		// Tap the button again
		l();
	}

	return l();
}

Input::Ret Input::closeAllDoors(bool suppress, double delay)
{
	auto&& state = Reader::getState();
	if (!validateInGame(state, suppress)) return {};

	if (!state.game->playerShip->doorControl)
	{
		if (!suppress) throw SystemNotInstalled(SystemType::Doors);
		else return {};
	}

	bool allClosed = true;

	for (auto&& door : state.game->playerShip->doors)
	{
		if (door.open)
		{
			allClosed = false;
			break;
		}
	}

	if (allClosed)
	{
		// Nothing to do
		return {};
	}

	return hotkeyOr(
		"closing all doors", "close", hotkeys(),
		state.ui.game->closeAllDoors->center(), suppress, delay
	);
}

Input::Ret Input::aimCancel(bool suppress, double delay)
{
	auto&& state = Reader::getState();
	validateInGame(state, suppress);

	if (state.game->pause.menu)
	{
		if (!suppress) throw WrongMenu("cancelling aiming");
		else return {};
	}

	return mouseClick(MouseButton::Right, nopSpot(), false, delay);
}

Input::Ret Input::crewSelect(
	const Crew& crew,
	bool exclusive,
	bool suppress,
	double delay)
{
	auto&& state = Reader::getState();
	if (!validateInGame(state, suppress)) return {};

	if (state.game->pause.menu)
	{
		if (!suppress) throw WrongMenu("selecting crew");
		else return {};
	}

	try
	{
		validateCrew(crew, state.game->playerCrew);
	}
	catch (const std::exception& e)
	{
		if (!suppress) throw e;
	}

	return useCrewSingle(crew, hotkeys(), exclusive, delay);
}

Input::Ret Input::crewSelect(
	const Input::CrewRefList& crew,
	bool exclusive,
	bool suppress,
	double delay)
{

	auto&& state = Reader::getState();
	if (!validateInGame(state, suppress)) return {};

	if (state.game->pause.menu)
	{
		if (!suppress) throw WrongMenu("selecting crew");
		else return {};
	}

	if (crew.empty() && exclusive)
	{
		return crewCancel(delay);
	}

	Input::CrewRefList group;
	group.reserve(crew.size());
	int prev = crew[0].get().uiBox-1;
	Input::Ret last;

	for (size_t i = 0; i < crew.size(); i++)
	{
		// Skip dead crew
		if (crew[i].get().dead) continue;

		// If not contiguous, input and clear group
		if (!group.empty() && prev != crew[i].get().uiBox-1)
		{
			if (group.size() == 1)
			{
				last = useCrewSingle(group[0], hotkeys(), exclusive, delay);
			}
			else
			{
				last = useCrewMany(group, exclusive, delay);
			}

			// stop being exclusive after first selection
			exclusive = false;

			group.clear();
		}

		group.push_back(crew[i]);
		prev = crew[i].get().uiBox;
	}

	// Final group
	if (!group.empty())
	{
		if (group.size() == state.ui.game->crewBoxes.size())
		{
			// Selecting all crew in order, use if hotkey possible
			auto hotkey = getHotkey(hotkeys(), "crew_all");

			if (hotkey != Key::Unknown)
			{
				return Input::keyPress(hotkey, false, delay);
			}
		}

		if (group.size() == 1)
		{
			last = useCrewSingle(group[0], hotkeys(), exclusive, delay);
		}
		else
		{
			last = useCrewMany(group, exclusive, delay);
		}
	}

	return last;
}

Input::Ret Input::crewSelect(
	int which,
	bool exclusive,
	bool suppress,
	double delay)
{
	auto&& state = Reader::getState();
	if (!validateInGame(state, suppress)) return {};

	auto&& list = state.game->playerCrew;
	int count = int(list.size());

	if (which < 0)
	{
		if (!suppress) throw InvalidCrewBoxChoice(which);
		else return {};
	}

	if (which > count)
	{
		if (!suppress) throw InvalidCrewBoxChoice(which, count);
		else return {};
	}

	return crewSelect(list.at(which), exclusive, suppress, delay);
}

Input::Ret Input::crewSelect(
	const std::vector<int>& which,
	bool exclusive,
	bool suppress,
	double delay)
{
	auto&& state = Reader::getState();
	if (!validateInGame(state, suppress)) return {};

	if (which.empty() && exclusive)
	{
		return crewCancel(delay);
	}

	Input::CrewRefList select;
	select.reserve(which.size());
	auto&& list = state.game->playerCrew;
	int count = int(list.size());

	for (size_t i = 0; i < which.size(); i++)
	{
		if (which[i] < 0)
		{
			if (!suppress) throw InvalidCrewBoxChoice(which[i]);
			else return {};
		}

		if (which[i] > count)
		{
			if (!suppress) throw InvalidCrewBoxChoice(which[i], count);
			else return {};
		}

		select.push_back(std::cref(state.game->playerCrew.at(which[i])));
	}

	return crewSelect(select, exclusive, suppress, delay);
}

Input::Ret Input::crewSelectAll(bool suppress, double delay)
{
	auto&& state = Reader::getState();
	if (!validateInGame(state, suppress)) return {};

	auto&& list = state.game->playerCrew;

	Input::CrewRefList select;
	select.reserve(list.size());

	// Should already be sorted
	for (size_t i = 0; i < list.size(); i++)
	{
		select.push_back(std::cref(list[i]));
	}

	return crewSelect(select, true, suppress, delay);
}

Input::Ret Input::crewCancel(bool suppress, double delay)
{
	auto&& state = Reader::getState();
	if (!validateInGame(state, suppress)) return {};

	if (state.game->pause.menu)
	{
		if (!suppress) throw WrongMenu("canceling crew selection");
		else return {};
	}

	return mouseClick(MouseButton::Left, nopSpot(), false, delay);
}

Input::Ret Input::crewSaveStations(bool suppress, double delay)
{
	auto&& state = Reader::getState();
	if (!validateInGame(state, suppress)) return {};

	return hotkeyOr(
		"saving crew stations", "savePositions", hotkeys(),
		state.ui.game->saveStations.center(), suppress, delay
	);
}

Input::Ret Input::crewLoadStations(bool suppress, double delay)
{
	auto&& state = Reader::getState();
	if (!validateInGame(state, suppress)) return {};

	return hotkeyOr(
		"loading crew stations", "loadPositions", hotkeys(),
		state.ui.game->loadStations.center(), suppress, delay
	);
}
