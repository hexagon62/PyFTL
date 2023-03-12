#pragma once

#include <memory>

#include "Thinker/Thinker.hpp"

class Player
{
public:
	Player(const StateReader& reader);

	void onLoop(int dt);

private:
	const StateReader& reader;
	Input input;
	std::unique_ptr<Thinker> thinker;
};
