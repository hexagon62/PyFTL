#pragma once

#include "Blueprints.hpp"
#include "HackLevel.hpp"
#include "Point.hpp"
#include "Power.hpp"

struct Weapon
{
	static constexpr int HARDCODED_MINIMUM_BEAM_AIM_DISTANCE = 11;

	int slot = -1;
	std::pair<float, float> cooldown{ 0.f, 0.f };
	WeaponBlueprint blueprint;
	Power power;
	bool player = false;
	bool autofire = false;
	bool fireWhenReady = false;
	bool artillery = false;
	bool targetingPlayer = false;
	bool cargo = false;
	float firingAngle = 0.f;
	float entryAngle = 0.f;
	Point<int> mount;
	HackLevel hackLevel = HackLevel::None;
	std::pair<int, int> boost{ 0, 0 };
	std::pair<int, int> charge{ 0, 0 };
	std::pair<float, float> shotTimer{ 0.f, 0.f };
	std::vector<Point<float>> targetPoints;

	bool powered() const
	{
		return this->power.total.first >= this->power.required;
	}
};
