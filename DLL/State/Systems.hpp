#pragma once

#include "System.hpp"
#include "Ellipse.hpp"
#include "Crew.hpp"
#include "WeaponBlueprint.hpp"
#include "HackLevel.hpp"
#include "Weapon.hpp"
#include "Drone.hpp"

#include <utility>
#include <vector>

struct ShieldSystem : System
{
	Ellipse<float> boundary;
	std::pair<int, int> bubbles{ 0, 0 };
	std::pair<float, float> charge{ 0.f, 0.f };
};

struct EngineSystem : System
{
	static constexpr std::array<int, 9> HARDCODED_EVASION_VALUES{
		0, 5, 10, 15, 20, 25, 28, 31, 35
	};

	static constexpr std::array<int, 4> HARDCODED_EVASION_SKILL_BOOST{
		0, 5, 7, 10
	};

	bool boostFTL = false;
};

struct MedbaySystem : System
{
	int slot = -1; // bay takes up a slot
};

struct ClonebaySystem : System
{
	static constexpr float HARDCODED_DEATH_TIME = 3.f;

	std::vector<Crew*> queue;
	std::pair<float, float> cloneTimer{ 0.f, 0.f };
	std::pair<float, float> deathTimer{ 0.f, 0.f };
	int slot = -1; // bay takes up a slot
};

struct OxygenSystem : System
{
	// Probably no extra fields?
};

struct TeleporterSystem : System
{
	int slots = 0;
	int targetRoom = -1;
	std::vector<Crew*> crewPresent;
	bool sending = false;
	bool receiving = false;
	bool canSend = false;
	bool canReceive = false;

	bool operable() const
	{
		return this->System::operable() &&
			this->power.ionLevel == 0 &&
			(this->canSend || this->canReceive);
	}
};

struct CloakingSystem : System
{
	static constexpr int HARDCODED_EVASION_BONUS = 60;

	bool on = false;
	std::pair<float, float> timer{ 0.f, 0.f };

	bool operable() const
	{
		return this->System::operable() &&
			this->power.ionLevel == 0 &&
			this->timer.first >= this->timer.second;
	}
};

struct ArtillerySystem : System
{
	Weapon weapon;
};

struct MindControlSystem : System
{
	bool on = false;
	bool targetingPlayerShip = false;
	bool canUse = false;
	std::pair<float, float> timer{ 0.f, 0.f };
	int targetRoom = -1;

	bool operable() const
	{
		return this->System::operable() &&
			this->power.ionLevel == 0 &&
			this->timer.first >= this->timer.second &&
			this->canUse;
	}
};

struct HackingSystem : System
{
	bool on = false;
	bool canUse = false;
	std::pair<float, float> timer{ 0.f, 0.f };
	SystemType target = SystemType::None, queued = SystemType::None;
	HackingDrone drone;

	bool operable() const
	{
		return this->System::operable() &&
			this->power.ionLevel == 0 &&
			this->timer.first >= this->timer.second &&
			this->canUse;
	}
};

struct WeaponSystem : System
{
	std::vector<Weapon> list;
	int slotCount = 0;
	bool autoFire = false;
};

struct DroneSystem : System
{
	std::vector<Drone> list;
	int slotCount = 0;
};

struct PilotingSystem : System
{
	static constexpr std::array<int, 4> HARDCODED_EVASION_SKILL_BOOST{
		0, 5, 7, 10
	};
};

struct SensorSystem : System
{
	// Probably no extra fields?
};

struct DoorSystem : System
{
	// Probably no extra fields?
};

struct BatterySystem : System
{
	bool on = false;
	std::pair<float, float> timer{ 0.f, 0.f };
	int provides = 0, providing = 0;

	bool operable() const
	{
		return this->System::operable() &&
			this->power.ionLevel == 0 &&
			this->timer.first >= this->timer.second;
	}
};
