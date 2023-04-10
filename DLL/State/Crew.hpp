#pragma once

#include "Point.hpp"
#include "CrewBlueprint.hpp"
#include "SystemType.hpp"

#include <utility>
#include <vector>

struct Path
{
	Point<int> start, finish;
	std::vector<int> doors;
	float distance = -1.f;
};

namespace raw
{

struct Drone;

}

struct Crew
{
	int uiBox = -1, selectionState = 0;
	CrewBlueprint blueprint;
	Point<float> position, goal;
	std::pair<float, float> health{ 0.f, 0.f };
	Point<float> speed;
	Path path;
	bool player = false;
	bool onPlayerShip = false;
	bool newPath = false;
	bool suffocating = false;
	bool repairing = false;
	bool intruder = false;
	bool fighting = false;
	bool dead = false;
	bool dying = false;
	bool manning = false;
	bool moving = false;
	bool healing = false;
	int onFire = 0;
	int room = 0, slot = 0; // slot is set when reading room slots... FTL moment
	SystemType mannedSystem = SystemType::None;
	int roomGoal = 0, slotGoal = 0;
	int roomSaved = 0, slotSaved = 0;
	int cloneQueuePosition = -1; // will be set when reading clonebay status
	int deathId = -1; // used to sort the clone queue
	float cloneDeathProgress = 0.f;
	bool readyToClone = false;
	bool mindControlled = false;
	int mindControlHealthBoost = 0;
	float mindControlDamageMultiplier = 1.f;
	float stunTime = 0.f;
	std::pair<float, float> teleportTimer{ 0.f, 0.f };
	bool teleporting = false;
	bool leaving = false;
	bool arriving = false;
	bool drone = false;
};
