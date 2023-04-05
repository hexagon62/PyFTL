#pragma once

#include "CleanState.hpp"
#include "Raw.hpp"

#include <chrono>
#include <thread>

using Clock = std::chrono::steady_clock;
using Duration = Clock::duration;
using TimePoint = Clock::time_point;

class Reader
{
public:
	Reader() = delete;

	static bool init(); // returns true if successful, false otherwise
	static void poll();
	static void wait(); // process waiting

	// How long to block the thread for before allowing another poll
	static void setPollDelay(const Duration& delay);
	static const Duration& getPollDelay();

	static const State& getState();

private:
	static Duration delay;
	static TimePoint nextPoll;

	static raw::State rs;
	static State state;
	static uintptr_t base;
};
