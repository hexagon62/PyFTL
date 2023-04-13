#pragma once

#include "Point.hpp"
#include "Rect.hpp"
#include "System.hpp"
#include "Weapon.hpp"

#include <optional>
#include <variant>
#include <functional>

struct ConfirmUIState
{
	Rect<int> yes, no;
};

struct StoreUIState
{
	Rect<int> close;

	Rect<int> buy;
	Rect<int> sell;
	std::optional<Rect<int>> page1;
	std::optional<Rect<int>> page2;

	std::optional<Rect<int>> buyFuel;
	std::optional<Rect<int>> buyMissiles;
	std::optional<Rect<int>> buyDroneParts;

	Rect<int> repair;
	Rect<int> repairAll;

	std::vector<Rect<int>> boxes;

	Rect<int> sellBox;
	std::optional<ConfirmUIState> confirm;
};

struct UpgradesUIState
{
	std::optional<Rect<int>> shields;
	std::optional<Rect<int>> engines;
	std::optional<Rect<int>> oxygen;
	std::optional<Rect<int>> weapons;
	std::optional<Rect<int>> drones;
	std::optional<Rect<int>> medbay;
	std::optional<Rect<int>> piloting;
	std::optional<Rect<int>> sensors;
	std::optional<Rect<int>> doorControl;
	std::optional<Rect<int>> teleporter;
	std::optional<Rect<int>> cloaking;
	std::vector<Rect<int>> artillery;
	std::optional<Rect<int>> battery;
	std::optional<Rect<int>> clonebay;
	std::optional<Rect<int>> mindControl;
	std::optional<Rect<int>> hacking;
	Rect<int> reactor;
	Rect<int> undo;
	Rect<int> accept;

	bool hasSystem(SystemType type, int which = 0) const
	{
		return ::hasSystem(*this, type, which);
	}

	const Rect<int>& getSystem(SystemType type, int which = 0) const
	{
		return ::getSystem<const Rect<int>&>(*this, type, which);
	}
};

struct CrewManifestBoxUIState
{
	Rect<int> box;
	Rect<int> rename;
	Rect<int> dismiss;
};

struct CrewManifestUIState
{
	std::vector<CrewManifestBoxUIState> boxes;
	std::optional<ConfirmUIState> confirm;
	Rect<int> accept;
};

struct CargoUIState
{
	std::vector<Rect<int>> boxes;
	std::optional<Rect<int>> overCapacity;
	Rect<int> accept;
};

struct CrewBoxUIState
{
	Rect<int> box;
	Rect<int> skills;
};

struct StarMapUIState
{
	Rect<int> back;
	std::optional<Rect<int>> wait;
	std::optional<Rect<int>> distress;
	std::optional<Rect<int>> nextSector;
};

struct EventChoiceUIState
{
	std::string text;
	Rect<int> box;
};

struct EventUIState
{
	static constexpr float HARDCODED_BRIEF_DELAY_TIME = 0.3f;

	std::string text;
	std::vector<EventChoiceUIState> choices;
	std::pair<float, float> openTime;
};

struct GameUIState
{
	static constexpr Point<int> HARDCODED_ARMAMENT_BOX_SIZE{ 95, 39 };

	Point<int> playerShip;
	Point<int> enemyShip;

	Rect<int> ftl;
	Rect<int> shipMenu;
	Rect<int> menu;
	std::optional<Rect<int>> storeButton;

	std::vector<CrewBoxUIState> crewBoxes;
	Rect<int> saveStations;
	Rect<int> loadStations;

	Rect<int> reactor;
	std::optional<Rect<int>> shields;
	std::optional<Rect<int>> engines;
	std::optional<Rect<int>> oxygen;
	std::optional<Rect<int>> weapons;
	std::optional<Rect<int>> drones;
	std::optional<Rect<int>> medbay;
	std::optional<Rect<int>> piloting;
	std::optional<Rect<int>> sensors;
	std::optional<Rect<int>> doorControl;
	std::optional<Rect<int>> teleporter;
	std::optional<Rect<int>> cloaking;
	std::vector<Rect<int>> artillery;
	std::optional<Rect<int>> battery;
	std::optional<Rect<int>> clonebay;
	std::optional<Rect<int>> mindControl;
	std::optional<Rect<int>> hacking;

	std::vector<Rect<int>> weaponBoxes;
	std::optional<Rect<int>> autofire;
	std::vector<Rect<int>> droneBoxes;
	std::optional<Rect<int>> openAllDoors;
	std::optional<Rect<int>> closeAllDoors;
	std::optional<Rect<int>> teleportSend;
	std::optional<Rect<int>> teleportReturn;
	std::optional<Rect<int>> startCloak;
	std::optional<Rect<int>> startBattery;
	std::optional<Rect<int>> startMindControl;
	std::optional<Rect<int>> startHack;

	std::optional<UpgradesUIState> upgrades;
	std::optional<CrewManifestUIState> crewManifest;
	std::optional<ConfirmUIState> leaveCrew;
	std::optional<CargoUIState> cargo;
	std::optional<StarMapUIState> starMap;
	std::optional<EventUIState> event;
	std::optional<StoreUIState> store;

	bool hasSystem(SystemType type, int which = 0) const
	{
		return ::hasSystem(*this, type, which);
	}

	const Rect<int>& getSystem(SystemType type, int which = 0) const
	{
		return ::getSystem<const Rect<int>&>(*this, type, which);
	}
};

struct MouseState
{
	Point<int> position, positionLast;
	std::optional<Point<int>> dragFrom;

	std::variant<
		std::monostate,
		std::reference_wrapper<const System>,
		std::reference_wrapper<const Weapon>
	> aiming;
};

struct UIState
{
	MouseState mouse;
	std::optional<GameUIState> game;
};
