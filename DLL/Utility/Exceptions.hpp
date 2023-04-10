#pragma once

#include <stdexcept>
#include <string>

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
			"tried to select choice " + std::to_string(choice+1) +
			" from an event with " + std::to_string(count) + " choice(s)"
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
