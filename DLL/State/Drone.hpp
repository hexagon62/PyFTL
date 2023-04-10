#pragma once

#include "Point.hpp"
#include "WeaponBlueprint.hpp"
#include "DroneBlueprint.hpp"
#include "Crew.hpp"
#include "HackLevel.hpp"
#include "Power.hpp"

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

	int slot = -1;
	DroneBlueprint blueprint;
	Power power;
	bool player = false;
	bool dead = false;
	bool dying = false;
	bool deployed = false;
	HackLevel hackLevel = HackLevel::None;
	float hackTime = 0.f;
	std::pair<float, float> destroyTimer{ 0.f, 0.f };

	std::optional<SpaceDroneInfo> space;
	std::optional<Crew> crew;

	bool powered() const
	{
		return this->power.total.first == this->power.total.second;
	}

	bool crewDrone() const
	{
		return
			this->blueprint.type == DroneType::AntiBoarder ||
			this->blueprint.type == DroneType::SystemRepair;
	}

	bool spaceDrone() const
	{
		return
			this->blueprint.type == DroneType::Combat ||
			this->blueprint.type == DroneType::Defense ||
			this->blueprint.type == DroneType::HullRepair ||
			this->blueprint.type == DroneType::Hacking ||
			this->blueprint.type == DroneType::Shield;
	}

	bool needsEnemy() const
	{
		return
			this->blueprint.type == DroneType::Combat ||
			this->blueprint.type == DroneType::Boarder ||
			this->blueprint.type == DroneType::Hacking;
	}
};

struct HackingDrone : Drone
{
	Point<float> start, goal;
	bool arrived = false, setUp = false;
	int room = -1;
};
