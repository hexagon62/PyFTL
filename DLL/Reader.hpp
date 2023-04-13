#pragma once

#include "Utility/ValueScopeGuard.hpp"
#include "State.hpp"
#include "Raw.hpp"

#include <chrono>

class MutableRawState
{
	// Only some classes may use this
	friend class Input;
	friend class Reader;

	MutableRawState() {}
};

class Reader
{
public:
	using Clock = std::chrono::steady_clock;
	using Duration = Clock::duration;
	using TimePoint = Clock::time_point;

	Reader() = delete;

	// handles polling, and input (if ready)
	static void poll();

	static bool init(); // returns true if successful, false otherwise
	static bool ready(); // returns true if initialized

	static double now(); // gets the time since start in seconds

	static const State& getState();
	static const raw::State& getRawState();
	static raw::State& getRawState(MutableRawState); // gets mutable raw state, if allowed

	// get an address relative to the game's base address
	static uintptr_t getRealAddress(uintptr_t offset, MutableRawState);

	// Used to signal the rest of the program that things need to be reloaded
	// This includes stuff like Python scripts
	static void reload();
	static void finishReload(); // for use by main loop ONLY, not in the Python scripts!
	static bool reloadRequested(); // for use by main loop ONLY, not in the Python scripts!

	// Used to request a quit
	static void quit();

private:
	static void iterate(); // handles polling, and input (if ready)
	static void read(); // process polling

	static TimePoint start;

	static raw::State rs;
	static State state;
	static uintptr_t base;
	static bool started, reloading;
};
