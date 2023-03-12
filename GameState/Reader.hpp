#pragma once

#include "State.hpp"
#include "Addresses.hpp"

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <initializer_list>
#include <unordered_map>

class ProcessReader;

// Does all the work with the known addresses and pointer offsets
class StateReader
{
public:
	StateReader(const AddressData& addresses);
	~StateReader();

	// If unconditional, the reader reads everything regardless of the
	// FTL window being minimized, focused, or the game being paused.
	// It will however still check if you're in-game or not.
	bool poll(bool unconditional = false);
	bool hooked() const;
	operator bool() const;

	const State& getState() const;
	const KnownBlueprints& getBlueprints() const;

	const ProcessReader& getProcessReader() const;

private:
	State state;
	KnownBlueprints blueprints;
	const AddressData& addresses;

	// Implementation stuff
	class Impl;
	std::unique_ptr<ProcessReader> processReader;
	std::unique_ptr<Impl> impl;

	void errorCheck() const;
};