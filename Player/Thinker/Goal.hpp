#pragma once

#include "Thinker.hpp"
#include "Reactor.hpp"

#include <unordered_map>
#include <utility>

struct Goal
{
	enum class Type
	{
		Jump, JumpOutOfDanger,

		ShieldsOff, ShieldsActive,

		EnginesMinimize, EnginesDodge, EnginesMaximize,

		OxygenIdle, OxygenMaintainRange, OxygenFillEmergency,

		MedbayIdle, MedbayHeal,

		DoorsCloseAll, DoorsVentUnimportantRooms, DoorsAssistWithBreach,
		DoorsSuffocateBoarders, DoorsBlockBoarders,

		ClonebayOff, ClonebayOn,

		MindOff, MindCancelOut, MindBoarder, MindSystem,

		HackingIdle, HackingActive
	};

	struct ShieldBubbles
	{
		int min = 0, max = 0;
	};

	struct OxygenRange
	{
		float min = 0.f, max = 1.f;
	};

	struct Room
	{
		int id = 0;
	};

	struct System
	{
		SystemID id = SystemID::Invalid;
		int artilleryIndex = -1;
	};

	Type type;

	union
	{
		ShieldBubbles shieldBubbles;
		OxygenRange oxygenRange;
		Room room;
		System system;
	};
};

class GoalThinker : public Thinker
{
public:
	GoalThinker(ThinkerParams params);

	void think() override;

	// Returns true if the goal was set
	bool set(const Goal& goal, bool overwrite = false);

	// Returns true if the goal was removed
	bool unset(Goal::Type goal);

	const Goal* get(Goal::Type goal) const;

	using GoalMutexesList = std::vector<std::pair<Goal::Type, Goal::Type>>;
	static const GoalMutexesList GoalMutexes;

private:
	Reactor reactor;
	std::unordered_map<Goal::Type, Goal> goals;

	bool fillingOxygen = true;

	void printGoal(const Goal& goal) const;

	void ftlThink();
	void shieldsThink();
	void enginesThink();
	void oxygenThink();
};
