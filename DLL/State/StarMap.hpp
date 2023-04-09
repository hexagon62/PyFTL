#pragma once

#include "Point.hpp"
#include "Ellipse.hpp"
#include "Event.hpp"
#include "EnvironmentType.hpp"

#include <vector>
#include <string>

enum class SectorType
{
	Invalid = -1,
	Civilian, Hostile, Nebula
};

struct Sector
{
	int id = -1;
	SectorType type = SectorType::Invalid;
	std::string name;
	bool visited = false;
	bool reachable = false;
	std::vector<Sector*> neighbors;
	Point<int> position;
	int level = -1;
	bool unique = false;
};

struct Location
{
	int id = -1;
	Point<float> position;
	std::vector<Location*> neighbors;
	int visits = 0;
	bool known = false;
	bool exit = false;
	bool hazard = false;
	bool nebula = false;
	bool flagshipPresent = false;
	bool quest = false;
	bool fleetOvertaking = false;
	bool enemyShip = false;
	LocationEvent event;
};

struct StarMap
{
	std::vector<Location> locations;
	bool lastStand = false;
	bool flagshipJumping = false;
	bool mapRevealed = false;
	bool secretSector = false;
	std::vector<Location*> flagshipPath;
	Point<float> translation;
	Ellipse<float> dangerZone; // actually a circle
	int pursuitDelay = 0;
	int turnsLeft = -1;
	std::vector<Sector> sectors;
	Location* currentLocation = nullptr;
	Sector* currentSector = nullptr;
	bool choosingNewSector = false;
	bool infiniteMode = false;
	bool nebulaSector = false;
	bool distressBeacon = false;
	int sectorNumber = -1;
};
