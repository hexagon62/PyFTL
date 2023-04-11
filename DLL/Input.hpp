#pragma once

#include "Reader.hpp"

// Very hacky way of signalling to Windows this is generated by the AI
// ... if the Windows API is needed anyways
#define WM_FTLAI 0x800

enum class MouseButton
{
	None = -1,
	Left, Middle, Right
};

enum class InputDirection
{
	Unchanged = -1,
	Up = 0, Down = 1
};

class Input
{
public:
	static constexpr int GAME_WIDTH = 1280;
	static constexpr int GAME_HEIGHT = 720;

	Input() = delete;

	static void iterate(); // make inputs
	static bool empty(); // check if no inputs are queued
	static bool queued(uintmax_t id); // check if an input with this id is queued
	static bool pop(uintmax_t id); // dequeue the input with this id, returns false if not in queue
	static bool ready(); // check if ready
	static void setReady(bool ready = true); // sets the ready state
	static void allowHumanMouse(bool allow = true); // sets if human mouse input is allowed
	static void allowHumanKeyboard(bool allow = true); // sets if human keyboard input is allowed
	static bool humanMouseAllowed(); // checks if human mouse input is allowed
	static bool humanKeyboardAllowed(); // checks if human keyboard input is allowed
	static void clear(); // clears the input queue

	// All inputs below are added to a queue, not done immediately
	// This is so they're roughly executed sequentially
	// They all will return a tuple containing:
	// - the id of the *last* input they put in the queue
	// - the time point the input will be made, measured in milliseconds since PyFTL started

	using Ret = std::pair<uintmax_t, double>;

	static Ret mouseMove(
		const Point<int>& pos,
		double delay = 0.0
	);

	static Ret mouseDown(
		MouseButton button = MouseButton::Left,
		const Point<int>& position = { -1, -1 },
		bool shift = false,
		double delay = 0.0
	);

	static Ret mouseUp(
		MouseButton button = MouseButton::Left,
		const Point<int>& position = { -1, -1 },
		bool shift = false,
		double delay = 0.0
	);

	static Ret mouseClick(
		MouseButton button = MouseButton::Left,
		const Point<int>& position = { -1, -1 },
		bool shift = false,
		bool partial = false, // don't mouse down if true
		double delay = 0.0
	);

	static Ret keyDown(
		Key key,
		bool shift = false,
		double delay = 0.0
	);

	static Ret keyUp(
		Key key,
		bool shift = false,
		double delay = 0.0
	);

	static Ret keyPress(
		Key key,
		bool shift = false,
		double delay = 0.0
	);

	static Ret hotkeyDown(
		const std::string& hotkey,
		bool shift = false,
		double delay = 0.0
	);

	static Ret hotkeyUp(
		const std::string& hotkey,
		bool shift = false,
		double delay = 0.0
	);

	static Ret hotkeyPress(
		const std::string& hotkey,
		bool shift = false,
		double delay = 0.0
	);

	static const decltype(Settings::hotkeys)& hotkeys();

	static Ret eventChoice(
		int which,
		bool suppress = false,
		double delay = 0.0
	);

	static Ret systemPower(
		const System& system,
		int set = 0, int delta = 0,
		bool suppress = false,
		double delay = 0.0
	);

	static Ret systemPower(
		SystemType which,
		int set = 0, int delta = 0,
		bool suppress = false,
		double delay = 0.0
	);

	static Ret weaponPower(
		const Weapon& weapon,
		bool on = true,
		bool suppress = false,
		double delay = 0.0
	);

	static Ret weaponPower(
		int which,
		bool on = true,
		bool suppress = false,
		double delay = 0.0
	);

	static Ret weaponSelect(
		const Weapon& weapon,
		bool suppress = false,
		double delay = 0.0
	);

	static Ret weaponSelect(
		int which,
		bool suppress = false,
		double delay = 0.0
	);

	static Ret dronePower(
		const Drone& drone,
		bool on = true,
		bool suppress = false,
		double delay = 0.0
	);

	static Ret dronePower(
		int which,
		bool on = true,
		bool suppress = false,
		double delay = 0.0
	);

private:
	class Impl;
	static Impl impl;

	static bool good, humanMouse, humanKeyboard;
};