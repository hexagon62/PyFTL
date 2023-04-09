#include "Input.hpp"
#include "Utility/Memory.hpp"
#include "Utility/Exceptions.hpp"
#include "Utility/Float.hpp"

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
		Point<int> position{ -1, -1 };
		MouseButton button;
		InputDirection direction = InputDirection::Unchanged;
		bool shift = false;
	};

	struct KeyboardCommand
	{
		Key key;
		InputDirection direction = InputDirection::Unchanged;
		bool shift = false;
	};

	struct Command
	{
		enum class Type
		{
			Mouse, Keyboard
		};

		Type type;
		double time;

		union
		{
			MouseCommand mouse;
			KeyboardCommand keyboard;
		};

		uintmax_t id = uintmax_t(-1);

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
			auto lowerBound = Reader::now() - 0.5;
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
		this->queue.back().time += Reader::now();
		std::push_heap(this->queue.begin(), this->queue.end());
		return { this->idCounter, this->queue.back().time };
	}

	bool empty() const
	{
		return this->queue.empty();
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
		this->ctrlStateHook.unhook();
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
	mem::Hook shiftStateHook, ctrlStateHook;

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

		if (held)
		{
			if (!this->shiftStateHook.hooked())
			{
				auto addr = (BYTE*)Reader::getRealAddress(SHIFT_STATE_ADDR, {});
				this->shiftStateHook.hook(addr, RET_1.data(), RET_1.size(), true);
			}
			mrs.app->shift_held = true;
		}
		else
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
			mrs.app->OnMouseMove(pos.x, pos.y, pos.x - old.x, pos.y - old.y, false, false, false);
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
	}

	static bool offScreen(const Point<int>& p)
	{
		return
			p.x < 0 || p.x >= Input::GAME_WIDTH ||
			p.y < 0 || p.y >= Input::GAME_HEIGHT;
	}

	static void postMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		PostMessage(g_hGameWindow, uMsg | WM_FTLAI, wParam, lParam);
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

namespace fl = float_util;

double nextDelay(double t)
{
	return fl::increment(t - Reader::now());
}

void validateDelay(double d)
{
	if (d < -1.0)
	{
		throw InvalidTime("we can't send commands to the past to undo our mistakes");
	}

	if (d > 86400.0)
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

}

void Input::iterate()
{
	impl.iterate();
}

bool Input::empty()
{
	return impl.empty();
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
	double time)
{
	validateDelay(time);
	return impl.push(Impl::Command{
		.type = Impl::Command::Type::Mouse,
		.time = time,
		.mouse = {
			.position = position
		},
	});
}

Input::Ret Input::mouseDown(
	MouseButton button,
	const Point<int>& position,
	bool shift,
	double time)
{
	validateDelay(time);
	auto [_, moveTime] = mouseMove(position, time);
	return impl.push(Impl::Command{
		.type = Impl::Command::Type::Mouse,
		.time = nextDelay(moveTime),
		.mouse = {
			.position = position,
			.button = button,
			.direction = InputDirection::Down,
			.shift = shift
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
	auto [_, moveTime] = mouseMove(position, delay);
	return impl.push(Impl::Command{
		.type = Impl::Command::Type::Mouse,
		.time = nextDelay(moveTime),
		.mouse = {
			.position = position,
			.button = button,
			.direction = InputDirection::Up,
			.shift = shift
		}
	});
}

Input::Ret Input::mouseClick(
	const Point<int>& position,
	MouseButton button,
	bool shift,
	double delay)
{
	auto [_, moveTime] = mouseMove(position, delay);
	auto [__, downTime] = impl.push(Impl::Command{
		.type = Impl::Command::Type::Mouse,
		.time = moveTime,
		.mouse = {
			.position = position,
			.button = button,
			.direction = InputDirection::Down,
			.shift = shift
		}
	});

	return impl.push(Impl::Command{
		.type = Impl::Command::Type::Mouse,
		.time = nextDelay(downTime),
		.mouse = {
			.position = position,
			.button = button,
			.direction = InputDirection::Up,
			.shift = shift
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
		.time = delay,
		.keyboard = {
			.key = key,
			.direction = InputDirection::Down,
			.shift = shift
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
		.time = delay,
		.keyboard = {
			.key = key,
			.direction = InputDirection::Up,
			.shift = shift
		}
	});
}

Input::Ret Input::keyPress(
	Key key,
	bool shift,
	double delay)
{
	auto [_, downTime] = keyDown(key, shift, delay);
	return keyUp(key, false, nextDelay(downTime));
}

struct PowerHotkey
{
	Key key = Key::Unknown;
	bool shift = false;
};

// Find any working keyboard combo to (un)power the system
PowerHotkey workingPowerHotkey(
	const System& sys,
	const decltype(Settings::hotkeys)& map,
	bool unpower)
{
	Key k;
	
	if (unpower)
	{
		// Try to get the direct unpower key
		k = getHotkey(map, systemUnpowerHotkey(sys.type));
		if (k != Key::Unknown) return { k, false };

		// Then, fallback to direct power key + shift
		k = getHotkey(map, systemPowerHotkey(sys.type));
		if (k != Key::Unknown) return { k, true };

		// Then, fallback to positional unpower key
		int id = sys.uiBox+1;
		k = getHotkey(map, "un_power_" + std::to_string(id));
		if (k != Key::Unknown) return { k, false };

		// Then, fallback to positional power key + shift
		k = getHotkey(map, "power_" + std::to_string(id));
		if (k != Key::Unknown) return { k, true };

		// Otherwise, there's no hotkey for it
		return { k, false };
	}

	// Try to get direct power key
	k = getHotkey(map, systemPowerHotkey(sys.type));
	if (k != Key::Unknown) return { k, false };

	// Then, fallback to positional power 
	int id = sys.uiBox+1;
	k = getHotkey(map, "power_" + std::to_string(id));
	if (k != Key::Unknown) return { k, false };

	// Otherwise, there's no hotkey for it
	return { k, false };
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

Input::Ret Input::eventChoice(
	int which,
	bool suppress,
	double delay)
{
	auto&& game = Reader::getState().game;

	if (!game)
	{
		if (!suppress) throw GameNotRunning();
		else return {};
	}
	else if(!game->event || !game->pause.event)
	{
		if (!suppress) throw NoEvent();
		else return {};
	}

	auto&& choices = game->event->choices;
		
	if (size_t(which) >= choices.size())
	{
		if (!suppress) throw InvalidEventChoice(which, int(choices.size()));
		else return {};
	}

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

	// Should be impossible, but just in-case
	if (!state.game || !state.game->playerShip)
	{
		if (!suppress) throw GameNotRunning();
		else return {};
	}

	if (delta != 0 && set > 0)
	{
		if (!suppress) throw std::invalid_argument("delta and set cannot both be non-zero");
		else return {};
	}

	int required = system.power.required;

	if (required == 0) // weapons or drones
	{
		if (!suppress) throw InvalidPowerRequest(system, "the power state of that system is not granular");
		else return {};
	}

	// ...if somehow this ever happens, we can't handle it yet
	// I tried to generalize but I couldn't think of a satisfying solution
	if (required > 2)
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
		int start = (current / required) * required;
		int end = ((desired + 1) / required) * required;
		upInputs = (end - start) / required;
		if (mod != 0) downInputs = 1;

		if (end - current > state.game->playerShip->reactor.total.first)
		{
			if (!suppress) throw InvalidPowerRequest(system, "there isn't enough power in the reactor");
			else downInputs = 0; // just input upwards for the "not enough power" message
		}
	}

	Ret last{ 0, delay + Reader::now() };

	auto doInput = [&](const PowerHotkey& k, bool depower) {
		auto&& [key, shift] = k;

		if (key != Key::Unknown) // hotkey found
		{
			last = keyDown(key, shift, nextDelay(last.second));
		}
		else // must click instead
		{
			auto&& ship = *state.game->playerShip;
			auto&& point = state.ui.game->getSystem(system.type, system.discriminator).bottomCenter();
			last = mouseClick(
				point,
				depower ? MouseButton::Right : MouseButton::Left,
				false,
				nextDelay(last.second)
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
