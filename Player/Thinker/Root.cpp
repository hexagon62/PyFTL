#include "Root.hpp"
#include <iostream>

RootThinker::RootThinker(ThinkerParams params)
	: Thinker(params)
	, goals(params)
	, idleThinker(params, goals)
	, battleThinker(params, goals)
{}

void RootThinker::think()
{
	this->evaluateState();

	switch (this->state)
	{
	case RootState::Idle: this->idleThinker.think(); break;
	case RootState::Battle: this->battleThinker.think(); break;
	}

	this->goals.think();
}

void RootThinker::evaluateState()
{
	if (this->targetAlive())
	{
		if (this->state != RootState::Battle)
		{
			std::cout << "Status: battling a ship!\n";
			this->state = RootState::Battle;
		}
	}
	else if (this->state != RootState::Idle)
	{
		std::cout << "Status: idle.\n";
		this->state = RootState::Idle;
	}
}

RootState RootThinker::getState() const
{
	return this->state;
}