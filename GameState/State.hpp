#pragma once

#include <utility>
#include <vector>
#include <array>
#include <unordered_map>

#include "../Utility/Geometry.hpp"

enum class SystemID : int
{
	Invalid = -1,
	Shields = 0,
	Engines,
	Oxygen,
	Weapons,
	Drones,
	Medbay,
	Pilot,
	Sensors,
	Doors,
	Teleporter,
	Cloaking,
	Artillery,
	Battery,
	Clonebay,
	Mind,
	Hacking
};

// In the same order as on the UI
constexpr std::array SystemOrder
{
	SystemID::Shields,
	SystemID::Engines,
	SystemID::Medbay,
	SystemID::Clonebay,
	SystemID::Oxygen,
	SystemID::Teleporter,
	SystemID::Cloaking,
	SystemID::Artillery,
	SystemID::Mind,
	SystemID::Hacking,
	SystemID::Weapons,
	SystemID::Drones,
	SystemID::Pilot,
	SystemID::Sensors,
	SystemID::Doors,
	SystemID::Battery
};

constexpr size_t SystemTypes = 16;
constexpr size_t PowerableSystemTypes = 12;

struct SystemPowerState
{
	int total = 0;
	int reactor = 0;
	int zoltan = 0;
	int battery = 0;
	int limit = 0;
};

struct SystemState
{
	SystemPowerState power;
	bool present = false;

	int room = -1;
	int manningLevel = 0;
	int hackLevel = 0;
	int health = 0;
	std::pair<int, int> level{ 0, 0 };
	std::pair<int, float> ion{ 0, 0.f };
	std::pair<float, float> partialHP{ 0.f, 0.f };
};

struct ShieldsState : SystemState
{
	int bubbles = 0;
	float charge = 0.f;
	Pointi center;
	std::pair<float, float> ellipse;
};

struct EnginesState : SystemState
{
	bool ftlBoost = false;
};

struct OxygenState : SystemState
{
	float max = 0.f, total = 0.f;
	std::vector<float> levels;
	bool leaking = false;
};

struct WeaponState
{
	uintptr_t ptr = 0;
	char blueprint[32]{};
	std::pair<float, float> cooldown{ 0.f, 0.f };
	bool autoFire = false, goingToFire = false;
	bool powered = false;
	int powerRequired = 0;
	float angle = 0.f;
	float cooldownModifier = 0.f;
	int radius = 0;
	std::pair<int, int> charge{ 0, 0 };
	bool artillery = false;
	int slot = -1;
};

struct WeaponControlState : SystemState
{
	std::vector<WeaponState> list;
	int slots = 0;
};

struct DroneState
{
	uintptr_t ptr = 0;
	int shipAffiliation = -1;
	char blueprint[32]{};
	bool powered = false, deployed = false;
	int powerRequired = 0;
	float destroyTimer = 0.f;
	int slot = -1;
};

struct DroneControlState : SystemState
{
	std::vector<DroneState> list;
	int slots = 0;
};

struct TeleporterState : SystemState
{
	std::pair<int, int> crew{ 0, 0 };
	bool canSend = false, canReceive = false;
};

struct CloakingState : SystemState
{
	bool on = false;
	std::pair<float, float> timer{ 0.f, 0.f };
};

struct ArtilleryState : SystemState
{
	WeaponState weapon;
};

struct BatteryState : SystemState
{
	bool on = false;
	std::pair<float, float> timer{ 0.f, 0.f };
};

struct ClonebayState : SystemState
{
	std::pair<float, float> timer{ 0.f, 0.f };
	float deathTimer = 0.f;
};

struct MindState : SystemState
{
	std::pair<float, float> timer{ 0.f, 0.f };
	bool canUse = false, armed = false;
};

struct HackingState : SystemState
{
	bool deployed = false, arrived = false, canUse = false;
	std::pair<float, float> timer{ 0.f, 0.f };
	Pointf start, destination;
};

struct SystemStates
{
	ShieldsState shields;
	EnginesState engines;
	OxygenState oxygen;
	WeaponControlState weapons;
	DroneControlState drones;
	SystemState medbay;
	SystemState pilot;
	SystemState sensors;
	SystemState doors;
	TeleporterState teleporter;
	CloakingState cloaking;
	BatteryState battery;
	ClonebayState clonebay;
	MindState mind;
	HackingState hacking;

	std::vector<ArtilleryState> artillery;

	const SystemState* get(SystemID what, size_t artilleryIndex = 0) const
	{
		auto ifPresent = [&](auto& s) {
			return s.present ? &s : nullptr;
		};

		switch (what)
		{
		case SystemID::Shields: return ifPresent(this->shields);
		case SystemID::Engines: return ifPresent(this->engines);
		case SystemID::Oxygen: return ifPresent(this->oxygen);
		case SystemID::Weapons: return ifPresent(this->weapons);
		case SystemID::Drones: return ifPresent(this->drones);
		case SystemID::Medbay: return ifPresent(this->medbay);
		case SystemID::Pilot: return ifPresent(this->pilot);
		case SystemID::Sensors: return ifPresent(this->sensors);
		case SystemID::Doors: return ifPresent(this->doors);
		case SystemID::Teleporter: return ifPresent(this->teleporter);
		case SystemID::Cloaking: return ifPresent(this->cloaking);
		case SystemID::Artillery: 
			if (this->artillery.empty())
				return nullptr;
			return &this->artillery.at(artilleryIndex);
		case SystemID::Battery: return ifPresent(this->battery);
		case SystemID::Clonebay: return ifPresent(this->clonebay);
		case SystemID::Mind: return ifPresent(this->mind);
		case SystemID::Hacking: return ifPresent(this->hacking);
		}

		return nullptr;
	}
};

struct CrewState
{
	int shipAffiliation = -1;
	Pointf position;
	int roomId = -1, shipId = -1;
	int manning = -1, repairing = -1;
	bool dead = false;
	std::pair<float, float> health;
	Pointf pathStart;
	Pointf pathDestination;
	int destinationRoomId = -1;
	int destinationSlotId = -1;
	float pathLength = -1.f;
	Pointf next;
	bool inDoor = false;
	bool suffocating = false;
	bool fighting = false;
	bool melee = false;
	char species[32]{}, name[32]{};
	bool mindControlled = false;
	int healthBoost = 0;
	float damageBoost = 1.f;
	float cloneDying = 0.f;
	bool cloning = false;
	float stun = 0.f;
	int guiSlot = -1;
};

constexpr int RoomTileLength = 35;

struct RoomState
{
	SystemID system = SystemID::Invalid;
	Pointi position;
	Pointi size; // in tiles
	int roomId = -1;
	int computer = -1;
	int hackLevel = 0;
};

struct DoorState
{
	int doorId = -1, roomA = -1, roomB = -1;
	bool open = false;
	int baseHealth = 0, health = 0;
	int level = 0;
	bool ioned = false;
	int hackLevel = 0;
	Pointi position;
	bool vertical = false;
	bool airlock = false;
};

struct ShipState
{
	SystemStates systems;
	std::unordered_map<int, RoomState> rooms;
	std::unordered_map<int, DoorState> doors;
	std::pair<int, int> reactor;
	int reactorLimit = 1000;
	std::pair<int, int> hull;
	int fuel = 0, missiles = 0, droneParts = 0, scrap = 0;
	bool jumping = false;
	std::pair<float, float> ftl;
	std::pair<int, int> superShield;
};

struct Damage
{
	int main = 0, ion = 0, system = 0, crew = 0, stun = 0;
	int pierce = 0;
	int fireChance = 0, breachChance = 0, stunChance = 0;
	bool hullBonus = false, lockdown = false, crystal = false, friendlyFire = false;
};

struct ProjectileState
{
	Pointf position, destination, velocity;
	int space = -1, destinationSpace = -1;
	float heading = 0.f;
	int owner = -1, target = -1;
	bool hit = false, missed = false, passed = false;
	bool dead = false;
	Damage damage;
	float lifespan = 0.f;
};

struct SpaceDroneState
{
	uintptr_t ptr = 0;
	int shipAffiliation = -1;
	int currentSpace = -1, destinationSpacee = -1;
	Pointf position, speed, destination;
	float weaponCooldown = 0.f;
	bool disrupted = false;
	float disruptTimer = 0.f;
	Pointf target, targetSpeed;
	Pointf beamCurrent, beamFinal;
	float beamSpeed = 0.f;
};

struct HazardsState
{
	int target = -1;

	// Asteroids!
	bool asteroids = false;

	std::array<std::pair<int, int>, 3> 
		asteroidSpawnRates{ { {0, 0}, {0, 0}, {0, 0} } },
		asteroidStateLengths{ { {0, 0}, {0, 0}, {0, 0} } };

	int shipCount = -1;
	int asteroidState = 0, asteroidSpace = 0;
	float asteroidVolleyTimer = 0.f, asteroidTimer = 0.f;
	int shieldInit = 0;

	// Other hazards, which are simpler
	bool sun = false, pulsar = false, pds = false;

	std::pair<float, float> timer;

	// Non-timed hazards
	bool nebula = false, storm = false;
};

struct PauseState
{
	bool normal = false, automatic = false;
	bool menu = false, event = false, touch = false;
	bool any = false;
};

struct State
{
	bool running = false;
	bool minimized = false;
	bool focused = false;

	bool inGame = false;
	bool justLoaded = false;

	bool targetPresent = false;
	bool store = false;

	HazardsState hazards;

	std::vector<ProjectileState> projectiles;
	std::vector<SpaceDroneState> spaceDrones;

	std::vector<CrewState> crew;
	int playerCrewCount = 0, targetCrewCount = 0;
	
	Pointi shipPosition, targetShipPosition;

	PauseState pause;
	ShipState player, target;
};

struct WeaponBlueprint
{
	char name[16], type[16];
	Damage damage;
	int shots = 0;
	int missiles = 0;
	float cooldown = 0.f;
	int beamLength = 0;
	float speed = 0.f;
};

struct DroneBlueprint
{
	char name[16], type[16];
};

struct KnownBlueprints
{
	std::unordered_map<std::string, WeaponBlueprint> weapons;
	std::unordered_map<std::string, DroneBlueprint> drones;
};
