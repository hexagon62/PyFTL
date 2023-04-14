#include "Input.hpp"
#include "Utility/Memory.hpp"
#include "Utility/Exceptions.hpp"
#include "Utility/Float.hpp"
#include "Python/Bind.hpp"

#include <array>
#include <vector>
#include <deque>
#include <algorithm>
#include <tuple>
#include <variant>

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

	struct CheatCommand
	{
		std::string command;
	};

	struct PowerCommand
	{
		SystemType system = SystemType::None;
		int set = 0;
	};

	struct WeaponCommand
	{
		int weapon = -1;
		bool on = false;
	};

	struct DroneCommand
	{
		int drone = -1;
		bool on = false;
	};

	struct DoorCommand
	{
		int door = -1;
		bool open = false;
	};

	struct AimCommand
	{
		int room = -1;
		bool self = false;
		Point<int> start, end;
	};

	struct DeselectionCommand
	{
		bool left = false, right = true;
	};

	struct CrewSelectionCommand
	{
		std::vector<int> crew;
		bool exclusive = true;
	};

	struct Command
	{
		enum class Type
		{
			None = -1,
			
			// Basic types
			Mouse, Keyboard, TextInput, TextEvent, Cheat,
			
			// Helper stuff
			Pause, EventChoice,
			PowerSystem, PowerWeapon, PowerDrone,
			Deselect, SelectWeapon, SelectCrew,
			Autofire,
			Door, OpenAllDoors, CloseAllDoors,
			Aim, AimBeam,
			SaveStations, LoadStations
		};

		Type type = Type::None;
		double time = 0.0;

		std::variant<
			std::monostate,
			MouseCommand,
			KeyboardCommand,
			char,
			raw::TextEvent,
			CheatCommand,
			bool,
			int,
			PowerCommand,
			WeaponCommand,
			DroneCommand,
			DoorCommand,
			AimCommand,
			DeselectionCommand,
			CrewSelectionCommand
		> args;

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
				try
				{
					auto&& cmd = this->top();
					this->setImmediate(cmd.time);

					switch (cmd.type)
					{
					case Command::Type::Mouse:
						this->mouseInput(std::get<MouseCommand>(cmd.args));
						inputsMade++;
						break;
					case Command::Type::Keyboard:
						this->keyboardInput(std::get<KeyboardCommand>(cmd.args));
						inputsMade++;
						break;
					case Command::Type::TextInput:
						this->textInput(std::get<char>(cmd.args));
						inputsMade++;
						break;
					case Command::Type::TextEvent:
						this->textEvent(std::get<raw::TextEvent>(cmd.args));
						inputsMade++;
						break;
					case Command::Type::Cheat: this->cheat(std::get<CheatCommand>(cmd.args)); break;
					case Command::Type::Pause: this->pause(std::get<bool>(cmd.args)); break;
					case Command::Type::EventChoice: this->eventChoice(std::get<int>(cmd.args)); break;
					case Command::Type::PowerSystem: this->powerSystem(std::get<PowerCommand>(cmd.args)); break;
					case Command::Type::PowerWeapon: this->powerWeapon(std::get<WeaponCommand>(cmd.args)); break;
					case Command::Type::PowerDrone: this->powerDrone(std::get<DroneCommand>(cmd.args)); break;
					case Command::Type::Deselect: this->deselect(std::get<DeselectionCommand>(cmd.args)); break;
					case Command::Type::SelectWeapon: this->selectWeapon(std::get<WeaponCommand>(cmd.args)); break;
					case Command::Type::SelectCrew: this->selectCrew(std::get<CrewSelectionCommand>(cmd.args)); break;
					case Command::Type::Autofire: this->autofire(std::get<bool>(cmd.args)); break;
					case Command::Type::Door: this->doorToggle(std::get<DoorCommand>(cmd.args)); break;
					case Command::Type::OpenAllDoors: this->openAllDoors(std::get<bool>(cmd.args)); break;
					case Command::Type::CloseAllDoors: this->closeAllDoors(); break;
					case Command::Type::Aim: this->aim(std::get<AimCommand>(cmd.args)); break;
					case Command::Type::AimBeam: this->aimBeam(std::get<AimCommand>(cmd.args)); break;
					case Command::Type::SaveStations: this->saveStations(); break;
					case Command::Type::LoadStations: this->loadStations(); break;
					}
				}
				catch (const std::exception& e)
				{
					this->resetImmediate();
					this->pop();
					throw e;
				}
			}

			this->resetImmediate();
			this->pop();
		}
	}

	Input::Ret push(const Command& cmd)
	{
		double t = this->immediate == 0.0
			? Reader::now() 
			: this->immediate;

		if (this->immediate > 0.0)
			float_util::increment(this->immediate);

		this->queue.emplace_back(cmd).time = t;
		std::push_heap(this->queue.begin(), this->queue.end());
		return { t };
	}

	bool empty() const
	{
		return this->queue.empty();
	}

	bool queued(const Input::Ret& ret) const
	{
		return this->search(ret) != this->queue.end();
	}

	bool remove(const Input::Ret& ret)
	{
		auto it = this->search(ret);

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

	Command& get(const Input::Ret& ret)
	{
		return *this->search(ret);
	}

	Command& top()
	{
		return this->queue.front();
	}

	Key getHotkey(const std::string& k)
	{
		try
		{
			Key key = Input::hotkeys().at(k);

			return key;
		}
		catch (const std::out_of_range&)
		{
			throw InvalidHotkey(k);
		}
	}

private:
	friend class Input;
	using Queue = std::vector<Command>;
	Queue queue;
	double immediate = 0.0, startImmediate = 0.0;

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

	void setImmediate(double t)
	{
		this->immediate = t;
		this->startImmediate = Reader::now();
	}

	void resetImmediate()
	{
		this->immediate = 0.0;
		this->startImmediate = 0.0;
	}

	// Search for input
	Queue::iterator search(const Input::Ret& ret)
	{
		return std::find_if(
			this->queue.begin(), this->queue.end(),
			[&ret](auto&& cmd) {
				return cmd.time == ret.time;
			}
		);
	}

	Queue::const_iterator search(const Input::Ret& ret) const
	{
		return std::find_if(
			this->queue.cbegin(), this->queue.cend(),
			[&ret](auto&& cmd) {
				return cmd.time == ret.time;
			}
		);
	}

	void pop()
	{
		std::pop_heap(this->queue.begin(), this->queue.end());
		this->queue.pop_back();
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

	void cheat(const CheatCommand& cheat)
	{
		auto&& cmd = cheat.command;

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

	void checkInGame(const char* what)
	{
		auto&& state = Reader::getState();
		if (!state.game) throw GameNotRunning(what);

		auto&& pause = state.game->pause;
		if (pause.menu || pause.event) throw WrongMenu(what);
	}

	// Generic function to try a hotkey and fallback to clicking
	void hotkeyOr(const char* hotkey, Point<int> fallback)
	{
		// Use if hotkey possible
		auto k = this->getHotkey(hotkey);

		if (k != Key::Unknown)
		{
			Input::keyPress(k, false);
			return;
		}

		Input::mouseClick(MouseButton::Left, fallback, false);
	}

	struct PowerHotkey
	{
		Key key = Key::Unknown;
		bool shift = false;
	};

	// Find any working keyboard combo to (un)power the system
	PowerHotkey workingPowerHotkey(
		const System& sys,
		bool unpower)
	{
		Key k;

		if (unpower)
		{
			// Try to get the direct unpower key
			k = this->getHotkey(systemUnpowerHotkey(sys.type));
			if (k != Key::Unknown) return { k, false };

			// Then, fallback to direct power key + shift
			k = this->getHotkey(systemPowerHotkey(sys.type));
			if (k != Key::Unknown) return { k, true };

			// Then, fallback to positional unpower key
			int id = sys.uiBox + 1;
			k = this->getHotkey("un_power_" + std::to_string(id));
			if (k != Key::Unknown) return { k, false };

			// Then, fallback to positional power key + shift
			k = this->getHotkey("power_" + std::to_string(id));
			if (k != Key::Unknown) return { k, true };

			// Otherwise, there's no hotkey for it
			return { k, false };
		}

		// Try to get direct power key
		k = this->getHotkey(systemPowerHotkey(sys.type));
		if (k != Key::Unknown) return { k, false };

		// Then, fallback to positional power 
		int id = sys.uiBox + 1;
		k = this->getHotkey("power_" + std::to_string(id));
		if (k != Key::Unknown) return { k, false };

		// Otherwise, there's no hotkey for it
		return { k, false };
	}


	Key crewHotkey(int which)
	{
		return this->getHotkey("crew" + std::to_string(which + 1));
	}

	// Assumes a bunch of other checks have already been made
	void useCrewSingle(const Crew& crew, bool exclusive = true)
	{
		// Try to use a hotkey
		auto hotkey = this->crewHotkey(crew.id);

		if (hotkey == Key::Unknown)
		{
			// If there is none, use the mouse
			auto&& crewBoxes = Reader::getState().ui.game->crewBoxes;
			auto pos = crewBoxes[crew.id].box.center();

			Input::mouseClick(MouseButton::Left, pos, !exclusive);
			return;
		}

		Input::keyPress(hotkey, !exclusive);
	}

	using CrewRefList = std::vector<std::reference_wrapper<const Crew>>;

	// Assumes a bunch of other checks have already been made
	// Assumes crew list is sorted by ui box order already
	// Always uses mouse
	void useCrewMany(
		const CrewRefList& crew,
		bool exclusive = true)
	{
		auto&& crewBoxes = Reader::getState().ui.game->crewBoxes;

		Point<int> first = crewBoxes[crew.front().get().id].box.center();
		Point<int> last = crewBoxes[crew.back().get().id].box.center();

		Input::mouseDown(MouseButton::Left, first, !exclusive);
		Input::mouseUp(MouseButton::Left, last, !exclusive);
	}

	void pause(bool on)
	{
		auto&& state = Reader::getState();
		if (!state.game) throw GameNotRunning("pausing");

		// nothing to do
		if (state.game->pause.any == on) return;

		// middle mouse is always available, no need to check for hotkey
		Input::mouseClick(MouseButton::Middle, { -1, -1 }, false);
	}

	void eventChoice(int which)
	{
		auto&& state = Reader::getState();
		if (!state.game) throw GameNotRunning("picking an event choice");
		if (!state.game->event || !state.game->pause.event) throw NoEvent();

		auto&& event = state.ui.game->event;
		auto&& choices = event->choices;

		if (size_t(which) >= choices.size())
		{
			throw InvalidEventChoice(which, int(choices.size()));
		}

		// Hotkeys disabled, must click...
		bool noHotkeys = state.settings.eventChoiceSelection != EventChoiceSelection::DisableHotkeys;
		bool delay = state.settings.eventChoiceSelection != EventChoiceSelection::BriefDelay;
		if (noHotkeys || (delay && event->openTime.first > 0.f))
		{
			auto&& choice = choices.at(which);
			bool topCenter = !impl.offScreen(choice.box.topCenter());

			if (!topCenter)
			{
				throw InvalidEventChoice(which, "hotkeys are disabled/delayed and the choice is off-screen");
			}

			bool center = !impl.offScreen(choice.box.center());

			Input::mouseClick(
				MouseButton::Left,
				center ? choice.box.center() : choice.box.topCenter(),
				false);

			return;
		}

		Input::keyPress(Key(int(Key::Num1) + which), false);
	}

	// Setting the system power is quite complicated and has many edge cases
	// Mostly due to shields and zoltan power...
	// So yeah, I'll be real glad this helper function exists if it works
	void powerSystem(const PowerCommand& power)
	{
		auto&& [type, set] = power;

		this->checkInGame("powering a system");

		auto&& state = Reader::getState();
		auto&& system = state.game->playerShip->getSystem(type);

		int required = system.power.required;

		if (required <= 0 || required > 2)
		{
			throw InvalidPowerRequest(system, "this function can't handle that system");
		}

		if (system.subsystem())
		{
			throw InvalidPowerRequest(system, "subsystem power cannot be controlled");
		}

		int current = system.power.total.first;

		// Nothing to do
		if (current == set) return;

		int delta = set - current;

		auto range = system.powerRange();
		if (set < range.first || set > range.second)
		{
			throw InvalidPowerRequest(system, set, range);
		}

		int mod = set % required;
		int zoltan = system.power.zoltan;

		// Power must be between a multiple and a multiple + zoltan power present
		if (mod > zoltan)
		{
			throw InvalidPowerRequest(system, set, required);
		}

		int upInputs = 0;
		int downInputs = 0;

		if (set < current)
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
			int end = required > 1 ? ((set + 1) / required) * required : set;
			upInputs = (end - start) / required;
			if (mod != 0) downInputs = 1;

			if (end - current > state.game->playerShip->reactor.total.first)
			{
				 throw InvalidPowerRequest(system, "there isn't enough power in the reactor");
			}
		}

		auto doInput = [&](const PowerHotkey& k, bool depower) {
			auto&& [key, shift] = k;

			if (key != Key::Unknown) // hotkey found
			{
				Input::keyDown(key, shift);
			}
			else // must click instead
			{
				auto&& ship = *state.game->playerShip;
				auto&& point = state.ui.game->getSystem(system.type, system.discriminator).bottomCenter();
				Input::mouseClick(
					depower ? MouseButton::Right : MouseButton::Left,
					point, false
				);
			}
		};

		if (upInputs > 0)
		{
			auto k = this->workingPowerHotkey(system, false);
			for (int i = 0; i < upInputs; i++) doInput(k, false);
		}

		if (downInputs > 0)
		{
			auto k = this->workingPowerHotkey(system, true);
			for (int i = 0; i < downInputs; i++) doInput(k, true);
		}
	}

	// Assumes a bunch of other checks have already been made
	void useWeapon(const Weapon& weapon, bool powerOff = false)
	{
		auto&& hotkeys = Input::hotkeys();

		// Try to use a hotkey
		auto hotkey = getHotkey("weapon" + std::to_string(weapon.slot + 1));

		// If there is none, use the mouse
		if (hotkey == Key::Unknown)
		{
			auto&& weaponBoxes = Reader::getState().ui.game->weaponBoxes;
			auto pos = weaponBoxes[weapon.slot].center();
			auto button = powerOff ? MouseButton::Right : MouseButton::Left;

			if (powerOff)
			{
				Input::mouseClick(button, pos, false);
				return;
			}

			Input::mouseUp(button, pos, false);
			return;
		}

		Input::keyPress(hotkey, powerOff);
		return;
	}

	void powerWeapon(const WeaponCommand& cmd)
	{
		auto&& [slot, on] = cmd;

		this->checkInGame("powering a weapon");

		auto&& state = Reader::getState();
		if (!state.game->playerShip->weapons) throw SystemNotInstalled(SystemType::Weapons);

		auto&& weapSys = *state.game->playerShip->weapons;
		auto slots = int(weapSys.list.size());
		if (slot < 0) throw InvalidSlotChoice("weapon", slot);
		if (slot >= slots) throw InvalidSlotChoice("weapon", slot, slots);
		auto&& weapon = weapSys.list[slot];

		auto range = weapSys.powerRange();
		int current = weapon.power.total.first;
		if (range.first == range.second)
		{
			int desired = current + weapon.power.required * (on ? 1 : -1);
			throw InvalidPowerRequest(weapSys, desired, range);
		}

		if (on)
		{
			int needs = weapon.power.required - current;

			// Nothing needs to be done
			if (needs == 0) return;

			// Need missiles
			int missiles = state.game->playerShip->cargo.missiles;
			int neededMissiles = weapon.blueprint.missiles;
			if (missiles < neededMissiles)
			{
				throw InvalidSlotChoice("weapon", weapon.slot, "there isn't enough missiles");
			}

			if (needs > state.game->playerShip->reactor.total.first)
			{
				throw InvalidPowerRequest(weapon, "there isn't enough power in the reactor");
			}

			int remaining = weapSys.power.total.second - weapSys.power.total.first;
			if (needs > remaining)
			{
				throw InvalidPowerRequest(weapon, "there isn't enough power in the weapon system");
			}
		}
		else
		{
			int del = current - weapon.power.zoltan;

			// Nothing needs to be done
			if (del == 0) return;

			if (weapon.power.zoltan >= weapon.power.required)
			{
				throw InvalidPowerRequest(weapon, "it is fully powered by zoltan crew");
			}
		}

		this->useWeapon(weapon, !on);
	}

	// Assumes a bunch of other checks have already been made
	void useDrone(const Drone& drone, bool powerOff = false)
	{
		auto&& hotkeys = Input::hotkeys();

		// Try to use a hotkey
		auto hotkey = getHotkey("drone" + std::to_string(drone.slot + 1));

		// If there is none, use the mouse
		if (hotkey == Key::Unknown)
		{
			auto&& droneBoxes = Reader::getState().ui.game->droneBoxes;
			auto pos = droneBoxes[drone.slot].center();
			auto button = powerOff ? MouseButton::Right : MouseButton::Left;

			if (powerOff)
			{
				Input::mouseClick(button, pos, false);
				return;
			}

			Input::mouseUp(button, pos, false);
			return;
		}

		Input::keyPress(hotkey, powerOff);
	}

	void powerDrone(const DroneCommand& cmd)
	{
		auto&& [slot, on] = cmd;

		this->checkInGame("powering a drone");

		auto&& state = Reader::getState();
		if (!state.game->playerShip->drones) throw SystemNotInstalled(SystemType::Drones);

		auto&& droneSys = *state.game->playerShip->drones;
		auto slots = int(droneSys.list.size());
		if (slot < 0) throw InvalidSlotChoice("drone", slot);
		if (slot >= slots) throw InvalidSlotChoice("drone",slot, slots);
		auto&& drone = droneSys.list[slot];

		auto range = droneSys.powerRange();
		int current = drone.power.total.first;
		if (range.first == range.second)
		{
			int desired = current + drone.power.required * (on ? 1 : -1);
			throw InvalidPowerRequest(droneSys, desired, range);
		}

		if (on)
		{
			int needs = drone.power.required - current;

			// Nothing needs to be done
			if (needs == 0) return;

			// Need drone parts
			int droneParts = state.game->playerShip->cargo.droneParts;
			if (droneParts < 1)
			{
				throw InvalidSlotChoice("drone", drone.slot, "there isn't enough drone parts");
			}

			// Need to not be destroyed
			if (drone.dead)
			{
				throw InvalidSlotChoice("drone", drone.slot, "it is destroyed or on cooldown");
			}

			// Various drone types need an enemy ship
			bool enemy = state.game->enemyShip.has_value();
			if (drone.needsEnemy() && !enemy)
			{
				throw InvalidSlotChoice("drone", drone.slot, "it needs an enemy to function");
			}

			if (needs > state.game->playerShip->reactor.total.first)
			{
				throw InvalidPowerRequest(drone, "there isn't enough power in the reactor");
			}

			int remaining = droneSys.power.total.second - droneSys.power.total.first;
			if (needs > remaining)
			{
				throw InvalidPowerRequest(drone, "there isn't enough power in the drone system");
			}
		}
		else
		{
			int del = current - drone.power.zoltan;

			// Nothing needs to be done
			if (del == 0) return;

			if (drone.power.zoltan >= drone.power.required)
			{
				throw InvalidPowerRequest(drone, "it is fully powered by zoltan crew");
			}
		}

		this->useDrone(drone, !on);
	}

	// Some arbitrary spot that should have no UI elements
	Point<int> nopSpot()
	{
		return { Input::GAME_WIDTH - 1, Input::GAME_HEIGHT - 1 };
	}

	void deselect(const DeselectionCommand& cmd)
	{
		this->checkInGame("deselection");
		if (cmd.left) Input::mouseClick(MouseButton::Left, this->nopSpot(), false);
		if (cmd.right) Input::mouseClick(MouseButton::Right, this->nopSpot(), false);
	}

	void selectWeapon(const WeaponCommand& cmd)
	{
		auto&& slot = cmd.weapon;

		this->checkInGame("selecting a weapon");

		auto&& state = Reader::getState();
		if (!state.game->playerShip->weapons) throw SystemNotInstalled(SystemType::Weapons);

		auto&& weapSys = *state.game->playerShip->weapons;
		auto slots = int(weapSys.list.size());
		if (slot < 0) throw InvalidSlotChoice("weapon", slot);
		if (slot >= slots) throw InvalidSlotChoice("weapon", slot, slots);
		auto&& weapon = weapSys.list[slot];

		if (!weapon.powered()) throw InvalidSlotChoice("weapon", weapon.slot, "it is not powered");

		useWeapon(weapon, false);
	}

	void selectCrew(const CrewSelectionCommand& cmd)
	{
		auto&& crew = cmd.crew;
		bool exclusive = cmd.exclusive;

		this->checkInGame("selecting crew");

		auto&& state = Reader::getState();

		if (crew.empty() && exclusive)
		{
			Input::deselectCrew();
			return;
		}

		auto count = int(state.game->playerCrew.size());

		for(auto&& box : crew)
		{
			if (box < 0) throw InvalidCrewBoxChoice(box);
			if (box > count) throw InvalidCrewBoxChoice(box, count);

			auto&& c = state.game->playerCrew[box];

			if (c.dead) throw InvalidCrewChoice(c, "they're dead");
			if (c.drone) throw InvalidCrewChoice(c, "they're a drone");
			if (!c.player) throw InvalidCrewChoice(c, "they're an enemy");
		}

		CrewRefList group;
		group.reserve(crew.size());
		int prev = crew[0] - 1;

		for (size_t i = 0; i < crew.size(); i++)
		{
			// If not contiguous, input and clear group
			if (!group.empty() && prev != crew[i] - 1)
			{
				if (group.size() == 1) this->useCrewSingle(group[0], exclusive);
				else this->useCrewMany(group, exclusive);

				// stop being exclusive after first selection
				exclusive = false;

				group.clear();
			}

			group.push_back(state.game->playerCrew[crew[i]]);
			prev = crew[i];
		}

		// Final group
		if (!group.empty())
		{
			if (group.size() == state.ui.game->crewBoxes.size())
			{
				// Selecting all crew in order, use if hotkey possible
				auto hotkey = this->getHotkey("crew_all");

				if (hotkey != Key::Unknown)
				{
					Input::keyPress(hotkey, false);
					return;
				}
			}

			if (group.size() == 1) this->useCrewSingle(group[0], exclusive);
			else this->useCrewMany(group, exclusive);
		}
	}

	void autofire(bool on)
	{
		this->checkInGame("toggling auto-fire");
		auto&& state = Reader::getState();

		if (!state.game->playerShip->weapons)
		{
			throw SystemNotInstalled(SystemType::Weapons);
		}

		if (state.game->playerShip->weapons->autoFire == on)
		{
			// Nothing to do
			return;
		}

		this->hotkeyOr("autofire", state.ui.game->autofire->center());
	}

	void doorToggle(const DoorCommand& cmd)
	{
		auto&& [id, open] = cmd;

		this->checkInGame("toggling a door");
		auto&& state = Reader::getState();

		auto&& doors = state.game->playerShip->doorControl;
		if (!doors) throw SystemNotInstalled(SystemType::Doors);
		if (size_t(id) >= state.game->playerShip->doors.size()) throw std::range_error("door has invalid id");
		auto&& door = state.game->playerShip->doors[id];

		if (!doors->operable()) throw DoorInoperable(door, "the door system is hacked or too damaged");
		if (!door.player) throw DoorInoperable(door, "it does not belong to the player");
		if (door.openFake) throw DoorInoperable(door, "has crew standing in it");

		if (door.open == open)
		{
			// nothing to do
			return;
		}

		Input::mouseClick(MouseButton::Left, door.rect.center(), false);
	}

	void openAllDoors(bool airlocks)
	{
		this->checkInGame("opening all doors");

		auto&& state = Reader::getState();
		auto&& doorControl = state.game->playerShip->doorControl;

		if (!doorControl) throw SystemNotInstalled(SystemType::Doors);
		if (!doorControl->operable()) throw DoorInoperable("it is either hacked or damaged");

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
			return;
		}

		auto p = state.ui.game->openAllDoors->center();

		if (airlocks && !onlyAirlocksClosed)
		{
			// Tap the button again
			this->hotkeyOr("open", p);
		}

		this->hotkeyOr("open", p);
	}

	void closeAllDoors()
	{
		this->checkInGame("closing all doors");

		auto&& state = Reader::getState();
		auto&& doorControl = state.game->playerShip->doorControl;

		if (!doorControl) throw SystemNotInstalled(SystemType::Doors);
		if (!doorControl->operable()) throw DoorInoperable("it is either hacked or damaged");

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
			return;
		}

		this->hotkeyOr("close", state.ui.game->closeAllDoors->center());
	}

	void aim(const AimCommand& aim)
	{
		auto&& id = aim.room;
		auto&& self = aim.self;

		this->checkInGame("aiming weapon/system");
		auto&& state = Reader::getState();

		if (self && size_t(id) >= state.game->playerShip->rooms.size())
		{
			throw std::range_error("room has invalid id");
		}

		if (!self && size_t(id) >= state.game->enemyShip->rooms.size())
		{
			throw std::range_error("room has invalid id");
		}

		const Room& room = self
			? state.game->playerShip->rooms[id]
			: state.game->enemyShip->rooms[id];

		if (std::holds_alternative<std::monostate>(state.ui.mouse.aiming))
		{
			throw NotSelected("weapon/system");
		}

		if (std::holds_alternative<SystemUIRef>(state.ui.mouse.aiming))
		{
			auto&& system = std::get<SystemUIRef>(state.ui.mouse.aiming).get();

			if (room.player)
			{
				if (system.type != SystemType::MindControl)
				{
					throw InvalidSelfAim("only mind control can do that");
				}
				else
				{
					bool selfMind = false;

					for (auto&& c : room.crew)
					{
						if (selfMind) break;

						if ((c->player && c->mindControlled) || !c->player)
							selfMind = true;
					}

					for (auto&& c : room.crewMoving)
					{
						if (selfMind) break;

						if ((c->player && c->mindControlled) || !c->player)
							selfMind = true;
					}

					if (!selfMind)
					{
						throw InvalidSelfAim("mind control requires enemy crew or mind controlled player crew");
					}
				}
			}
		}

		if (std::holds_alternative<WeaponUIRef>(state.ui.mouse.aiming))
		{
			auto&& weapon = std::get<WeaponUIRef>(state.ui.mouse.aiming).get();

			if (room.player && weapon.blueprint.type != WeaponType::Bomb)
			{
				throw InvalidSelfAim("only bomb weapons can do that");
			}
		}

		Input::mouseClick(MouseButton::Left, room.rect.center(), false);
	}

	void aimBeam(const AimCommand& aim)
	{
		auto&& [id, self, start, end] = aim;

		this->checkInGame("aiming beam");
		auto&& state = Reader::getState();

		if (self && size_t(id) >= state.game->playerShip->rooms.size())
		{
			throw std::range_error("room has invalid id");
		}

		if (!self && size_t(id) >= state.game->enemyShip->rooms.size())
		{
			throw std::range_error("room has invalid id");
		}

		const Room& room = self
			? state.game->playerShip->rooms[id]
			: state.game->enemyShip->rooms[id];

		if (!std::holds_alternative<WeaponUIRef>(state.ui.mouse.aiming))
		{
			throw NotSelected("beam weapon");
		}

		auto&& weapon = std::get<WeaponUIRef>(state.ui.mouse.aiming).get();

		if (weapon.blueprint.type != WeaponType::Beam)
		{
			throw NotSelected("beam weapon");
		}

		if (room.player) throw InvalidSelfAim("only bomb weapons can do that");

		bool notInside =
			start.x < 1 || start.y < 1 ||
			start.x >= room.rect.w || start.y >= room.rect.h;

		if (notInside)
		{
			throw std::invalid_argument("tried to aim a beam from a position not inside a room");
		}

		Point<int> diff = end - start;
		int offsetMagnitudeSquared = diff.x * diff.x + diff.y * diff.y;
		constexpr int MIN_DIST = Weapon::HARDCODED_MINIMUM_BEAM_AIM_DISTANCE;
		constexpr int MIN_DIST_SQUARED = MIN_DIST * MIN_DIST;

		if (offsetMagnitudeSquared < MIN_DIST_SQUARED)
		{
			throw std::invalid_argument(
				"the two beam aiming points must be at least " +
				std::to_string(MIN_DIST) + " pixels apart"
			);
		}

		Input::mouseClick(MouseButton::Left, room.rect.topLeft() + start, false);
		Input::mouseClick(MouseButton::Left, room.rect.topLeft() + end, false);
	}

	void saveStations()
	{
		this->checkInGame("saving crew stations");
		auto&& state = Reader::getState();
		this->hotkeyOr("savePositions", state.ui.game->saveStations.center());
	}

	void loadStations()
	{
		this->checkInGame("loading crew stations");
		auto&& state = Reader::getState();
		this->hotkeyOr("loadPositions", state.ui.game->loadStations.center());
	}
};

Input::Impl Input::impl;
bool Input::good = false;
bool Input::humanMouse = true;
bool Input::humanKeyboard = true;

namespace
{

const Weapon* weaponAt(int which, bool suppressExceptions = false)
{
	auto&& opt = Reader::getState().game->playerShip->weapons;

	if (!opt)
	{
		if (!suppressExceptions) throw SystemNotInstalled(SystemType::Weapons);
		else return nullptr;
	}

	auto&& weapons = *opt;
	auto size = int(weapons.list.size());

	if (which < 0 || which >= size)
	{
		if (!suppressExceptions) throw InvalidSlotChoice("weapon", which, size);
		else return nullptr;
	}

	return &weapons.list.at(which);
}

const Drone* droneAt(int which, bool suppressExceptions = false)
{
	auto&& opt = Reader::getState().game->playerShip->drones;

	if (!opt)
	{
		if (!suppressExceptions) throw SystemNotInstalled(SystemType::Drones);
		else return nullptr;
	}

	auto&& drones = *opt;
	auto size = int(drones.list.size());

	if (which < 0 || which >= size)
	{
		if (!suppressExceptions) throw InvalidSlotChoice("drone", which, size);
		else return nullptr;
	}

	return &drones.list.at(which);
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

bool Input::queued(const Input::Ret& input)
{
	return impl.queued(input);
}

bool Input::pop(const Input::Ret& input)
{
	return impl.remove(input);
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

Input::Ret::operator bool() const
{
	return queued(*this);
}

Input::Ret Input::dummy()
{
	return {};
}

Input::Ret Input::mouseMove(const Point<int>& position)
{
	return impl.push({
		.type = Impl::Command::Type::Mouse,
		.args = Impl::MouseCommand{
			.position = position
		},
	});
}

Input::Ret Input::mouseDown(
	MouseButton button,
	const Point<int>& position,
	bool shift)
{
	mouseMove(position);
	return impl.push({
		.type = Impl::Command::Type::Mouse,
		.args = Impl::MouseCommand{
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
	bool shift)
{
	mouseMove(position);
	return impl.push({
		.type = Impl::Command::Type::Mouse,
		.args = Impl::MouseCommand{
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
	bool shift)
{
	mouseMove(position);

	impl.push({
		.type = Impl::Command::Type::Mouse,
		.args = Impl::MouseCommand{
			.button = button,
			.position = position,
			.shift = shift,
			.direction = InputDirection::Down
		}
	});

	return impl.push({
		.type = Impl::Command::Type::Mouse,
		.args = Impl::MouseCommand{
			.button = button,
			.position = position,
			.shift = shift,
			.direction = InputDirection::Up
		}
	});
}

Input::Ret Input::keyDown(
	Key key,
	bool shift)
{
	return impl.push({
		.type = Impl::Command::Type::Keyboard,
		.args = Impl::KeyboardCommand{
			.key = key,
			.shift = shift,
			.direction = InputDirection::Down
		}
	});
}

Input::Ret Input::keyUp(
	Key key,
	bool shift)
{
	return impl.push({
		.type = Impl::Command::Type::Keyboard,
		.args = Impl::KeyboardCommand{
			.key = key,
			.shift = shift,
			.direction = InputDirection::Up
		}
	});
}

Input::Ret Input::keyPress(
	Key key,
	bool shift)
{
	keyDown(key, shift);
	return keyUp(key, false);
}

Input::Ret Input::hotkeyDown(
	const std::string& hotkey,
	bool shift)
{
	Key key = impl.getHotkey(hotkey);
	return keyDown(key, shift);
}

Input::Ret Input::hotkeyUp(
	const std::string& hotkey,
	bool shift)
{
	Key key = impl.getHotkey(hotkey);
	return keyUp(key, shift);
}

Input::Ret Input::hotkeyPress(
	const std::string& hotkey,
	bool shift)
{
	Key key = impl.getHotkey(hotkey);
	return keyPress(key, shift);
}

const decltype(Settings::hotkeys)& Input::hotkeys()
{
	return Reader::getState().settings.hotkeys;
}

Input::Ret Input::text(char ch)
{
	return impl.push({
		.type = Impl::Command::Type::TextInput,
		.args = ch
	});
}

Input::Ret Input::text(const std::string& str)
{
	Input::Ret last;
	for (auto&& c : str) last = text(c);
	return last;
}

Input::Ret Input::textConfirm()
{
	return impl.push({
		.type = Impl::Command::Type::TextEvent,
		.args = raw::TEXT_CONFIRM
	});
}

Input::Ret Input::textClear()
{
	return impl.push({
		.type = Impl::Command::Type::TextEvent,
		.args = raw::TEXT_CLEAR
	});
}

Input::Ret Input::textBackspace()
{
	return impl.push({
		.type = Impl::Command::Type::TextEvent,
		.args = raw::TEXT_BACKSPACE
	});
}

Input::Ret Input::textDelete()
{
	return impl.push({
		.type = Impl::Command::Type::TextEvent,
		.args = raw::TEXT_DELETE
	});
}

Input::Ret Input::textLeft()
{
	return impl.push({
		.type = Impl::Command::Type::TextEvent,
		.args = raw::TEXT_LEFT
	});
}

Input::Ret Input::textRight()
{
	return impl.push({
		.type = Impl::Command::Type::TextEvent,
		.args = raw::TEXT_RIGHT
	});
}

Input::Ret Input::textHome()
{
	return impl.push({
		.type = Impl::Command::Type::TextEvent,
		.args = raw::TEXT_HOME
	});
}

Input::Ret Input::textEnd()
{
	return impl.push({
		.type = Impl::Command::Type::TextEvent,
		.args = raw::TEXT_END
	});
}

Input::Ret Input::cheat(const std::string& command)
{
	return impl.push({
		.type = Impl::Command::Type::Cheat,
		.args = Impl::CheatCommand{
			.command = command
		}
	});
}

Input::Ret Input::pause(bool on)
{
	return impl.push({
		.type = Impl::Command::Type::Pause,
		.args = on
	});
}

Input::Ret Input::choice(int which)
{
	return impl.push({
		.type = Impl::Command::Type::EventChoice,
		.args = which
	});
}

Input::Ret Input::powerSystem(SystemType system, int set)
{
	return impl.push({
		.type = Impl::Command::Type::PowerSystem,
		.args = Impl::PowerCommand{
			.system = system,
			.set = set
		}
	});
}

Input::Ret Input::powerWeapon(int weapon, bool on)
{
	return impl.push({
		.type = Impl::Command::Type::PowerWeapon,
		.args = Impl::WeaponCommand{
			.weapon = weapon,
			.on = on
		}
	});
}

Input::Ret Input::powerDrone(int drone, bool on)
{
	return impl.push({
		.type = Impl::Command::Type::PowerDrone,
		.args = Impl::DroneCommand{
			.drone = drone,
			.on = on
		}
	});
}

Input::Ret Input::selectWeapon(int weapon)
{
	return impl.push({
		.type = Impl::Command::Type::SelectWeapon,
		.args = Impl::WeaponCommand{
			.weapon = weapon
		}
	});
}

Input::Ret Input::selectCrew(const std::vector<int>& crew, bool exclusive)
{
	return impl.push({
		.type = Impl::Command::Type::SelectCrew,
		.args = Impl::CrewSelectionCommand{
			.crew = crew
		}
	});
}

Input::Ret Input::door(int door, bool open)
{
	return impl.push({
		.type = Impl::Command::Type::Door,
		.args = Impl::DoorCommand{
			.door = door,
			.open = open
		}
	});
}

Input::Ret Input::doorAll(bool open, bool airlocks)
{
	if (open)
	{
		return impl.push({
			.type = Impl::Command::Type::OpenAllDoors,
			.args = airlocks
		});
	}

	return impl.push({
		.type = Impl::Command::Type::CloseAllDoors,
	});
}

Input::Ret Input::autofire(bool on)
{
	return impl.push({
		.type = Impl::Command::Type::Autofire,
		.args = on
	});
}

Input::Ret Input::aim(int room, bool self)
{
	return impl.push({
		.type = Impl::Command::Type::Aim,
		.args = Impl::AimCommand{
			.room = room,
			.self = self
		}
	});
}

Input::Ret Input::aim(
	int room,
	const Point<int>& start,
	const Point<int>& end)
{
	return impl.push({
		.type = Impl::Command::Type::AimBeam,
		.args = Impl::AimCommand{
			.room = room,
			.self = false,
			.start = start,
			.end = end
		}
	});
}

Input::Ret Input::quitAiming()
{
	return impl.push({
		.type = Impl::Command::Type::Deselect,
		.args = Impl::DeselectionCommand{
			.left = false,
			.right = true
		}
	});
}

Input::Ret Input::deselectCrew()
{
	return impl.push({
		.type = Impl::Command::Type::Deselect,
		.args = Impl::DeselectionCommand{
			.left = true,
			.right = false
		}
	});
}

Input::Ret Input::saveStations()
{
	return impl.push({
		.type = Impl::Command::Type::SaveStations
	});
}

Input::Ret Input::loadStations()
{
	return impl.push({
		.type = Impl::Command::Type::LoadStations
	});
}
