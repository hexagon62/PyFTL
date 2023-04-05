#pragma once

#include <memory>
#include <vector>
#include <array>

#include "../GameState/Reader.hpp"
#include "Input.hpp"

extern bool g_active;
extern bool g_pause;

struct ReactorPower
{
	int shields = 0;
	int engines = 0;
	int medbay = 0;
	int clonebay = 0;
	int oxygen = 0;
	int teleporter = 0;
	int cloaking = 0;
	int artillery = 0;
	int mind = 0;
	int hacking = 0;
	int weapons = 0;
	int drones = 0;
};

struct PlayerState
{
	ReactorPower desiredPower;

	int shieldsNeeded = 0;
	bool preppingForJump = false;

	std::array<float, 4> oxygenBreakpoints{ 0.5f, 0.4f, 0.2f, 0.1f };
};

class Player
{
public:
	Player(const StateReader& reader);

	void iterate(int dt = 0);
	void think(int dt = 0);
	void act(int dt = 0);

	const PlayerState& getState() const
	{
		return this->state;
	}

private:
	const StateReader& reader;
	Input input;
	PlayerState state;

	void processReactorInputs();
};
