#pragma once

#include "Blueprint.hpp"
#include "Damage.hpp"
#include "WeaponType.hpp"
#include "BoostPower.hpp"

struct WeaponBlueprint : Blueprint
{
	Damage damage;
	WeaponType type = WeaponType::Invalid;
	int shots = 0;
	int missiles = 0;
	float cooldown = 0.f;
	int power = 0;
	int beamLength = -1;
	int burstRadius = 0;
	int chargeLevels = 0;
	BoostPower boost;
	int projectilesFake = 0, projectiles = 0;
	int projectilesTotal = 0;
};
