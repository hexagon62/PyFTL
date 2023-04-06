#pragma once

#include "Point.hpp"
#include "WeaponBlueprint.hpp"
#include "DroneBlueprint.hpp"
#include "HackLevel.hpp"

#include <array>
#include <optional>

struct SpaceDroneMovementExtra
{
	Point<float> destinationLast;
	float progress = 0.f;
	float heading = 0.f, headingLast = 0.f;
};

struct SpaceDroneInfo
{
	bool playerSpace = false, playerSpaceIsDestination = false;

	bool moving = false;
	Point<float> position, positionLast, destination;
	Point<float> speed;

	float pause = 0.f;
	std::pair<float, float> cooldown{ 0.f, 0.f };
	float angle = 0.f, angleDesired = 0.f, angleMalfunction = 0.f;
	float ionTime = 0.f;

	std::optional<WeaponBlueprint> weapon;
	std::optional<SpaceDroneMovementExtra> extraMovement;
};

struct Drone
{
	static constexpr float HARDCODED_REBUILD_TIME = 10.f;
	static constexpr float HARDCODED_SAFE_MALFUNCTION_TIME = 2.f;

	DroneBlueprint blueprint;
	bool player = false;
	bool powered = false;
	bool dead = false;
	int zoltanPower = 0;
	HackLevel hackLevel = HackLevel::None;
	float hackTime = 0.f;
	std::pair<float, float> destroyTimer{ 0.f, 0.f };

	std::optional<SpaceDroneInfo> space;
};

struct HackingDrone : Drone
{
	Point<float> start, goal;
	bool arrived = false, setUp = false;
	int room = -1;
};
