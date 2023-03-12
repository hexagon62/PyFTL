#include "BattleThinker.hpp"
#include "Reactor.hpp"

BattleThinker::BattleThinker(ThinkerParams params, GoalThinker& goals)
	: Thinker(params)
	, goals(goals)
{

}

void BattleThinker::think()
{
	this->goals.unset(Goal::Type::Jump);
	this->goals.unset(Goal::Type::JumpOutOfDanger);

	this->shieldsThink();
	this->oxygenThink();
}

void BattleThinker::shieldsThink()
{
	int bubbles = this->clampLevel(SystemID::Shields) / 2;

	this->goals.set({
		.type = Goal::Type::ShieldsActive,
		.shieldBubbles = {
			.min = bubbles,
			.max = bubbles
		}
	});
}

void BattleThinker::oxygenThink()
{
	auto& oxygen = this->state.player.systems.oxygen;
	if (!oxygen.present)
		return;

	if (oxygen.total < 0.2f)
		this->goals.set({ Goal::Type::OxygenFillEmergency });
	else
		this->goals.set({
			.type = Goal::Type::OxygenMaintainRange,
			.oxygenRange = {
				.min = 0.4f,
				.max = 0.49f
			}
		});
}
