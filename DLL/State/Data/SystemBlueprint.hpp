#pragma once

#include "Blueprint.hpp"

#include <vector>

struct SystemBlueprint : Blueprint
{
	int powerStart = 0, powerMax = 0;
	std::vector<int> upgradeCosts;
};
