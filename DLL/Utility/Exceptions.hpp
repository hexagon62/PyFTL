#pragma once

#include <stdexcept>
#include <string>

class WrongMenu final : public std::runtime_error
{
public:
	WrongMenu()
		: std::runtime_error("an input was attempted in the wrong menu state")
	{}

	WrongMenu(const std::string& what)
		: std::runtime_error(what + " was attempted in the wrong menu state")
	{}
};

struct MenuNotAvailable final : public std::runtime_error
{
public:
	MenuNotAvailable()
		: std::runtime_error("tried to open a menu that's not available")
	{}

	MenuNotAvailable(const std::string& what, const std::string& why)
		: std::runtime_error("tried to open " + what + " but " + why)
	{}
};

class InvalidTime final : public std::invalid_argument
{
public:
	InvalidTime(const std::string& why)
		: std::invalid_argument(
			"tried to use an invalid time value (" + why + ")"
		)
	{}
};

class GameNotRunning final : public std::runtime_error
{
public:
	GameNotRunning()
		: std::runtime_error("an input was attempted when the game wasn't running")
	{}

	GameNotRunning(const std::string& what)
		: std::runtime_error(what + " was attempted when the game wasn't running")
	{}
};

class InvalidHotkey final : public std::invalid_argument
{
public:
	InvalidHotkey(const std::string& hotkey)
		: std::invalid_argument(
			"there is no hotkey called '" + hotkey + "'"
		)
	{}
};

class NoEvent final : public std::runtime_error
{
public:
	NoEvent()
		: std::runtime_error(
			"tried to do an input that requires an active event"
		)
	{}
};

class NoStore final : public std::runtime_error
{
public:
	NoStore()
		: std::runtime_error(
			"tried to do an input that requires a store present"
		)
	{}
};

class InvalidEventChoice final : public std::out_of_range
{
public:
	InvalidEventChoice(int choice, int count)
		: std::out_of_range(
			"tried to select event choice #" + std::to_string(choice + 1) +
			" from an event with " + std::to_string(count) + " choice(s)"
		)
	{}

	InvalidEventChoice(int choice, const std::string& why)
		: std::out_of_range(
			"tried to select event choice #" + std::to_string(choice + 1) + " but " + why
		)
	{}
};

class InvalidPowerRequest final : public std::invalid_argument
{
public:
	InvalidPowerRequest(const Weapon& weapon, const std::string& why)
		: std::invalid_argument(
			"tried to toggle weapon #" + std::to_string(weapon.slot) + " but " + why
		)
	{}

	InvalidPowerRequest(const Drone& drone, const std::string & why)
		: std::invalid_argument(
			"tried to toggle drone #" + std::to_string(drone.slot) + " but " + why
		)
	{}

	InvalidPowerRequest(const System& system, const std::string& why)
		: std::invalid_argument(
			"tried to change the power of " + systemName(system.type) +
			" (@" + std::to_string(system.uiBox) + ") but " + why
		)
	{}

	InvalidPowerRequest(const System & system, int attempted, int multiple)
		: std::invalid_argument(
			"tried to change the power of " + systemName(system.type) +
			" (@" + std::to_string(system.uiBox) + ") to " + std::to_string(attempted) +
			", but the power must be a multiple of " +
			std::to_string(multiple) + " plus/minus any bonus power present"
		)
	{}

	InvalidPowerRequest(const System& system, int attempted, const std::pair<int, int>& range)
		: std::invalid_argument(
			"tried to change the power of " + systemName(system.type) +
			" (@" + std::to_string(system.uiBox) + ") to " + std::to_string(attempted) +
			", but the current legal range is " +
			std::to_string(range.first) + "-" + std::to_string(range.second)
		)
	{}
};

class SystemInoperable final : public std::runtime_error
{
public:
	SystemInoperable(const System& system)
		: std::runtime_error(
			"tried to operate " + systemName(system.type) +
			" (@" + std::to_string(system.uiBox) + ") but it's in an inoperable state"
		)
	{}

	SystemInoperable(const System& system, const std::string& why)
		: std::runtime_error(
			"tried to operate " + systemName(system.type) +
			" (@" + std::to_string(system.uiBox) + ") but " + why
		)
	{}
};

class DoorInoperable final : public std::runtime_error
{
public:
	DoorInoperable(const Door& door, const std::string& why)
		: std::runtime_error(
			"tried to operate door #" + std::to_string(door.id) + " but " + why
		)
	{}
};

class InvalidSlotChoice final : public std::out_of_range
{
public:
	InvalidSlotChoice(const std::string& what, int slot)
		: std::out_of_range(
			"tried to use a " + what + " in invalid slot #" + std::to_string(slot)
		)
	{}

	InvalidSlotChoice(const std::string& what, int slot, int count)
		: std::out_of_range(
			"tried to use a " + what + " in slot #" + std::to_string(slot) +
			" but there are only " + std::to_string(count) + " usable slots"
		)
	{}

	InvalidSlotChoice(const std::string& what, int slot, const std::string& why)
		: std::out_of_range(
			"tried to use a " + what + " in slot #" + std::to_string(slot) + " but " + why
		)
	{}
};

class InvalidSwap final : public std::out_of_range
{
public:
	InvalidSwap(const std::string& whatA, int slotA, const std::string& whatB, int slotB)
		: std::out_of_range(
			"tried to swap " + whatA + " slot #" + std::to_string(slotA+1) +
			" and " + whatB + " slot #" + std::to_string(slotB+1) +
			" but both slots are empty or one slot is invalid"
		)
	{}
};

class InvalidDiscard final : public std::out_of_range
{
public:
	InvalidDiscard(const std::string& what, int slot)
		: std::out_of_range(
			"tried to discard item in " + what + " slot #" + std::to_string(slot+1) +
			" but the slot is empty or invalid"
		)
	{}
};

class InvalidCrewChoice final : public std::invalid_argument
{
public:
	InvalidCrewChoice(const Crew& crew, const std::string& why)
		: std::invalid_argument(
			"tried to use crewmember " + crew.blueprint.name +
			" (@" + std::to_string(crew.id) + ") but " + why
		)
	{}
};

class InvalidCrewBoxChoice final : public std::out_of_range
{
public:
	InvalidCrewBoxChoice(int which)
		: std::out_of_range(
			"tried to use crewmember #" + std::to_string(which) + " which is invalid"
		)
	{}

	InvalidCrewBoxChoice(int which, int count)
		: std::out_of_range(
			"tried to use crewmember #" + std::to_string(which) +
			" but there's only " + std::to_string(count) + " selectable crew"
		)
	{}
};

class NotSelected final : public std::runtime_error
{
public:
	NotSelected(const std::string& what)
		: std::runtime_error(
			"tried to do an action that requires a " + what + " to be selected"
		)
	{}
};

class InvalidSelfAim final : public std::runtime_error
{
public:
	InvalidSelfAim(const std::string& why)
		: std::runtime_error(
			"tried to aim at the player's own ship but " + why
		)
	{}
};

class InvalidHackingInput final : public std::runtime_error
{
public:
	InvalidHackingInput(bool setup, const std::string& why)
		: std::runtime_error(
			"tried to " + std::string(setup ? "hack" : "send hacking drone") + " but " + why
		)
	{}
};

class CannotAfford final : public std::invalid_argument
{
public:
	CannotAfford(const std::string& what, int scrap, int cost)
		: std::invalid_argument(
			"tried to purchase " + what + " but it costs " + std::to_string(cost) +
			" scrap and I only have " + std::to_string(scrap) + " scrap"
		)
	{}
};

class InvalidUpgrade final : public std::invalid_argument
{
public:
	InvalidUpgrade(SystemType type, int from, int to, int cap)
		: std::invalid_argument(
			"tried to upgrade the " + systemName(type) + " system from levels " +
			std::to_string(from) + " to " + std::to_string(to) + " which is invalid "
			"because the system's maximum level is " + std::to_string(cap)
		)
	{}

	InvalidUpgrade(SystemType type, int from, int to)
		: std::invalid_argument(
			"tried to upgrade the " + systemName(type) + " system from levels " +
			std::to_string(from) + " to " + std::to_string(to) + " which is invalid"
		)
	{}

	InvalidUpgrade(int from, int to, int cap)
		: std::invalid_argument(
			"tried to upgrade the reactor from levels " +
			std::to_string(from) + " to " + std::to_string(to) + " which is invalid "
			"because the reactor's maximum level is " + std::to_string(cap)
		)
	{}

	InvalidUpgrade(int from, int to)
		: std::invalid_argument(
			"tried to upgrade the reactor from levels " +
			std::to_string(from) + " to " + std::to_string(to) + " which is invalid"
		)
	{}
};
