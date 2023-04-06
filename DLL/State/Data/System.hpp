#pragma once

#include "SystemType.hpp"
#include "SystemBlueprint.hpp"
#include "Power.hpp"

#include <array>
#include <utility>

struct System
{
	SystemType type = SystemType::None;
	SystemBlueprint blueprint;
	int room = -1;
	Power power;
	std::pair<int, int> health{ 0, 0 }, level{ 0, 0 };
	int manningLevel = 0;
	bool needsManning = false;
	bool occupied = false;
	bool onFire = false;
	bool breached = false;
	bool boardersAttacking = false;
	float damageProgress = 0.f;
	float repairProgress = 0.f;
};
