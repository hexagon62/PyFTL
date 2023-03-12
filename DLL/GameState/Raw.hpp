#pragma once
#pragma once

#include <cstdint>
#include <utility>

namespace raw
{

using vptr = uint8_t[4];

template<typename T>
struct Vector
{
	T* begin = nullptr;
	T* end = nullptr;
	T* storageEnd = nullptr;

	size_t size() const
	{
		return (reinterpret_cast<size_t>(this->end) - reinterpret_cast<size_t>(this->begin)) / sizeof(T);
	}

	T& operator[](size_t i)
	{
		return this->begin[i];
	}

	const T& operator[](size_t i) const
	{
		return this->begin[i];
	}
};

struct ShipObject
{
	vptr _vptr;
	int iShipId = 0;
};

struct Targetable
{
	vptr _vptr;
	int type = 0;
	bool hostile = false;
	bool targeted = false;
	uint8_t u0[2];
};

struct Collideable
{
	vptr _vptr;
};

struct ShipSystem
{

};

struct OxygenSystem
{

};

struct TeleportSystem
{

};

struct CloakingSystem
{

};

struct BatterySystem
{

};

struct MindSystem
{

};

struct CloneSystem
{

};

struct HackingSystem
{

};

struct Shields
{

};

struct WeaponSystem
{

};

struct DroneSystem
{

};

struct MedbaySystem
{

};

struct EngineSystem
{

};

struct ArtillerySystem
{

};

struct CrewMember
{

};

template<typename T>
struct Spreader : ShipObject
{
	int count = 0;
	Vector<int> roomCount;
	Vector<T> grid;
};

struct Selectable
{
	vptr _vptr;
	int selectedState = 0;
};

struct Point
{
	int x = 0, y = 0;
};

struct Repairable : Selectable, ShipObject
{
	float fDamage = 0.f;
	Point pLoc;
	float fMaxDamage = 0.f;
	char* name = nullptr;
	int roomId = 0;
	int iRepairCount = 0;
};

struct Spreadable
{
	char* soundName = nullptr;
};

struct Fire : Spreadable
{
	float fDeathTimer = 0.f;
	float fStartTimer = 0.f;
	float fOxygen = 0.f;
	uint8_t u0[288];
	bool bWasOnFire;
};

struct ShipManager : ShipObject, Targetable, Collideable
{
	Vector<ShipSystem*> vSystemList;
	OxygenSystem* oxygenSystem = nullptr;
	TeleportSystem* teleportSystem = nullptr;
	CloakingSystem* cloakSystem = nullptr;
	BatterySystem* batterySystem = nullptr;
	MindSystem* mindSystem = nullptr;
	CloneSystem* cloneSystem = nullptr;
	HackingSystem* hackingSystem = nullptr;
	bool showNetwork = false;
	bool addedSystem = false;
	uint8_t u0[2];
	Shields* shieldSystem = nullptr;
	WeaponSystem* weaponSystem = nullptr;
	DroneSystem* droneSystem = nullptr;
	MedbaySystem* medbaySystem = nullptr;
	EngineSystem* engineSystem = nullptr;
	Vector<ArtillerySystem*> artillerySystems;
	Vector<CrewMember*> vCrewList;
	Spreader<Fire> fireSpreader;
	uint8_t u1[0x3f8];
	std::pair<float, float> jump_timer = { 0.f, 0.f };
};

struct SpaceManager
{

};

struct CompleteShip
{
	vptr _vptr;
	int iShipId = 0;
	ShipManager* shipManager = nullptr;
	SpaceManager* spaceManager = nullptr;
	bool bPlayerShip = false;
	uint8_t u0[3];

};

struct ShipStatus
{
	Point location;
	float size;
	ShipManager* ship = nullptr;
};

struct CommandGui
{
	ShipStatus shipStatus;
};

struct WorldManager
{

};

struct MainMenu
{
	bool open = false;
};

struct CApp
{
	uint8_t u0[4];
	bool Running = false;
	bool shift_held = false;
	uint8_t u1[2];
	CommandGui* gui = nullptr;
	WorldManager* world = nullptr;
	MainMenu menu;
};

struct State
{
	CApp* app = nullptr;
};

constexpr uintptr_t CAppBase = 0x4C5020;

}
