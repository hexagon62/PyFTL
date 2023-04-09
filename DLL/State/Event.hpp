#pragma once

#include "RandomAmount.hpp"
#include "WeaponBlueprint.hpp"
#include "DroneBlueprint.hpp"
#include "CrewBlueprint.hpp"
#include "SystemBlueprint.hpp"
#include "Augment.hpp"
#include "SystemType.hpp"
#include "EnvironmentType.hpp"

#include <optional>
#include <memory>

struct ShipEvent
{
	//ShipBlueprint ship;
	bool hostile = false;
	RandomAmount<int> surrenderThreshold, escapeThreshold;
};

struct ResourceEvent
{
	int missiles = 0, fuel = 0, droneParts = 0, scrap = 0, crew = 0;
	bool traitor = false, cloneable = false, steal = false, intruders = false;
	std::optional<WeaponBlueprint> weapon;
	std::optional<DroneBlueprint> drone;
	std::optional<Augment> augment;
	std::string crewType;
	int fleetDelay = 0, hullDamage = 0;
	SystemType system = SystemType::None;
	int upgradeAmount = 0;
	std::string removeAugment;
};

struct BoardingEvent
{
	std::string crewType;
	int min = 0, max = 0, amount = 0;
	bool breach = false;
};

enum class StoreBoxType
{
	Invalid = -1,
	Weapon = 0,
	Drone = 1,
	Augment = 2,
	Crew = 3,
	System = 4,
	Item = 5
};

struct StoreBox
{
	StoreBoxType type = StoreBoxType::Invalid;
	int actualPrice = 0; // may differ from blueprint!
	int id = 0;
	bool page2 = false;

	std::optional<WeaponBlueprint> weapon;
	std::optional<DroneBlueprint> drone; // may be present with drone system!
	std::optional<Augment> augment;
	std::optional<CrewBlueprint> crew;
	std::optional<SystemBlueprint> system;
};

struct Store
{
	static constexpr int HARDCODED_BOXES_PER_SECTION = 3;

	std::vector<StoreBox> boxes;
	std::vector<StoreBoxType> sections;
	int fuel = 0, fuelCost = 0;
	int missiles = 0, missileCost = 0;
	int droneParts = 0, dronePartCost = 0;
	int repairCost = 0, repairCostFull = 0;
	bool page2 = false;
};

struct EventDamage
{
	SystemType system;
	int amount = 0, effect = 0;
};

struct LocationEvent;

struct Choice
{
	std::shared_ptr<LocationEvent> event; // would've used unique but idk how to with pybind
	std::string requiredObject;
	int levelMin = 0, levelMax = std::numeric_limits<int>::max();
	int maxGroup = std::numeric_limits<int>::max();
	bool blue = false;
	bool hiddenReward = false;
};

namespace raw
{

struct Store;

}

struct LocationEvent
{
	EnvironmentType environment = EnvironmentType::Normal;
	bool environmentTargetsEnemy = false;
	bool exit = false;
	bool distress = false;
	bool revealMap = false;
	bool repair = false;
	int unlockShip = -1;

	std::optional<ShipEvent> ship;
	ResourceEvent resources, reward;
	BoardingEvent boarders;
	std::optional<Store> store;

	std::vector<EventDamage> damage;
	std::vector<Choice> choices;

	// Some implementation stuffs
	raw::Store* _storePtr = nullptr;
};
