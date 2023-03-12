#pragma once

#include <memory>

#include "../GameState/Reader.hpp"
#include "Input.hpp"

extern bool g_active;

struct PlayerState
{
	int shieldsNeeded = 0;
	bool preppingForJump = false;
};

class Player
{
public:
	Player(const StateReader& reader);

	void think(int dt = 0);

	const PlayerState& getState() const
	{
		return this->state;
	}

private:
	const StateReader& reader;
	Input input;
	PlayerState state;

	int shieldsNeeded(const State& state) const;
};
