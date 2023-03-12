#pragma once

#include "Thinker.hpp"
#include "Goal.hpp"
#include "IdleThinker.hpp"
#include "BattleThinker.hpp"

enum class RootState
{
	Idle, Battle
};

class RootThinker : public Thinker
{
public:
	RootThinker(ThinkerParams params);

	void think() override;
	RootState getState() const;

private:
	GoalThinker goals;
	IdleThinker idleThinker;
	BattleThinker battleThinker;

	void evaluateState();

	RootState state = RootState::Idle;
};
