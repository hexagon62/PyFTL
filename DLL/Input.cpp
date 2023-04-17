#include "Input.hpp"
#include "Utility/Memory.hpp"
#include "Utility/Exceptions.hpp"
#include "Utility/Float.hpp"
#include "Python/Bind.hpp"

#include <array>
#include <vector>
#include <list>
#include <algorithm>
#include <tuple>
#include <variant>

struct WaitCommand
{
	double time = 0.0;
};

struct MouseCommand
{
	MouseButton button;
	Point<int> pos{ -1, -1 };
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
	int which = 0;
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

struct SwapCommand
{
	int slotA = -1, slotB = -1;
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
	std::optional<bool> autofire;
	Point<int> start, end;
};

struct DeselectCommand
{
	bool left = true, right = true;
};

struct CrewSelectionCommand
{
	std::vector<int> crew;
};

struct SendCrewCommand
{
	int room = -1;
	bool self = false;
};

struct UpgradeSystemCommand
{
	SystemType system;
	int to = 0;
	int which = 0;
};

struct UpgradeReactorCommand
{
	int to = 0;
};

struct RenameCrewCommand
{
	int which = -1;
	std::string name;
};

struct DiscardCommand
{
	int which = -1;
};

struct Command
{
	enum class Type
	{
		None = -1,

		// Basic types
		Wait,
		Mouse, Keyboard,
		TextInput, TextEvent,
		Cheat,

		// In-game helper stuff
		Pause, EventChoice,
		PowerSystem, PowerWeapon, PowerDrone,
		Deselect, SelectWeapon, SelectCrew,
		SwapWeapons, SwapDrones,
		CrewAbility,
		Autofire,
		TeleportSend, TeleportReturn,
		Cloak, Battery, MindControl,
		SetupHack, Hack,
		Door, OpenAllDoors, CloseAllDoors,
		Aim, AimBeam,
		SendCrew, SaveStations, LoadStations,

		// Menus opened directly from in-game
		Jump, LeaveCrew,
		Upgrades, CrewManifest, Cargo,
		Store, Menu,

		// Ship menu stuff
		UpgradeSystem, UpgradeReactor, UndoUpgrades,
		RenameCrew, DismissCrew, ConfirmDismissCrew,
		SwapCargo, SwapWeaponCargo, SwapDroneCargo,
		DiscardCargo, DiscardWeapon, DiscardDrone, DiscardAugment,

		// Store stuff
		BuyItem,
		BuyFuel, BuyMissiles, BuyDroneParts,
		BuyRepair, BuyRepairAll,
		ConfirmPurchase,

		// Star map stuff
		JumpToBeacon, OpenSectors, JumpToSector,

		// Misc menu stuff
		CloseMenu
	};

	Type type = Type::None;
	uintmax_t id = 0;

	std::variant<
		std::monostate,
		WaitCommand,
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
		SwapCommand,
		DoorCommand,
		AimCommand,
		CrewSelectionCommand,
		DeselectCommand,
		SendCrewCommand,
		UpgradeSystemCommand,
		UpgradeReactorCommand,
		RenameCrewCommand,
		DiscardCommand
	> args;
};

class Input::Impl
{
public:
	static constexpr int INPUT_CAP = 1;

	Impl()
	{
		auto&& mrs = Reader::getRawState(MutableRawState{});
		this->resetImmediate();
	}

	~Impl() = default;

	void iterate()
	{
		int inputsMade = 0;

		while (inputsMade < INPUT_CAP && !this->empty())
		{
			try
			{
				this->setImmediate();
				auto&& cmd = this->top();

				switch (cmd.type)
				{
				case Command::Type::Wait:
					if (this->waiting(std::get<WaitCommand>(cmd.args)))
					{
						this->resetImmediate();
						return;
					}
					break;
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
				case Command::Type::Cheat:
					this->cheat(std::get<CheatCommand>(cmd.args));
					inputsMade++;
					break;
				case Command::Type::Pause: this->pause(std::get<bool>(cmd.args)); break;
				case Command::Type::EventChoice: this->choice(std::get<int>(cmd.args)); break;
				case Command::Type::PowerSystem: this->powerSystem(std::get<PowerCommand>(cmd.args)); break;
				case Command::Type::PowerWeapon: this->powerWeapon(std::get<WeaponCommand>(cmd.args)); break;
				case Command::Type::PowerDrone: this->powerDrone(std::get<DroneCommand>(cmd.args)); break;
				case Command::Type::Deselect: this->deselect(std::get<DeselectCommand>(cmd.args)); break;
				case Command::Type::SelectWeapon: this->selectWeapon(std::get<WeaponCommand>(cmd.args)); break;
				case Command::Type::SelectCrew: this->selectCrew(std::get<CrewSelectionCommand>(cmd.args)); break;
				case Command::Type::SwapWeapons: this->swapWeapons(std::get<SwapCommand>(cmd.args)); break;
				case Command::Type::SwapDrones: this->swapDrones(std::get<SwapCommand>(cmd.args)); break;
				case Command::Type::CrewAbility: this->crewAbility(); break;
				case Command::Type::Autofire: this->autofire(std::get<bool>(cmd.args)); break;
				case Command::Type::TeleportSend: this->teleportSend(); break;
				case Command::Type::TeleportReturn:this->teleportReturn(); break;
				case Command::Type::Cloak: this->cloak(); break;
				case Command::Type::Battery: this->battery(); break;
				case Command::Type::MindControl: this->mindControl(); break;
				case Command::Type::SetupHack: this->setupHack(); break;
				case Command::Type::Hack: this->hack(); break;
				case Command::Type::Door: this->doorToggle(std::get<DoorCommand>(cmd.args)); break;
				case Command::Type::OpenAllDoors: this->openAllDoors(std::get<bool>(cmd.args)); break;
				case Command::Type::CloseAllDoors: this->closeAllDoors(); break;
				case Command::Type::Aim: this->aim(std::get<AimCommand>(cmd.args)); break;
				case Command::Type::AimBeam: this->aimBeam(std::get<AimCommand>(cmd.args)); break;
				case Command::Type::SendCrew: this->sendCrew(std::get<SendCrewCommand>(cmd.args)); break;
				case Command::Type::SaveStations: this->saveStations(); break;
				case Command::Type::LoadStations: this->loadStations(); break;
				case Command::Type::Jump: this->jump(); break;
				case Command::Type::LeaveCrew: this->leaveCrew(std::get<bool>(cmd.args)); break;
				case Command::Type::Upgrades: this->upgrades(); break;
				case Command::Type::CrewManifest: this->crewManifest(); break;
				case Command::Type::Cargo: this->cargo(); break;
				case Command::Type::Store: this->store(); break;
				case Command::Type::Menu: this->menu(); break;
				case Command::Type::UpgradeSystem: this->upgradeSystem(std::get<UpgradeSystemCommand>(cmd.args)); break;
				case Command::Type::UpgradeReactor: this->upgradeReactor(std::get<UpgradeReactorCommand>(cmd.args)); break;
				case Command::Type::UndoUpgrades: this->undoUpgrades(); break;
				case Command::Type::RenameCrew: this->renameCrew(std::get<RenameCrewCommand>(cmd.args)); break;
				case Command::Type::DismissCrew: this->dismissCrew(std::get<DiscardCommand>(cmd.args)); break;
				case Command::Type::ConfirmDismissCrew: this->confirmDismissCrew(std::get<bool>(cmd.args)); break;
				case Command::Type::SwapCargo: this->swapCargo(std::get<SwapCommand>(cmd.args)); break;
				case Command::Type::SwapWeaponCargo: this->swapWeaponCargo(std::get<SwapCommand>(cmd.args)); break;
				case Command::Type::SwapDroneCargo: this->swapDroneCargo(std::get<SwapCommand>(cmd.args)); break;
				case Command::Type::DiscardCargo: this->discardCargo(std::get<DiscardCommand>(cmd.args)); break;
				case Command::Type::DiscardWeapon: this->discardWeapon(std::get<DiscardCommand>(cmd.args)); break;
				case Command::Type::DiscardDrone: this->discardDrone(std::get<DiscardCommand>(cmd.args)); break;
				case Command::Type::DiscardAugment: this->discardAugment(std::get<DiscardCommand>(cmd.args)); break;
				}
			}
			catch (const std::exception& e)
			{
				this->pop();
				throw e;
			}

			this->pop();
		}
	}

	Input::Ret push(const Command& cmd)
	{
		if (this->immediate)
		{
			this->immediateIt++;
			this->immediateIt = this->queue.insert(this->immediateIt, cmd);
			this->immediateIt->id = ++this->idCounter;
			return this->idCounter;
		}

		this->queue.push_back(cmd);
		this->queue.back().id = ++this->idCounter;

		return this->idCounter;
	}

	bool empty() const
	{
		return this->queue.empty();
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
	using Queue = std::list<Command>;
	Queue queue;
	bool immediate = false;
	Queue::iterator immediateIt;
	uintmax_t idCounter = 0;

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

	void setImmediate()
	{
		this->immediateIt = this->queue.begin();
		this->immediate = true;
	}

	void resetImmediate()
	{
		this->immediateIt = this->queue.end();
		this->immediate = false;
	}

	void pop()
	{
		this->queue.pop_front();
		this->resetImmediate();
	}

	// Returns true if waiting
	bool waiting(const WaitCommand& cmd)
	{
		static double base = 0.0;
		if (base == 0.0) base = Reader::now();
		bool res = Reader::now() < base + cmd.time;
		if (!res) base = 0.0;
		return res;
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
		Point<int> pos = mouse.pos;
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
	// Returns true/false based on if hotkey was available
	bool hotkeyOr(const char* hotkey, Point<int> fallback)
	{
		// Use if hotkey possible
		auto k = this->getHotkey(hotkey);

		if (k != Key::Unknown)
		{
			Input::keyPress(k, false);
			return true;
		}

		Input::mouseClick(MouseButton::Left, fallback, false);
		return false;
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

			// Then, fallback to positional unpower key
			int id = sys.uiBox + 1;
			k = this->getHotkey("un_power_" + std::to_string(id));
			if (k != Key::Unknown) return { k, false };

			// Then, fallback to direct power key + shift
			k = this->getHotkey(systemPowerHotkey(sys.type));
			if (k != Key::Unknown) return { k, true };

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
	void useCrewSingle(const Crew& crew, bool exclusive)
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
	void useCrewMany(const CrewRefList& crew, bool exclusive)
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
		if (state.game->pause.normal == on) return;

		// middle mouse is always available, no need to check for hotkey
		Input::mouseClick(MouseButton::Middle, { -1, -1 }, false);
	}

	void choice(int which)
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
				if (delay)
				{
					Input::wait(event->openTime.first);
					Input::choice(which);
					return;
				}

				throw InvalidEventChoice(which, "hotkeys are disabled and the choice is off-screen");
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
		auto&& [type, set, which] = power;

		this->checkInGame("powering a system");

		auto&& state = Reader::getState();
		auto&& system = state.game->playerShip->getSystem(type, which);

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

		if (upInputs && downInputs) return;
		this->deselect();

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
		this->deselect();

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
		this->deselect();

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

	void deselect(const DeselectCommand& cmd = {})
	{
		this->checkInGame("deselection");

		auto&& state = Reader::getState();

		if (cmd.left)
		{
			auto&& crew = state.game->playerCrew;
			bool anyCrew = false;
			for (auto&& c : crew)
			{
				if (c.selectionId != -1)
				{
					anyCrew = true;
					break;
				}
			}

			if (anyCrew) Input::mouseClick(MouseButton::Left, this->nopSpot(), false);
		}

		if (cmd.right)
		{
			bool aiming = !std::holds_alternative<std::monostate>(state.ui.mouse.aiming);
			if (aiming) Input::mouseClick(MouseButton::Right, this->nopSpot(), false);
		}
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

		this->checkInGame("selecting crew");

		auto&& state = Reader::getState();
		this->deselect({.left = false});

		if (crew.empty())
		{
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
		bool exclusive = true;

		for (size_t i = 0; i < crew.size(); i++)
		{
			// If not contiguous, input and clear group
			if (!group.empty() && prev != crew[i] - 1)
			{
				if (group.size() == 1) this->useCrewSingle(group[0], exclusive);
				else this->useCrewMany(group, exclusive);

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

	void swapWeapons(const SwapCommand& cmd)
	{
		constexpr char act[] = "swapping equipped weapons";

		auto&& state = Reader::getState();
		if (!state.game) throw GameNotRunning(act);
		if (state.game->pause.menu && !state.ui.game->cargo) throw WrongMenu(act);
		if (!state.game->playerShip->weapons) throw SystemNotInstalled(SystemType::Weapons);

		auto&& weapons = state.ui.game->cargo
			? state.ui.game->cargo->weapons
			: state.ui.game->weaponBoxes;

		auto&& [a, b] = cmd;
		if (a < 0 || a >= int(weapons.size()) || b < 0 || b >= int(weapons.size()))
		{
			throw InvalidSwap("weapon", a, "weapon", b);
		}

		bool aEmpty = a >= int(state.game->playerShip->weapons->list.size());
		bool bEmpty = b >= int(state.game->playerShip->weapons->list.size());
		if (aEmpty && bEmpty)
		{
			throw InvalidSwap("weapon", a, "weapon", b);
		}

		if (a == b) return; // do nothing, no swap needed

		Input::mouseDown(MouseButton::Left, weapons[a].center());
		Input::mouseUp(MouseButton::Left, weapons[b].center());
	}

	void swapDrones(const SwapCommand& cmd)
	{
		constexpr char act[] = "swapping equipped drones";

		auto&& state = Reader::getState();
		if (!state.game) throw GameNotRunning(act);
		if (state.game->pause.menu && !state.ui.game->cargo) throw WrongMenu(act);
		if (!state.game->playerShip->drones) throw SystemNotInstalled(SystemType::Drones);

		auto&& drones = state.ui.game->cargo
			? state.ui.game->cargo->drones
			: state.ui.game->droneBoxes;

		auto&& [a, b] = cmd;
		if (a < 0 || a >= int(drones.size()) || b < 0 || b >= int(drones.size()))
		{
			throw InvalidSwap("drone", a, "drone", b);
		}

		bool aEmpty = a >= int(state.game->playerShip->drones->list.size());
		bool bEmpty = b >= int(state.game->playerShip->drones->list.size());
		if (aEmpty && bEmpty)
		{
			throw InvalidSwap("drone", a, "drone", b);
		}

		if (a == b) return; // do nothing, no swap needed

		Input::mouseDown(MouseButton::Left, drones[a].center());
		Input::mouseUp(MouseButton::Left, drones[b].center());
	}

	void crewAbility()
	{
		this->checkInGame("using crew abilities");
		auto&& state = Reader::getState();
		
		// Try to use a hotkey
		auto hotkey = getHotkey("lockdown");

		if (hotkey != Key::Unknown)
		{
			Input::keyDown(hotkey);
			return;
		}
		
		// If no hotkey, we need to click on all crew buttons individually
		auto&& crew = state.game->playerCrew;

		for (auto&& c : crew)
		{
			if (c.selectionId != -1)
			{
				auto&& box = state.ui.game->crewBoxes[c.id];

				if (!box.power) continue;

				Input::mouseMove(box.box.center());
				Input::mouseClick(MouseButton::Left, box.power->center());
			}
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

	void teleportSend()
	{
		this->checkInGame("aiming teleport send");
		auto&& state = Reader::getState();

		if (!state.game->playerShip->teleporter)
		{
			throw SystemNotInstalled(SystemType::Teleporter);
		}

		auto&& tele = *state.game->playerShip->teleporter;
		if (!tele.operable()) throw SystemInoperable(tele);
		if (!tele.canSend) throw SystemInoperable(tele, "it cannot send crew right now");

		this->deselect();
		this->hotkeyOr("send_tele", state.ui.game->teleportSend->center());
	}

	void teleportReturn()
	{
		this->checkInGame("aiming teleport return");
		auto&& state = Reader::getState();

		if (!state.game->playerShip->teleporter)
		{
			throw SystemNotInstalled(SystemType::Teleporter);
		}

		auto&& tele = *state.game->playerShip->teleporter;
		if (!tele.operable()) throw SystemInoperable(tele);
		if (!tele.canReceive) throw SystemInoperable(tele, "it cannot receive crew right now");

		this->deselect();
		this->hotkeyOr("ret_tele", state.ui.game->teleportSend->center());
	}

	void cloak()
	{
		this->checkInGame("cloaking");
		auto&& state = Reader::getState();

		if (!state.game->playerShip->cloaking)
		{
			throw SystemNotInstalled(SystemType::Cloaking);
		}

		auto&& cloak = *state.game->playerShip->cloaking;
		if (!cloak.operable()) throw SystemInoperable(cloak);

		this->deselect();
		this->hotkeyOr("activate_cloak", state.ui.game->startCloak->center());
	}

	void battery()
	{
		this->checkInGame("activating battery");
		auto&& state = Reader::getState();

		if (!state.game->playerShip->battery)
		{
			throw SystemNotInstalled(SystemType::Battery);
		}

		auto&& battery = *state.game->playerShip->battery;
		if (!battery.operable()) throw SystemInoperable(battery);

		this->deselect();
		this->hotkeyOr("activate_battery", state.ui.game->startBattery->center());
	}

	void mindControl()
	{
		this->checkInGame("activating mind control");
		auto&& state = Reader::getState();

		if (!state.game->playerShip->mindControl)
		{
			throw SystemNotInstalled(SystemType::MindControl);
		}

		auto&& mindControl = *state.game->playerShip->mindControl;
		if (!mindControl.operable()) throw SystemInoperable(mindControl);

		this->deselect();
		this->hotkeyOr("mindControl", state.ui.game->startMindControl->center());
	}

	void setupHack()
	{
		this->checkInGame("setting up hacking");
		auto&& state = Reader::getState();

		if (!state.game->playerShip->hacking)
		{
			throw SystemNotInstalled(SystemType::Hacking);
		}

		auto&& hacking = *state.game->playerShip->hacking;
		if (!hacking.operable()) throw SystemInoperable(hacking);

		if (hacking.drone.setUp)
		{
			throw InvalidHackingInput(false, "the drone is already set up");
		}

		this->deselect();
		this->hotkeyOr("start_hacking", state.ui.game->startHack->center());
	}

	void hack()
	{
		this->checkInGame("hacking");
		auto&& state = Reader::getState();

		if (!state.game->playerShip->hacking)
		{
			throw SystemNotInstalled(SystemType::Hacking);
		}

		auto&& hacking = *state.game->playerShip->hacking;
		if (!hacking.operable()) throw SystemInoperable(hacking);

		if (!hacking.drone.setUp)
		{
			throw InvalidHackingInput(false, "the drone is not set up");
		}

		this->hotkeyOr("start_hacking", state.ui.game->startHack->center());
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

		if (!doors->operable()) throw DoorInoperable(door, "the door system is inoperable");
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
		if (!doorControl->operable()) throw SystemInoperable(*doorControl);

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
		if (!doorControl->operable()) throw SystemInoperable(*doorControl);

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

	void sendCrew(const SendCrewCommand& cmd)
	{
		auto&& [id, self] = cmd;

		this->checkInGame("sending crew to a room");
		auto&& state = Reader::getState();

		if (self && size_t(id) >= state.game->playerShip->rooms.size())
		{
			throw std::range_error("room has invalid id");
		}

		auto&& enemy = state.game->enemyShip;
		if (!self && (!enemy || size_t(id) >= enemy->rooms.size()))
		{
			throw std::range_error("enemy room has invalid id");
		}

		const Room& room = self
			? state.game->playerShip->rooms[id]
			: state.game->enemyShip->rooms[id];

		bool anyCrew = false;
		for (auto&& c : state.game->playerCrew)
		{
			if (c.selectionId >= 0)
			{
				anyCrew = true;
				break;
			}

			if (c.onPlayerShip != self)
			{
				throw InvalidCrewChoice(c, "they're on the wrong ship");
			}
		}

		if (!anyCrew) throw NotSelected("crewmember");

		Input::mouseClick(MouseButton::Right, room.rect.center(), false);
	}

	// to: nullopt - inherit, true - on, false - off
	// relase: undoes inversion if already done
	void invertAutofire(std::optional<bool> to, bool release)
	{
		this->checkInGame("inverting autofire");
		auto&& state = Reader::getState();

		if (!to) return;

		if (!state.ui.game->autofire)
		{
			throw SystemNotInstalled(SystemType::Weapons);
		}

		// Use if hotkey possible
		auto k = this->getHotkey("force_autofire");

		if (k != Key::Unknown)
		{
			if (release) Input::keyUp(k, false);
			else Input::keyDown(k, false);
			return;
		}

		Input::mouseClick(MouseButton::Left, state.ui.game->autofire->center(), false);
	}

	void aim(const AimCommand& aim)
	{
		auto&& id = aim.room;
		auto&& self = aim.self;
		auto&& autofire = aim.autofire;

		this->checkInGame("aiming weapon/system");
		auto&& state = Reader::getState();

		if (self && size_t(id) >= state.game->playerShip->rooms.size())
		{
			throw std::range_error("room has invalid id");
		}

		auto&& enemy = state.game->enemyShip;
		if (!self && (!enemy || size_t(id) >= enemy->rooms.size()))
		{
			throw std::range_error("enemy room has invalid id");
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

			if (system.type == SystemType::MindControl)
			{
				if (!room.mindControllable())
				{
					throw InvalidSelfAim("mind control requires a visible room with intruding crew");
				}
			}
			else if (room.player)
			{
				throw InvalidSelfAim("only mind control can do that");
			}

			if (autofire)
			{
				throw std::invalid_argument("cannot specify autofire for a system");
			}
		}

		bool aimingWeapon = false;
		if (std::holds_alternative<WeaponUIRef>(state.ui.mouse.aiming))
		{
			auto&& weapon = std::get<WeaponUIRef>(state.ui.mouse.aiming).get();

			if (room.player && weapon.blueprint.type != WeaponType::Bomb)
			{
				throw InvalidSelfAim("only bomb weapons can do that");
			}

			aimingWeapon = true;
		}

		if (aimingWeapon) this->invertAutofire(autofire, false);
		Input::mouseClick(MouseButton::Left, room.rect.center(), false);
		if (aimingWeapon) this->invertAutofire(autofire, true);
	}

	void aimBeam(const AimCommand& aim)
	{
		auto&& [id, self, autofire, start, end] = aim;

		this->checkInGame("aiming beam");
		auto&& state = Reader::getState();

		if (self && size_t(id) >= state.game->playerShip->rooms.size())
		{
			throw std::range_error("room has invalid id");
		}

		auto&& enemy = state.game->enemyShip;
		if (!self && (!enemy || size_t(id) >= enemy->rooms.size()))
		{
			throw std::range_error("enemy room has invalid id");
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

		this->invertAutofire(autofire, false);
		Input::mouseClick(MouseButton::Left, room.rect.topLeft() + start, false);
		Input::mouseClick(MouseButton::Left, room.rect.topLeft() + end, false);
		this->invertAutofire(autofire, true);
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

	void jump()
	{
		this->checkInGame("opening jump menu");
		auto&& state = Reader::getState();

		if (!state.game->playerShip->canJump)
		{
			throw MenuNotAvailable("jump menu", "player can't currently jump");
		}

		this->hotkeyOr("jump", state.ui.game->ftl.center());
	}

	void leaveCrew(bool confirm)
	{
		auto&& state = Reader::getState();
		constexpr char act[] = "choosing to leave crew behind";

		if (!state.game) throw GameNotRunning(act);
		if (!state.ui.game->leaveCrew) throw WrongMenu(act);

		auto&& yes = state.ui.game->leaveCrew->yes;
		auto&& no = state.ui.game->leaveCrew->no;

		Input::mouseClick(MouseButton::Left, confirm ? yes.center() : no.center());
	}

	void upgrades()
	{
		constexpr char act[] = "opening upgrades menu";

		auto&& state = Reader::getState();
		if (!state.game) throw GameNotRunning(act);

		if (!state.game->playerShip->canInventory)
		{
			throw MenuNotAvailable("upgrades menu", "the player is in danger");
		}

		auto&& tab = state.ui.game->upgradesTab;
		if (state.game->pause.menu && !tab) throw WrongMenu(act);

		// Already open, nothing to do
		if (state.ui.game->upgrades) return;

		if (tab)
		{
			// Ship screens already open, click tab
			Input::mouseClick(MouseButton::Left, tab->center());
			return;
		}

		// Otherwise, click button
		// Default tab will be upgrades
		this->hotkeyOr("ship_info", state.ui.game->shipButton.center());
	}

	void crewManifest()
	{
		constexpr char act[] = "opening crew manifest";

		auto&& state = Reader::getState();
		if (!state.game) throw GameNotRunning(act);

		if (!state.game->playerShip->canInventory)
		{
			throw MenuNotAvailable("upgrades menu", "the player is in danger");
		}

		auto&& tab = state.ui.game->crewTab;
		if (state.game->pause.menu && !tab) throw WrongMenu(act);

		// Already open, nothing to do
		if (state.ui.game->crewMenu) return;

		if (tab)
		{
			// Ship screens already open, click tab
			Input::mouseClick(MouseButton::Left, tab->center());
			return;
		}

		// Try to directly hotkey to it
		bool direct = this->hotkeyOr("ship_crew", state.ui.game->shipButton.center());

		// Otherwise, re-queue this command to click the tab ones the upgrades menu is open
		if (!direct)
		{
			this->push(Command{
				 .type = Command::Type::CrewManifest
			});
		}
	}

	void cargo()
	{
		constexpr char act[] = "opening cargo";

		auto&& state = Reader::getState();
		if (!state.game) throw GameNotRunning(act);

		if (!state.game->playerShip->canInventory)
		{
			throw MenuNotAvailable("upgrades menu", "the player is in danger");
		}

		auto&& tab = state.ui.game->cargoTab;
		if (state.game->pause.menu && !tab) throw WrongMenu(act);

		// Already open, nothing to do
		if (state.ui.game->cargo) return;

		if (tab)
		{
			// Ship screens already open, click tab
			Input::mouseClick(MouseButton::Left, tab->center());
			return;
		}

		// Try to directly hotkey to it
		bool direct = this->hotkeyOr("ship_inv", state.ui.game->shipButton.center());

		// Otherwise, re-queue this command to click the tab ones the upgrades menu is open
		if (!direct)
		{
			this->push(Command{
				 .type = Command::Type::Cargo
			});
		}
	}

	void store()
	{
		constexpr char act[] = "opening store menu";

		auto&& state = Reader::getState();
		if (!state.game) throw GameNotRunning(act);

		if (!state.game->playerShip->canInventory)
		{
			throw MenuNotAvailable("upgrades menu", "the player is in danger");
		}

		auto&& button = state.ui.game->storeButton;
		auto&& store = state.ui.game->store;

		if (state.game->pause.menu) throw WrongMenu(act);
		if (!button || !store) throw NoStore();

		this->hotkeyOr("store", button->center());
	}

	void menu()
	{
		constexpr char act[] = "opening pause menu";

		auto&& state = Reader::getState();
		if (!state.game) throw GameNotRunning(act);
		auto&& menu = state.ui.game->menu;
		if (state.game->pause.menu && !menu) throw WrongMenu(act);

		Input::keyPress(Key::Escape);
	}

	void upgradeSystem(const UpgradeSystemCommand& cmd)
	{
		constexpr char act[] = "upgrading a system";

		auto&& [sys, desired, which] = cmd;

		auto&& state = Reader::getState();
		if (!state.game) throw GameNotRunning(act);
		auto&& up = state.ui.game->upgrades;
		if (!up) throw WrongMenu(act);

		auto&& actualSys = state.game->playerShip->getSystem(sys, which);
		int max = actualSys.level.second;
		auto&& box = up->getSystem(sys, which);
		auto&& [from, to] = box.upgrade;

		if (desired < from) throw InvalidUpgrade(sys, from, desired);
		if (desired > max) throw InvalidUpgrade(sys, from, desired, max);

		int diff = desired - to;

		if (diff < 0)
		{
			for (int i = 0; i > diff; i--)
			{
				Input::mouseClick(MouseButton::Right, box.box.center());
			}
		}
		else
		{
			int sum = 0;
			for (int i = to + 1; i <= desired; i++)
			{
				sum += actualSys.blueprint.upgradeCosts[i];
			}

			int scrap = state.game->playerShip->cargo.scrap;
			if (sum > scrap)
			{
				throw CannotAfford(systemName(sys) + " upgrade", scrap, sum);
			}

			for (int i = 0; i < diff; i++)
			{
				Input::mouseClick(MouseButton::Left, box.box.center());
			}
		}
	}

	void upgradeReactor(const UpgradeReactorCommand& cmd)
	{
		constexpr char act[] = "upgrading the reactor";

		auto&& [desired] = cmd;

		auto&& state = Reader::getState();
		if (!state.game) throw GameNotRunning(act);
		auto&& up = state.ui.game->upgrades;
		if (!up) throw WrongMenu(act);

		int max = state.game->playerShip->reactor.level.second;
		auto&& box = up->reactor;
		auto&& [from, to] = box.upgrade;

		if (desired < from) throw InvalidUpgrade(from, desired);
		if (desired > max) throw InvalidUpgrade(from, desired, max);

		int diff = desired - to;

		if (diff < 0)
		{
			for (int i = 0; i > diff; i--)
			{
				Input::mouseClick(MouseButton::Right, box.box.center());
			}
		}
		else
		{
			int sum = 0;
			for (int i = to + 1; i <= desired; i++)
			{
				sum += Reactor::HARDCODED_UPGRADE_COSTS[i];
			}

			int scrap = state.game->playerShip->cargo.scrap;
			if (sum > scrap)
			{
				throw CannotAfford("reactor upgrade", scrap, sum);
			}

			for (int i = 0; i < diff; i++)
			{
				Input::mouseClick(MouseButton::Left, box.box.center());
			}
		}
	}

	void undoUpgrades()
	{
		constexpr char act[] = "undoing upgrades";

		auto&& state = Reader::getState();
		if (!state.game) throw GameNotRunning(act);
		auto&& up = state.ui.game->upgrades;
		if (!up) throw WrongMenu(act);

		Input::mouseClick(MouseButton::Left, up->undo.center());
	}

	void renameCrew(const RenameCrewCommand& cmd)
	{
		constexpr char act[] = "renaming crew";

		auto&& state = Reader::getState();
		if (!state.game) throw GameNotRunning(act);
		auto&& crew = state.ui.game->crewMenu;
		if (!crew) throw WrongMenu(act);
		auto&& confirm = crew->confirm;
		if (confirm) throw WrongMenu(act);

		auto&& [which, name] = cmd;

		if (name.size() > CrewBlueprint::HARDCODED_MAX_NAME_LENGTH)
		{
			throw std::out_of_range("desired crewmember name is too long");
		}

		auto count = int(crew->boxes.size());

		if (which < 0) throw InvalidCrewBoxChoice(which);
		if (which >= count) throw InvalidCrewBoxChoice(which, count);

		auto&& rename = crew->boxes[which].rename;
		Input::mouseClick(MouseButton::Left, rename.center());
		Input::textClear();
		Input::text(name);
		Input::textConfirm();
	}

	void dismissCrew(const DiscardCommand& cmd)
	{
		constexpr char act[] = "dismissing crew";

		auto&& state = Reader::getState();
		if (!state.game) throw GameNotRunning(act);
		auto&& crew = state.ui.game->crewMenu;
		if (!crew) throw WrongMenu(act);
		auto&& confirm = crew->confirm;
		if (confirm) throw WrongMenu(act);

		auto&& [which] = cmd;
		auto count = int(crew->boxes.size());

		if (which < 0) throw InvalidCrewBoxChoice(which);
		if (which >= count) throw InvalidCrewBoxChoice(which, count);
		if (count < 2) throw std::runtime_error("need at least 2 crew to be able to dismiss any");

		auto&& dismiss = crew->boxes[which].dismiss;
		Input::mouseClick(MouseButton::Left, dismiss.center());
	}

	void confirmDismissCrew(bool yes)
	{
		constexpr char act[] = "confirming crew dismissal";

		auto&& state = Reader::getState();
		if (!state.game) throw GameNotRunning(act);
		auto&& crew = state.ui.game->crewMenu;
		if (!crew) throw WrongMenu(act);
		auto&& confirm = crew->confirm;
		if (!confirm) throw WrongMenu(act);

		if(yes) Input::mouseClick(MouseButton::Left, confirm->yes.center());
		else Input::mouseClick(MouseButton::Left, confirm->no.center());
	}

	void swapCargo(const SwapCommand& cmd)
	{
		constexpr char act[] = "swapping cargo";

		auto&& state = Reader::getState();
		if (!state.game) throw GameNotRunning(act);
		auto&& cargo = state.ui.game->cargo;
		if (!cargo) throw WrongMenu(act);

		auto&& [a, b] = cmd;
		auto&& storage = state.game->playerShip->cargo.storage;

		auto storageCount = int(storage.size());
		if (a < 0) throw InvalidSlotChoice("storage slot", a);
		if (a >= storageCount) throw InvalidSlotChoice("storage slot", a, storageCount);

		if (b < 0) throw InvalidSlotChoice("storage slot", b);
		if (a >= storageCount) throw InvalidSlotChoice("storage slot", b, storageCount);

		bool aEmpty = std::holds_alternative<std::monostate>(storage[a]);
		bool bEmpty = std::holds_alternative<std::monostate>(storage[b]);

		// Check if swapping two empty slots
		if (aEmpty && bEmpty)
		{
			throw InvalidSwap("cargo", a, "cargo", b);
		}

		Point<int> first, second;

		if (aEmpty)
		{
			first = cargo->storage[b].center();
			second = cargo->storage[a].center();
		}
		else
		{
			first = cargo->storage[a].center();
			second = cargo->storage[b].center();
		}

		Input::mouseDown(MouseButton::Left, first);
		Input::mouseUp(MouseButton::Left, second);
	}

	void swapWeaponCargo(const SwapCommand& cmd)
	{
		constexpr char act[] = "swapping weapons and cargo";

		auto&& state = Reader::getState();
		if (!state.game) throw GameNotRunning(act);
		auto&& cargo = state.ui.game->cargo;
		if (!cargo) throw WrongMenu(act);

		auto&& [a, b] = cmd;
		auto&& weapons = state.game->playerShip->weapons;
		auto&& storage = state.game->playerShip->cargo.storage;

		if (!weapons) throw SystemNotInstalled(SystemType::Weapons);

		auto weaponCount = int(weapons->slotCount);
		if (a < 0) throw InvalidSlotChoice("weapon", a);
		if (a >= weaponCount) throw InvalidSlotChoice("weapon", a, weaponCount);

		auto storageCount = int(storage.size());
		if (b < 0) throw InvalidSlotChoice("storage slot", b);
		if (a >= storageCount) throw InvalidSlotChoice("storage slot", b, storageCount);

		// Check if swapping two empty slots
		bool aEmpty = a >= int(weapons->list.size());
		bool bEmpty = std::holds_alternative<std::monostate>(storage[b]);
		if (aEmpty && bEmpty)
		{
			throw InvalidSwap("weapon", a, "cargo", b);
		}

		Point<int> first, second;

		if (aEmpty)
		{
			first = cargo->storage[b].center();
			second = cargo->weapons[a].center();
		}
		else
		{
			first = cargo->weapons[a].center();
			second = cargo->storage[b].center();
		}

		Input::mouseDown(MouseButton::Left, first);
		Input::mouseUp(MouseButton::Left, second);
	}

	void swapDroneCargo(const SwapCommand& cmd)
	{
		constexpr char act[] = "swapping drones and cargo";

		auto&& state = Reader::getState();
		if (!state.game) throw GameNotRunning(act);
		auto&& cargo = state.ui.game->cargo;
		if (!cargo) throw WrongMenu(act);

		auto&& [a, b] = cmd;
		auto&& drones = state.game->playerShip->drones;
		auto&& storage = state.game->playerShip->cargo.storage;

		if (!drones) throw SystemNotInstalled(SystemType::Drones);

		auto droneCount = int(drones->slotCount);
		if (a < 0) throw InvalidSlotChoice("drone", a);
		if (a >= droneCount) throw InvalidSlotChoice("drone", a, droneCount);

		auto storageCount = int(storage.size());
		if (b < 0) throw InvalidSlotChoice("storage slot", b);
		if (a >= storageCount) throw InvalidSlotChoice("storage slot", b, storageCount);

		// Check if swapping two empty slots
		bool aEmpty = a >= int(drones->list.size());
		bool bEmpty = std::holds_alternative<std::monostate>(storage[b]);
		if (aEmpty && bEmpty)
		{
			throw InvalidSwap("weapon", a, "cargo", b);
		}

		Point<int> first, second;

		if (aEmpty)
		{
			first = cargo->storage[b].center();
			second = cargo->drones[a].center();
		}
		else
		{
			first = cargo->drones[a].center();
			second = cargo->storage[b].center();
		}

		Input::mouseDown(MouseButton::Left, first);
		Input::mouseUp(MouseButton::Left, second);
	}

	void discardCargo(const DiscardCommand& cmd)
	{
		constexpr char act[] = "discarding cargo";

		auto&& state = Reader::getState();
		if (!state.game) throw GameNotRunning(act);
		auto&& cargo = state.ui.game->cargo;
		if (!cargo) throw WrongMenu(act);
		auto&& box = cargo->discard;
		if (!box) throw WrongMenu(act);

		auto&& [which] = cmd;
		auto&& storage = state.game->playerShip->cargo.storage;

		int storageCount = int(storage.size());
		if (which < 0) throw InvalidSlotChoice("storage slot", which);
		if (which >= storageCount) throw InvalidSlotChoice("storage slot", which, storageCount);

		bool empty = std::holds_alternative<std::monostate>(storage[which]);
		if (empty) throw InvalidSlotChoice("cargo", which);

		Input::mouseDown(MouseButton::Left, cargo->storage[which].center());
		Input::mouseUp(MouseButton::Left, box->center());
	}

	void discardWeapon(const DiscardCommand& cmd)
	{
		constexpr char act[] = "discarding weapon";

		auto&& state = Reader::getState();
		if (!state.game) throw GameNotRunning(act);
		auto&& cargo = state.ui.game->cargo;
		if (!cargo) throw WrongMenu(act);
		auto&& box = cargo->discard;
		if (!box) throw WrongMenu(act);

		auto&& [which] = cmd;
		auto&& weapons = state.game->playerShip->weapons;

		if (!weapons) throw SystemNotInstalled(SystemType::Weapons);

		int weaponCount = int(weapons->list.size());
		if (which < 0) throw InvalidSlotChoice("weapon", which);
		if (which >= weaponCount) throw InvalidSlotChoice("weapon", which, weaponCount);

		Input::mouseDown(MouseButton::Left, cargo->weapons[which].center());
		Input::mouseUp(MouseButton::Left, box->center());
	}

	void discardDrone(const DiscardCommand& cmd)
	{
		constexpr char act[] = "discarding drone";

		auto&& state = Reader::getState();
		if (!state.game) throw GameNotRunning(act);
		auto&& cargo = state.ui.game->cargo;
		if (!cargo) throw WrongMenu(act);
		auto&& box = cargo->discard;
		if (!box) throw WrongMenu(act);

		auto&& [which] = cmd;
		auto&& drones = state.game->playerShip->drones;

		if (!drones) throw SystemNotInstalled(SystemType::Drones);

		int droneCount = int(drones->list.size());
		if (which < 0) throw InvalidSlotChoice("drone", which);
		if (which >= droneCount) throw InvalidSlotChoice("drone", which, droneCount);

		Input::mouseDown(MouseButton::Left, cargo->drones[which].center());
		Input::mouseUp(MouseButton::Left, box->center());
	}

	void discardAugment(const DiscardCommand& cmd)
	{
		constexpr char act[] = "discarding augment";

		auto&& state = Reader::getState();
		if (!state.game) throw GameNotRunning(act);
		auto&& cargo = state.ui.game->cargo;
		if (!cargo) throw WrongMenu(act);
		auto&& box = cargo->discard;
		if (!box) throw WrongMenu(act);

		auto&& [which] = cmd;
		auto&& augments = state.game->playerShip->cargo.augments;

		int augmentCount = int(augments.size());
		if (which < 0) throw InvalidSlotChoice("augment", which);
		if (which >= augmentCount) throw InvalidSlotChoice("augment", which, augmentCount);

		Input::mouseDown(MouseButton::Left, cargo->augments[which].center());
		Input::mouseUp(MouseButton::Left, box->center());
	}
};

Input::Impl Input::impl;
bool Input::good = false;
bool Input::humanMouse = true;
bool Input::humanKeyboard = true;

void Input::iterate()
{
	impl.iterate();
}

bool Input::empty()
{
	return impl.empty();
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

Input::Ret Input::dummy()
{
	return {};
}

Input::Ret Input::wait(double time)
{
	return impl.push({
		.type = Command::Type::Wait,
		.args = WaitCommand{
			.time = time
		},
	});
}

Input::Ret Input::mouseMove(const Point<int>& pos)
{
	return impl.push({
		.type = Command::Type::Mouse,
		.args = MouseCommand{
			.button = MouseButton::None,
			.pos = pos
		},
	});
}

Input::Ret Input::mouseDown(
	MouseButton button,
	const Point<int>& pos,
	bool shift)
{
	mouseMove(pos);
	return impl.push({
		.type = Command::Type::Mouse,
		.args = MouseCommand{
			.button = button,
			.pos = pos,
			.shift = shift,
			.direction = InputDirection::Down
		}
	});
}

Input::Ret Input::mouseUp(
	MouseButton button,
	const Point<int>& pos,
	bool shift)
{
	mouseMove(pos);
	return impl.push({
		.type = Command::Type::Mouse,
		.args = MouseCommand{
			.button = button,
			.pos = pos,
			.shift = shift,
			.direction = InputDirection::Up
		}
	});
}

Input::Ret Input::mouseClick(
	MouseButton button,
	const Point<int>& pos,
	bool shift)
{
	mouseMove(pos);

	impl.push({
		.type = Command::Type::Mouse,
		.args = MouseCommand{
			.button = button,
			.pos = pos,
			.shift = shift,
			.direction = InputDirection::Down
		}
	});

	return impl.push({
		.type = Command::Type::Mouse,
		.args = MouseCommand{
			.button = button,
			.pos = pos,
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
		.type = Command::Type::Keyboard,
		.args = KeyboardCommand{
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
		.type = Command::Type::Keyboard,
		.args = KeyboardCommand{
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
		.type = Command::Type::TextInput,
		.args = ch
	});
}

Input::Ret Input::text(const std::string& str)
{
	Input::Ret last{};
	for (auto&& c : str) last = text(c);
	return last;
}

Input::Ret Input::textConfirm()
{
	return impl.push({
		.type = Command::Type::TextEvent,
		.args = raw::TEXT_CONFIRM
	});
}

Input::Ret Input::textClear()
{
	return impl.push({
		.type = Command::Type::TextEvent,
		.args = raw::TEXT_CLEAR
	});
}

Input::Ret Input::textBackspace()
{
	return impl.push({
		.type = Command::Type::TextEvent,
		.args = raw::TEXT_BACKSPACE
	});
}

Input::Ret Input::textDelete()
{
	return impl.push({
		.type = Command::Type::TextEvent,
		.args = raw::TEXT_DELETE
	});
}

Input::Ret Input::textLeft()
{
	return impl.push({
		.type = Command::Type::TextEvent,
		.args = raw::TEXT_LEFT
	});
}

Input::Ret Input::textRight()
{
	return impl.push({
		.type = Command::Type::TextEvent,
		.args = raw::TEXT_RIGHT
	});
}

Input::Ret Input::textHome()
{
	return impl.push({
		.type = Command::Type::TextEvent,
		.args = raw::TEXT_HOME
	});
}

Input::Ret Input::textEnd()
{
	return impl.push({
		.type = Command::Type::TextEvent,
		.args = raw::TEXT_END
	});
}

Input::Ret Input::cheat(const std::string& command)
{
	return impl.push({
		.type = Command::Type::Cheat,
		.args = CheatCommand{
			.command = command
		}
	});
}

Input::Ret Input::pause(bool on)
{
	return impl.push({
		.type = Command::Type::Pause,
		.args = on
	});
}

Input::Ret Input::choice(int which)
{
	return impl.push({
		.type = Command::Type::EventChoice,
		.args = which
	});
}

Input::Ret Input::powerSystem(SystemType system, int set, int which)
{
	return impl.push({
		.type = Command::Type::PowerSystem,
		.args = PowerCommand{
			.system = system,
			.set = set,
			.which = which
		}
	});
}

Input::Ret Input::powerWeapon(int weapon, bool on)
{
	return impl.push({
		.type = Command::Type::PowerWeapon,
		.args = WeaponCommand{
			.weapon = weapon,
			.on = on
		}
	});
}

Input::Ret Input::powerDrone(int drone, bool on)
{
	return impl.push({
		.type = Command::Type::PowerDrone,
		.args = DroneCommand{
			.drone = drone,
			.on = on
		}
	});
}

Input::Ret Input::selectWeapon(int weapon)
{
	return impl.push({
		.type = Command::Type::SelectWeapon,
		.args = WeaponCommand{
			.weapon = weapon
		}
	});
}

Input::Ret Input::selectCrew(const std::vector<int>& crew)
{
	return impl.push({
		.type = Command::Type::SelectCrew,
		.args = CrewSelectionCommand{
			.crew = crew
		}
	});
}

Input::Ret Input::swapWeapons(int slotA, int slotB)
{
	return impl.push({
		.type = Command::Type::SwapWeapons,
		.args = SwapCommand{
			.slotA = slotA,
			.slotB = slotB
		}
	});
}

Input::Ret Input::swapDrones(int slotA, int slotB)
{
	return impl.push({
		.type = Command::Type::SwapDrones,
		.args = SwapCommand{
			.slotA = slotA,
			.slotB = slotB
		}
	});
}

Input::Ret Input::crewAbility()
{
	return impl.push({
		.type = Command::Type::CrewAbility
	});
}

Input::Ret Input::autofire(bool on)
{
	return impl.push({
		.type = Command::Type::Autofire,
		.args = on
	});
}

Input::Ret Input::teleportSend()
{
	return impl.push({
		.type = Command::Type::TeleportSend
	});
}

Input::Ret Input::teleportReturn()
{
	return impl.push({
		.type = Command::Type::TeleportReturn
	});
}

Input::Ret Input::cloak()
{
	return impl.push({
		.type = Command::Type::Cloak
	});
}

Input::Ret Input::battery()
{
	return impl.push({
		.type = Command::Type::Battery
	});
}

Input::Ret Input::mindControl()
{
	return impl.push({
		.type = Command::Type::MindControl
	});
}

Input::Ret Input::setupHack()
{
	return impl.push({
		.type = Command::Type::SetupHack
	});
}

Input::Ret Input::hack()
{
	return impl.push({
		.type = Command::Type::Hack
	});
}

Input::Ret Input::door(int door, bool open)
{
	return impl.push({
		.type = Command::Type::Door,
		.args = DoorCommand{
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
			.type = Command::Type::OpenAllDoors,
			.args = airlocks
		});
	}

	return impl.push({
		.type = Command::Type::CloseAllDoors,
	});
}

Input::Ret Input::aim(
	int room, bool self,
	std::optional<bool> autofire)
{
	return impl.push({
		.type = Command::Type::Aim,
		.args = AimCommand{
			.room = room,
			.self = self,
			.autofire = autofire
		}
	});
}

Input::Ret Input::aim(
	int room,
	const Point<int>& start,
	const Point<int>& end,
	std::optional<bool> autofire)
{
	return impl.push({
		.type = Command::Type::AimBeam,
		.args = AimCommand{
			.room = room,
			.self = false,
			.autofire = autofire,
			.start = start,
			.end = end
		}
	});
}

Input::Ret Input::deselect()
{
	return impl.push({
		.type = Command::Type::Deselect,
		.args = DeselectCommand{
			.left = true,
			.right = true
		}
	});
}

Input::Ret Input::sendCrew(int room, bool self)
{
	return impl.push({
		.type = Command::Type::SendCrew,
		.args = SendCrewCommand{
			.room = room,
			.self = self
		}
	});
}

Input::Ret Input::saveStations()
{
	return impl.push({
		.type = Command::Type::SaveStations
	});
}

Input::Ret Input::loadStations()
{
	return impl.push({
		.type = Command::Type::LoadStations
	});
}

Input::Ret Input::jump()
{
	return impl.push({
		.type = Command::Type::Jump
	});
}

Input::Ret Input::leaveCrew(bool yes)
{
	return impl.push({
		.type = Command::Type::LeaveCrew,
		.args = yes
	});
}

Input::Ret Input::upgrades()
{
	return impl.push({
		.type = Command::Type::Upgrades
	});
}

Input::Ret Input::crewManifest()
{
	return impl.push({
		.type = Command::Type::CrewManifest
	});
}

Input::Ret Input::cargo()
{
	return impl.push({
		.type = Command::Type::Cargo
	});
}

Input::Ret Input::store()
{
	return impl.push({
		.type = Command::Type::Store
	});
}

Input::Ret Input::menu()
{
	return impl.push({
		.type = Command::Type::Menu
	});
}

Input::Ret Input::upgradeSystem(SystemType system, int to, int which)
{
	return impl.push({
		.type = Command::Type::UpgradeSystem,
		.args = UpgradeSystemCommand{
			.system = system,
			.to = to,
			.which = which
		}
	});
}

Input::Ret Input::upgradeReactor(int to)
{
	return impl.push({
		.type = Command::Type::UpgradeReactor,
		.args = UpgradeReactorCommand{
			.to = to
		}
	});
}

Input::Ret Input::undoUpgrades()
{
	return impl.push({
		.type = Command::Type::UndoUpgrades
	});
}

Input::Ret Input::renameCrew(int which, const std::string& name)
{
	return impl.push({
		.type = Command::Type::RenameCrew,
		.args = RenameCrewCommand{
			.which = which,
			.name = name
		}
	});
}

Input::Ret Input::dismissCrew(int which)
{
	return impl.push({
		.type = Command::Type::DismissCrew,
		.args = DiscardCommand{
			.which = which
		}
	});
}

Input::Ret Input::confirmDismissCrew(bool yes)
{
	return impl.push({
		.type = Command::Type::ConfirmDismissCrew,
		.args = yes
	});
}

Input::Ret Input::swapCargo(int slotA, int slotB)
{
	return impl.push({
		.type = Command::Type::SwapCargo,
		.args = SwapCommand{
			.slotA = slotA,
			.slotB = slotB
		}
	});
}

Input::Ret Input::swapWeaponCargo(int weaponSlot, int cargoSlot)
{
	return impl.push({
		.type = Command::Type::SwapWeaponCargo,
		.args = SwapCommand{
			.slotA = weaponSlot,
			.slotB = cargoSlot
		}
	});
}

Input::Ret Input::swapDroneCargo(int droneSlot, int cargoSlot)
{
	return impl.push({
		.type = Command::Type::SwapDroneCargo,
		.args = SwapCommand{
			.slotA = droneSlot,
			.slotB = cargoSlot
		}
	});
}

Input::Ret Input::discardCargo(int slot)
{
	return impl.push({
		.type = Command::Type::DiscardCargo,
		.args = DiscardCommand{
			.which = slot
		}
	});
}

Input::Ret Input::discardWeapon(int slot)
{
	return impl.push({
		.type = Command::Type::DiscardWeapon,
		.args = DiscardCommand{
			.which = slot
		}
	});
}

Input::Ret Input::discardDrone(int slot)
{
	return impl.push({
		.type = Command::Type::DiscardDrone,
		.args = DiscardCommand{
			.which = slot
		}
		});
}

Input::Ret Input::discardAugment(int slot)
{
	return impl.push({
		.type = Command::Type::DiscardAugment,
		.args = DiscardCommand{
			.which = slot
		}
	});
}
