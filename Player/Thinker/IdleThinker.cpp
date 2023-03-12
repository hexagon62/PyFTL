#include "IdleThinker.hpp"

IdleThinker::IdleThinker(ThinkerParams params, GoalThinker& goals)
	: Thinker(params)
	, goals(goals)
{

}

void IdleThinker::think()
{
	this->shieldsThink();
	this->oxygenThink();
	this->ftlThink();
}

void IdleThinker::shieldsThink()
{
	auto& shields = this->state.player.systems.shields;
	if (!shields.present)
		return;

	this->goals.set({
		.type = Goal::Type::ShieldsActive,
		.shieldBubbles = {
			.min = this->clampLevel(SystemID::Shields, 2) / 2,
			.max = this->clampLevel(SystemID::Shields) / 2
		}
	});
}

void IdleThinker::oxygenThink()
{
	auto& oxygen = this->state.player.systems.oxygen;
	if (!oxygen.present)
		return;

	if (oxygen.total < 0.2f)
		this->goals.set({ Goal::Type::OxygenFillEmergency });
	else
		this->goals.set({ Goal::Type::OxygenIdle });
}

void IdleThinker::ftlThink()
{
	auto& oxygen = this->state.player.systems.oxygen;

	if (!oxygen.present || oxygen.total >= 0.75f)
	{
		this->goals.set({ Goal::Type::Jump });
	}
}