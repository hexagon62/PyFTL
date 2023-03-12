#pragma once

#include "Thinker.hpp"
#include "Goal.hpp"

class BattleThinker : public Thinker
{
public:
	BattleThinker(ThinkerParams params, GoalThinker& goals);

	void think() override;

private:
	GoalThinker& goals;

	void shieldsThink();
	void oxygenThink();
};
