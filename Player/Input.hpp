#pragma once

#include "../GameState/State.hpp"

#include <memory>
#include <queue>

class StateReader;

class Input
{
public:
	Input(const StateReader& reader);
	~Input();

	void iterate();
	
	void openJumpMenu();
	void openUpgradeMenu();
	void openStore();

	void selectCrew(int crew, bool shift = false);
	void crewToRoom(int room, bool ownShip = true);
	void saveStations();
	void returnToStations();
	void crystalAbility(int crew);

	void changePower(SystemID which, int amount, bool set = true);
	void toggleWeapon(int which, bool on);
	void toggleDrone(int which, bool on);

	void moveWeapon(int from, int to);
	void moveDrone(int from, int to);

	// Offset is in tiles.
	// Offsets that fall outside the room are moved to the center in the respective dimension.
	struct WeaponFireOptions
	{
		int weapon = -1, roomA = -1, roomB = -1;
		bool ownShip = false, autoFire = false;
		Pointf offsetA = { -1.f, -1.f }, offsetB = { -1.f, -1.f };
	};

	void fireAtRoom(const WeaponFireOptions& options);

	void toggleDoor(int door, bool open);
	void openAllDoors(bool airlocks = false);
	void closeAllDoors();

	void teleport(int room, bool retrieve);
	void cloak();
	void battery();
	void mind(int room, bool ownShip);
	void hack();

	void deployHackingDrone(SystemID system, int artilleryIndex = -1);

	void wait(int iterations = 1);

	bool idle() const;

	static constexpr int GameWidth = 1280, GameHeight = 720;

private:
	const StateReader& reader;

	struct FTLCommand
	{
		enum Type
		{
			Nothing,
			OpenJumpMenu, OpenUpgradeMenu, OpenStore,
			SelectCrew, CrewToRoom, SaveStations, ReturnToStations, CrystalAbility,
			ChangePower, ToggleWeapon, ToggleDrone,
			MoveWeapon, MoveDrone,
			FireAtRoom,
			ToggleDoor, OpenAllDoors, CloseAllDoors,
			Teleport, Cloak, Battery, Mind, Hack,
			DeployHackingDrone
		};

		struct CrewSelection
		{
			int id = -1;
			bool shift = false;
		};

		struct RoomSelection
		{
			int id = -1;
			bool ownShip = true;
		};

		struct PowerChange
		{
			SystemID system = SystemID::Invalid;
			int amount = 0;
			bool set = false;
		};

		struct SlottedPowerChange
		{
			int slot = -1;
			bool on = false;
		};

		struct SlotSwap
		{
			int from = -1, to = -1;
		};

		struct DoorChange
		{
			int id = -1;
			bool open = false;
		};

		struct AllDoorChange
		{
			bool airlocks = false;
		};

		struct Teleportation
		{
			int room = -1;
			bool retrieve = false;
		};

		struct HackingDroneDeployment
		{
			SystemID system = SystemID::Invalid;
			int artilleryIndex = -1;
		};

		Type type;

		union
		{
			int wait;
			CrewSelection crewSelection;
			RoomSelection roomSelection;
			PowerChange powerChange;
			SlottedPowerChange slotPowerChange;
			SlotSwap slotSwap;
			WeaponFireOptions weaponFire;
			DoorChange doorChange;
			AllDoorChange allDoorChange;
			Teleportation teleportation;
			HackingDroneDeployment hackingDroneDeployment;
		};
	};

	std::queue<FTLCommand> queue;

	class Impl;
	std::unique_ptr<Impl> impl;

	Pointi systemPos(SystemID which);
	Pointi crewPos(int which);
	Pointi weaponSlotPos(int which);
	Pointi droneSlotPos(int which);
	Pointi roomPos(int which, bool ownShip = true, const Pointf& offset = { -1.f, -1.f });

	struct MouseOptions
	{
		bool right = false, shift = false, ctrl = false;
	};

	struct InputCommand
	{
		enum Type
		{
			Nothing, MouseMove, Click
		};

		struct MouseClick
		{
			bool down = false, up = false;
			MouseOptions options;
		};

		Type type;

		union
		{
			int wait;
			Pointi position;
			MouseClick mouseClick;
		};
	};
};
