#pragma once

#include "../Utility/ValueScopeGuard.hpp"
#include "State.hpp"
#include "Raw.hpp"

#include <chrono>
#include <thread>

using Clock = std::chrono::steady_clock;
using Duration = Clock::duration;
using TimePoint = Clock::time_point;

class MutableRawState
{
	// Only some classes may use this
	friend class Input;

	MutableRawState() {}
};

class Reader
{
public:
	Reader() = delete;

	static bool init(); // returns true if successful, false otherwise
	static void fullPoll(); // handles waiting, polling, and input (if ready)
	static void poll(); // process polling
	static void wait(); // process waiting

	// How long to block the thread for before allowing another poll
	static void setPollDelay(const Duration& delay);
	static const Duration& getPollDelay();

	static const State& getState();
	static const raw::State& getRawState();
	static raw::State& getRawState(MutableRawState); // gets mutable raw state, if allowed

	// gets a pointer to some address, if allowed; very low level
	static void* getMemory(uintptr_t offset, MutableRawState);

	// When using a separate thread for the reader
	// the reader will poll in its own thread
	// however whether or not we still call Python functions like on_update
	// depends on how the main loop is implemented
	static void setSeperateThread(bool on = true);
	static bool usingSeperateThread();

	// Used to signal the rest of the program that things need to be reloaded
	// This includes stuff like Python scripts
	static void reload();
	static void finishReload(); // for use by main loop ONLY, not in the Python scripts!
	static bool reloadRequested(); // for use by main loop ONLY, not in the Python scripts!

	// Used to request a quit
	static void quit();

private:
	static Duration delay;
	static TimePoint nextPoll;

	static raw::State rs;
	static State state;
	static uintptr_t base;
	static std::jthread thread;
	static bool reloading;
};
