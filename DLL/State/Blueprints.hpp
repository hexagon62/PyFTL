#pragma once

#include "WeaponBlueprint.hpp"
#include "DroneBlueprint.hpp"
#include "Augment.hpp"
#include "CrewBlueprint.hpp"
#include "SystemBlueprint.hpp"

#include <string>
#include <map>

struct Blueprints
{
	//int rarityTotal = 0;
	//gcc::map<gcc::string, ShipBlueprint> shipBlueprints;
	std::map<std::string, WeaponBlueprint> weaponBlueprints;
	std::map<std::string, DroneBlueprint> droneBlueprints;
	std::map<std::string, Augment> augmentBlueprints;
	std::map<std::string, CrewBlueprint> crewBlueprints;
	//gcc::map<gcc::string, bool> nameList;
	//gcc::map<gcc::string, gcc::string> shortNames;
	//gcc::map<gcc::string, gcc::map<gcc::string, bool>> languageNameLists;
	//gcc::map<gcc::string, ItemBlueprint> itemBlueprints;
	std::map<std::string, SystemBlueprint> systemBlueprints;
	//gcc::map<gcc::string, gcc::vector<gcc::string>> blueprintLists;
	//gcc::vector<gcc::string> currentNames;
};
