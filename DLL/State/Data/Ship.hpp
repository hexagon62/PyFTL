#pragma once

#include "Room.hpp"
#include "Door.hpp"
#include "Crew.hpp"
#include "Augment.hpp"
#include "Systems.hpp"

#include <vector>
#include <utility>
#include <array>
#include <optional>

struct Cargo
{
	int scrap = 0, fuel = 0, missiles = 0, droneParts = 0;

	std::vector<Weapon> weapons;
	std::vector<Drone> drones;
	std::vector<Augment> augments;
};

struct Reactor
{
	std::pair<int, int> total{ 0, 0 };
	std::pair<int, int> normal{ 0, 0 };
	std::pair<int, int> battery{ 0, 0 };
	int level = 0, cap = 0;
};

struct Ship
{
	bool player = false;
	bool destroyed = false;
	bool automated = false;
	bool jumping = false;
	bool canJump = false;
	bool canInventory = false;

	std::pair<float, float> jumpTimer{ 0.f, 0.f };

	std::pair<int, int> hull{ 0, 0 };
	std::pair<int, int> superShields{ 0, 0 };

	int evasion = 0;
	float totalOxygen = 0.f;

	std::vector<Room> rooms;
	std::vector<Door> doors;

	Reactor reactor;
	std::optional<ShieldSystem> shields;
	std::optional<EngineSystem> engines;
	std::optional<MedbaySystem> medbay;
	std::optional<ClonebaySystem> clonebay;
	std::optional<OxygenSystem> oxygen;
	std::optional<TeleporterSystem> teleporter;
	std::optional<CloakingSystem> cloaking;
	std::vector<ArtillerySystem> artillery;
	std::optional<MindControlSystem> mindControl;
	std::optional<HackingSystem> hacking;
	std::optional<WeaponSystem> weapons;
	std::optional<DroneSystem> drones;
	std::optional<PilotingSystem> piloting;
	std::optional<SensorSystem> sensors;
	std::optional<DoorSystem> doorControl;
	std::optional<BatterySystem> battery;

	Cargo cargo;
};