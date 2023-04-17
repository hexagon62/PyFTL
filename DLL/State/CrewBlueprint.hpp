#pragma once

#include "Blueprint.hpp"

#include <string>
#include <utility>

struct CrewBlueprint : Blueprint
{
	static constexpr int HARDCODED_MAX_NAME_LENGTH = 15;

	std::string name, nameLong, species;
	bool male = false;
	std::pair<int, int> skillPiloting{ 0, 0 };
	std::pair<int, int> skillEngines{ 0, 0 };
	std::pair<int, int> skillShields{ 0, 0 };
	std::pair<int, int> skillWeapons{ 0, 0 };
	std::pair<int, int> skillRepair{ 0, 0 };
	std::pair<int, int> skillCombat{ 0, 0 };
};
