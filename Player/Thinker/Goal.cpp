#include "Goal.hpp"
#include "../../GameState/StatePrinting.hpp"

#include <vector>

namespace
{

template<typename... Ts>
void logGoal(Ts&&... args)
{
	std::cout << "Goal: ";
	(std::cout << ... << std::forward<Ts>(args));
	std::cout << '\n';
}

}

const GoalThinker::GoalMutexesList GoalThinker::GoalMutexes{
	{Goal::Type::Jump, Goal::Type::JumpOutOfDanger},
	{Goal::Type::ShieldsOff, Goal::Type::ShieldsActive},
	{Goal::Type::EnginesMinimize, Goal::Type::EnginesMaximize},
	{Goal::Type::OxygenIdle, Goal::Type::OxygenFillEmergency},
	{Goal::Type::MedbayIdle, Goal::Type::MedbayHeal},
	{Goal::Type::DoorsCloseAll, Goal::Type::DoorsBlockBoarders},
	{Goal::Type::ClonebayOff, Goal::Type::ClonebayOn},
	{Goal::Type::MindOff, Goal::Type::MindSystem},
	{Goal::Type::HackingIdle, Goal::Type::HackingActive},
};

GoalThinker::GoalThinker(ThinkerParams params)
	: Thinker(params)
	, reactor(params)
{

}

void GoalThinker::think()
{
	this->shieldsThink();
	this->oxygenThink();

	this->ftlThink();

	this->reactor.think();
}

void GoalThinker::ftlThink()
{
	auto jump = this->goals.find(Goal::Type::Jump);
	auto jumpUrgent = this->goals.find(Goal::Type::JumpOutOfDanger);

	auto& [ftl, max] = this->state.player.ftl;

	if (jump != this->goals.end())
	{
		if (this->reactor.systemDesire(SystemID::Engines) == 0)
			this->reactor.set(SystemID::Engines, 1, Priority::Maximum);

		if (ftl == max)
			this->input.openJumpMenu();
	}

	if (jumpUrgent != this->goals.end())
	{
		this->set({ Goal::Type::EnginesMaximize });

		if (ftl == max)
			this->input.openJumpMenu();
	}
}

void GoalThinker::shieldsThink()
{
	auto off = this->goals.find(Goal::Type::ShieldsOff);
	auto active = this->goals.find(Goal::Type::ShieldsActive);

	if (off != this->goals.end())
	{
		this->reactor.removeGroup(SystemID::Shields);
	}

	if (active != this->goals.end())
	{
		auto& [min, max] = active->second.shieldBubbles;

		this->reactor.set(SystemID::Shields, min * 2, Priority::High);
		this->reactor.set(SystemID::Shields, (max-min)*2, Priority::MediumHigh);
	}
}

void GoalThinker::enginesThink()
{
	auto minimize = this->goals.find(Goal::Type::EnginesMinimize);
	auto dodge = this->goals.find(Goal::Type::EnginesDodge);
	auto maximize = this->goals.find(Goal::Type::EnginesMaximize);

	// ...
}

void GoalThinker::oxygenThink()
{
	auto idle = this->goals.find(Goal::Type::OxygenIdle);
	auto range = this->goals.find(Goal::Type::OxygenMaintainRange);
	auto emergency = this->goals.find(Goal::Type::OxygenFillEmergency);

	if (idle != this->goals.end())
	{
		this->reactor.setGroup(
			SystemID::Oxygen,
			1,
			Priority::Low);
	}
	
	if (range != this->goals.end())
	{
		auto& [min, max] = range->second.oxygenRange;
		auto& oxygen = this->state.player.systems.oxygen;

		if (oxygen.total >= max && this->fillingOxygen)
			this->fillingOxygen = false;
		else if (oxygen.total <= min && !this->fillingOxygen)
			this->fillingOxygen = true;

		int desiredPower = this->fillingOxygen ? 1 : 0;

		this->reactor.setGroup(
			SystemID::Oxygen,
			desiredPower,
			Priority::Medium);
	}
	else this->fillingOxygen = true;

	if (emergency != this->goals.end())
	{
		this->reactor.setGroup(
			SystemID::Oxygen,
			this->clampLevel(SystemID::Oxygen),
			Priority::Maximum);
	}
}

bool GoalThinker::set(const Goal& goal, bool overwrite)
{
	{
		auto existing = this->goals.find(goal.type);

		if (existing != this->goals.end())
		{
			if (!overwrite)
				return false;

			// If we already have a goal of this type, we'll overwrite it
			this->goals.erase(goal.type);
		}

		// If there's any mutually exclusive goals present, we'll remove those
		auto mutex = std::find_if(GoalMutexes.begin(), GoalMutexes.end(),
			[&goal](auto& range) {
				return range.first <= goal.type && range.second >= goal.type;
			}
		);

		if (mutex != GoalMutexes.end())
		{
			for (int i = int(mutex->first); i <= int(mutex->second); ++i)
				this->unset(Goal::Type(i));
		}

		this->goals.emplace(goal.type, goal);
	}

	this->printGoal(goal);

	return true;
}

bool GoalThinker::unset(Goal::Type goal)
{
	this->goals.erase(goal);
	return true;
}

const Goal* GoalThinker::get(Goal::Type goal) const
{
	auto g = this->goals.find(goal);
	if (g == this->goals.end())
		return nullptr;

	return &g->second;
}

void GoalThinker::printGoal(const Goal& goal) const
{

	switch (goal.type)
	{
	case Goal::Type::Jump:
	{
		logGoal("Jump.");
		break;
	}
	case Goal::Type::JumpOutOfDanger:
	{
		logGoal("It's too dangerous to stay. Jump ASAP!");
		break;
	}
	case Goal::Type::ShieldsOff:
	{
		logGoal("Shields off.");
		break;
	}
	case Goal::Type::ShieldsActive:
	{
		auto& [min, max] = goal.shieldBubbles;
		logGoal("Maintain ", min, " to ", max, " shield bubbles.");
		break;
	}
	case Goal::Type::EnginesMinimize:
	{
		logGoal("Minimize engine usage.");
		break;
	}
	case Goal::Type::EnginesDodge:
	{
		logGoal("Maximize engine usage when projectiles are about to potentially hit.");
		break;
	}
	case Goal::Type::EnginesMaximize:
	{
		logGoal("Maximize engine usage.");
		break;
	}
	case Goal::Type::OxygenIdle:
	{
		logGoal("Let oxygen fill if power bars are available.");
		break;
	}
	case Goal::Type::OxygenMaintainRange:
	{
		auto& [min, max] = goal.oxygenRange;
		logGoal("Try to keep oxygen between ", min * 100.f, "% and ", max * 100.f, "%.");
		break;
	}
	case Goal::Type::OxygenFillEmergency:
	{
		logGoal("Forcefully refill oxygen as quickly as possible.");
		break;
	}
	case Goal::Type::HackingIdle:
	{
		logGoal("Don't hack.");
		break;
	}
	case Goal::Type::HackingActive:
	{
		auto& [id, art] = goal.system;

		if (art >= 0)
			logGoal("Hack the enemy's ", id, "[", art, "]");
		else
			logGoal("Hack the enemy's ", id);

		break;
	}
	}
}