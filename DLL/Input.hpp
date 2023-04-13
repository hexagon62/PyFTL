#pragma once

#include "Reader.hpp"

#include <functional>

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
	static bool emptyWithin(double time); // check if no inputs are queued within this amount of time from now
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

	static Ret text(char ch, double delay = 0.0);
	static Ret text(const std::string& str, double delay = 0.0);
	static Ret textConfirm(double delay = 0.0);
	static Ret textClear(double delay = 0.0);
	static Ret textBackspace(double delay = 0.0);
	static Ret textDelete(double delay = 0.0);
	static Ret textLeft(double delay = 0.0);
	static Ret textRight(double delay = 0.0);
	static Ret textHome(double delay = 0.0);
	static Ret textEnd(double delay = 0.0);

	static void cheat(const std::string& command, bool suppress = false);

	static Ret pause(
		bool on = true,
		bool suppress = false,
		double delay = 0.0
	);

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

	static Ret weaponPower(
		const Weapon& weapon,
		bool on = true,
		bool suppress = false,
		double delay = 0.0
	);

	static Ret weaponSelect(
		const Weapon& weapon,
		bool suppress = false,
		double delay = 0.0
	);

	static Ret dronePower(
		const Drone& drone,
		bool on = true,
		bool suppress = false,
		double delay = 0.0
	);

	static Ret autofire(
		bool on = false,
		bool suppress = false,
		double delay = 0.0
	);

	static Ret openAllDoors(
		bool airlocks = false,
		bool suppress = false,
		double delay = 0.0
	);

	static Ret closeAllDoors(
		bool suppress = false,
		double delay = 0.0
	);

	static Ret aimCancel(
		bool suppress = false,
		double delay = 0.0
	);

	static Ret crewSelect(
		const Crew& crew,
		bool exclusive = true,
		bool suppress = false,
		double delay = 0.0
	);

	using CrewRefList = std::vector<std::reference_wrapper<const Crew>>;

	// Yes, you can't just slap in a plain std::vector<Crew> here
	// That's because selection order matters!
	// So at least on the C++ side if we call this, we should be deliberate
	static Ret crewSelect(
		const CrewRefList& crew,
		bool exclusive = true,
		bool suppress = false,
		double delay = 0.0
	);

	static Ret crewSelectAll(
		bool suppress = false,
		double delay = 0.0
	);

	static Ret crewCancel(
		bool suppress = false,
		double delay = 0.0
	);

	static Ret crewSaveStations(
		bool suppress = false,
		double delay = 0.0
	);

	static Ret crewLoadStations(
		bool suppress = false,
		double delay = 0.0
	);

private:
	class Impl;
	static Impl impl;

	static bool good, humanMouse, humanKeyboard;
};
