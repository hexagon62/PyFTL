#pragma once

#include "Blueprint.hpp"
#include "DroneType.hpp"

#include <array>

struct DroneBlueprint : Blueprint
{
	static constexpr std::array<float, 5> HARDCODED_SHIELD_COOLDOWNS{ 8.f, 10.f, 13.f, 16.f, 20.f };

	DroneType type = DroneType::Invalid;
	int power = 0;
	float cooldown = 0.f;
	int speed = 0;
};
