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
	std::vector<Rect<int>> pages;
	int currentPage = -1;

	std::optional<Rect<int>> fuel;
	std::optional<Rect<int>> missiles;
	std::optional<Rect<int>> droneParts;

	Rect<int> repair;
	Rect<int> repairAll;

	std::vector<Rect<int>> boxes;

	std::optional<ConfirmUIState> confirm;
};

struct SystemUpgradeUIState
{
	Rect<int> box;
	int upgradeTo = -1;
};

struct UpgradesUIState
{
	std::optional<SystemUpgradeUIState> shields;
	std::optional<SystemUpgradeUIState> engines;
	std::optional<SystemUpgradeUIState> oxygen;
	std::optional<SystemUpgradeUIState> weapons;
	std::optional<SystemUpgradeUIState> drones;
	std::optional<SystemUpgradeUIState> medbay;
	std::optional<SystemUpgradeUIState> piloting;
	std::optional<SystemUpgradeUIState> sensors;
	std::optional<SystemUpgradeUIState> doorControl;
	std::optional<SystemUpgradeUIState> teleporter;
	std::optional<SystemUpgradeUIState> cloaking;
	std::vector<SystemUpgradeUIState> artillery;
	std::optional<SystemUpgradeUIState> battery;
	std::optional<SystemUpgradeUIState> clonebay;
	std::optional<SystemUpgradeUIState> mindControl;
	std::optional<SystemUpgradeUIState> hacking;
	SystemUpgradeUIState reactor;
	Rect<int> undo;
	Rect<int> accept;

	bool hasSystem(SystemType type, int which = 0) const
	{
		return ::hasSystem(*this, type, which);
	}

	const SystemUpgradeUIState& getSystem(SystemType type, int which = 0) const
	{
		return ::getSystem<const SystemUpgradeUIState&>(*this, type, which);
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
	std::vector<Rect<int>> weapons;
	std::vector<Rect<int>> drones;
	std::vector<Rect<int>> augments;
	std::vector<Rect<int>> storage;
	std::optional<Rect<int>> discard;
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

struct GameOverUIState
{
	Rect<int> stats;
	Rect<int> restart;
	Rect<int> hangar;
	Rect<int> mainMenu;
	Rect<int> quit;
};

struct MenuUIState
{
	Rect<int> continueButton;
	Rect<int> mainMenu;
	Rect<int> hangar;
	Rect<int> restart;
	Rect<int> options;
	Rect<int> controls;
	Rect<int> quit;
	Rect<int> difficulty;
	Rect<int> aeEnabled;
	std::vector<Rect<int>> achievements;
	bool showControls = false;
	std::optional<ConfirmUIState> confirm;
};

struct GameUIState
{
	static constexpr Point<int> HARDCODED_ARMAMENT_BOX_SIZE{ 95, 39 };

	Point<int> playerShip;
	Point<int> enemyShip;

	Rect<int> ftl;
	Rect<int> shipButton;
	Rect<int> menuButton;
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

	std::optional<Rect<int>> upgradesTab;
	std::optional<Rect<int>> crewTab;
	std::optional<Rect<int>> cargoTab;

	std::optional<UpgradesUIState> upgrades;
	std::optional<CrewManifestUIState> crewMenu;
	std::optional<ConfirmUIState> leaveCrew;
	std::optional<CargoUIState> cargo;
	std::optional<StarMapUIState> starMap;
	std::optional<EventUIState> event;
	std::optional<StoreUIState> store;
	std::optional<MenuUIState> menu;
	std::optional<GameOverUIState> gameOver;

	bool hasSystem(SystemType type, int which = 0) const
	{
		return ::hasSystem(*this, type, which);
	}

	const Rect<int>& getSystem(SystemType type, int which = 0) const
	{
		return ::getSystem<const Rect<int>&>(*this, type, which);
	}
};

struct MainMenuUIState
{
	Rect<int> continueButton;
	Rect<int> newGame;
	Rect<int> tutorial;
	Rect<int> stats;
	Rect<int> options;
	Rect<int> credits;
	Rect<int> quit;
	std::optional<ConfirmUIState> confirm;
};

struct ShipBoxUIState
{
	Rect<int> box;
	std::vector<Rect<int>> achievements;
};

struct ShipListUIState
{
	std::vector<ShipBoxUIState> ships;
	Rect<int> layoutA, layoutB, layoutC;
};

struct HangarCrewBoxUIState
{
	Rect<int> rename;
	Rect<int> customize;
};

struct CustomizeCrewUIState
{
	Rect<int> previousStyle, nextStyle;
	Rect<int> rename;
	Rect<int> accept;
};

struct HangarUIState
{
	Rect<int> rename;
	Rect<int> previousShip, nextShip;
	Rect<int> listShips;
	Rect<int> randomShip;
	Rect<int> layoutA, layoutB, layoutC;
	Rect<int> hideRooms;
	std::vector<Rect<int>> achievements;
	std::vector<HangarCrewBoxUIState> crew;
	std::optional<CustomizeCrewUIState> crewCustomization;
	Rect<int> easy, normal, hard;
	Rect<int> start;
	Rect<int> disableAE;
	Rect<int> enableAE;
};

struct StatsUIState
{

};

struct OptionsUIState
{

};

struct CreditsUIState
{

};

using SystemUIRef = std::reference_wrapper<const System>;
using WeaponUIRef = std::reference_wrapper<const Weapon>;

struct MouseState
{
	Point<int> position, positionLast;
	std::optional<Point<int>> dragFrom;

	std::variant<std::monostate, SystemUIRef, WeaponUIRef> aiming;
	std::optional<bool> autofire;
};

struct UIState
{
	MouseState mouse;
	std::optional<GameUIState> game;
	std::optional<MainMenuUIState> menu;
	std::optional<HangarUIState> hangar;
	std::optional<StatsUIState> stats;
	std::optional<OptionsUIState> options;
	std::optional<CreditsUIState> credits;
};
