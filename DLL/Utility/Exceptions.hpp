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
		: std::runtime_error(
			"tried to do an input that requires the game to be running"
		)
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

class InvalidSlotChoice final : public std::out_of_range
{
public:
	InvalidSlotChoice(const std::string& what, int slot)
		: std::out_of_range(
			"tried to toggle a " + what + " in invalid slot #" + std::to_string(slot)
		)
	{}

	InvalidSlotChoice(const std::string& what, int slot, int count)
		: std::out_of_range(
			"tried to toggle a " + what + " in slot #" + std::to_string(slot) +
			" but there are only " + std::to_string(count) + " taken slots"
		)
	{}

	InvalidSlotChoice(const std::string& what, int slot, const std::string& why)
		: std::out_of_range(
			"tried to toggle a " + what + " in slot #" + std::to_string(slot) + " but " + why
		)
	{}
};

class InvalidCrewChoice final : public std::invalid_argument
{
public:
	InvalidCrewChoice(const Crew& crew, const std::string& why)
		: std::invalid_argument(
			"tried to select crewmember " + crew.blueprint.name +
			" (@" + std::to_string(crew.uiBox) + ") but " + why
		)
	{}
};

class InvalidCrewBoxChoice final : public std::out_of_range
{
public:
	InvalidCrewBoxChoice(int which)
		: std::out_of_range(
			"tried to select crewmember #" + std::to_string(which) + " which is invalid"
		)
	{}

	InvalidCrewBoxChoice(int which, int count)
		: std::out_of_range(
			"tried to select crewmember #" + std::to_string(which) +
			" but there's only " + std::to_string(count) + " selectable crew"
		)
	{}
};
