#pragma once

#include "Thinker.hpp"
#include "Goal.hpp"

// We switch to this logic when there is no enemy present
class IdleThinker : public Thinker
{
public:
	IdleThinker(ThinkerParams params, GoalThinker& goals);

	void think() override;

private:
	GoalThinker& goals;

	void shieldsThink();
	void oxygenThink();
	void ftlThink();
};
