#pragma once

#include "State.hpp"
#include "Raw.hpp"
#include "Addresses.hpp"

#include <memory>

// Does all the work with the known addresses and pointer offsets
class StateReader
{
public:
	StateReader(const AddressData& addresses);
	~StateReader();

	void poll();

	const State& getState() const;
	const raw::State& getRawState() const;
	const KnownBlueprints& getBlueprints() const;

private:
	State state;
	raw::State rawState;
	KnownBlueprints blueprints;
	const AddressData& addresses;

	// Implementation stuff
	class Impl;
	std::unique_ptr<Impl> impl;
};
