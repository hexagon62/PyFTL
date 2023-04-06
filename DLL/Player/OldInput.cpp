#include "Input.hpp"
#include "../GameState/Reader.hpp"
#include "../Utility/Geometry.hpp"

#include <cmath>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winuser.h>
#include <dwmapi.h>

extern HWND g_hGameWindow;
extern WNDPROC g_hGameWindowProc;

class Input::Impl
{
public:
	Impl() = default;
	~Impl() {}

	void iterate()
	{
		int cap = 1;

		while (!this->queue.empty() && cap > 0)
		{
			auto& cmd = this->queue.front();

			switch (cmd.type)
			{
			case InputCommand::Nothing:
				if (cmd.wait-- > 0) return;
				break;
			case InputCommand::MouseMove:
			{
				Point<int> p = this->gameToScreen(cmd.position.x, cmd.position.y);
				this->setCursorPos(p.x, p.y);
				break;
			}
			case InputCommand::Click:
				this->mouseInput(cmd.mouseClick);
				break;
			}

			this->queue.pop();
			--cap;
		}
	}

	void mouseMove(const Point<int>& pos)
	{
		this->queue.push({
			.type = InputCommand::MouseMove,
			.position = pos
		});
	}

	void mouseUp(const MouseOptions& options = {})
	{
		this->queue.push({
			.type = InputCommand::Click,
			.mouseClick = {
				.up = true,
				.options = options
			}
		});
	}

	void mouseDown(const MouseOptions& options = {})
	{
		this->queue.push({
			.type = InputCommand::Click,
			.mouseClick = {
				.down = true,
				.options = options
			}
		});
	}

	void mouseClick(const MouseOptions& options = {})
	{
		this->queue.push({
			.type = InputCommand::Click,
			.mouseClick = {
				.down = true,
				.up = true,
				.options = options
			}
		});
	}

	void clickAt(const Point<int>& pos, const MouseOptions& options = {})
	{
		this->mouseMove(pos);
		this->mouseClick(options);
	}

	void wait(int iterations = 1)
	{
		this->queue.push({
			.type = InputCommand::Nothing,
			.wait = iterations
		});
	}

	bool idle() const
	{
		return this->queue.empty();
	}

private:
	std::queue<InputCommand> queue;

	void mouseInput(const InputCommand::MouseClick& cmd)
	{
		auto& [down, up, options] = cmd;
		auto& [right, shift, ctrl] = options;

		if (!down && !up)
			return;

		WPARAM w = (shift ? MK_SHIFT : 0) | (ctrl ? MK_CONTROL : 0);

		if (down)
		{
			UINT uMsg = right
				? WM_RBUTTONDOWN
				: WM_LBUTTONDOWN;

			this->postMessage(uMsg, w, 0);
		}

		if (up)
		{
			UINT uMsg = right
				? WM_RBUTTONUP
				: WM_LBUTTONUP;

			this->postMessage(uMsg, w, 0);
		}
	}

	Point<int> gameToScreen(int x, int y)
	{
		RECT rect;
		GetClientRect(g_hGameWindow, &rect);

		Point<float> scale{
			float(rect.right)/float(Input::GameWidth),
			float(rect.bottom)/float(Input::GameHeight)
		};

		scale.x *= float(x);
		scale.y *= float(y);

		Point<int> result = { int(rect.left) + int(scale.x), int(rect.top) + int(scale.y) };

		if (result.x < rect.left) result.x = rect.left;
		if (result.y < rect.top) result.y = rect.top;
		if (result.x >= rect.right) result.x = rect.right - 1;
		if (result.y >= rect.bottom) result.y = rect.bottom - 1;

		return { int(result.x), int(result.y) };
	}

	void setCursorPos(int x, int y)
	{
		LPARAM l = (x & 0xffff) | ((y & 0xffff) << 16);
		this->postMessage(WM_MOUSEMOVE, 0, l);
	}

	void postMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		PostMessage(g_hGameWindow, uMsg | WM_FTLAI, wParam, lParam);
	}
};

// Thanks windows
#undef min

namespace
{

// Misc buttons
constexpr Point<int> JumpButton{ 572, 49 };
constexpr Point<int> UpgradeButton{ 661, 49 };
constexpr Point<int> StoreButton{ 753, 49 };


// Crew stuff
constexpr Point<int> FirstCrewButton{ 53, 168 };
constexpr Point<int> CrewButtonDelta{ 0, 30 };
constexpr Point<int> CrystalAbilityDelta{ 86, 0 };
constexpr Point<int> SaveStationsButton{ -19, 33 };
constexpr Point<int> ReturnToStationsButton{ 21, 33 };


// System row positioning
constexpr Point<int> FirstSystemButton{ 89, 683 };
constexpr Point<int> SystemButtonDelta{ 36, 0 };
constexpr Point<int> WideSystemButtonDelta{ 54, 0 };

constexpr Point<int> SystemSmallButton{ 24, -25 };
constexpr Point<int> SystemMediumButton{ 24, -32 };
constexpr Point<int> SystemBigButton{ 24, -37 };
constexpr Point<int> SystemLowerButton{ 24, -24 };
constexpr Point<int> SystemUpperButton{ 24, -48 };

constexpr Point<int> WeaponSlotsPadding{ 24, 0 };
constexpr Point<int> WeaponSlotsFirst{ 68, -41 };
constexpr Point<int> WeaponAutoFire{ -9, -40 };
constexpr Point<int> DroneSlotsPadding{ 16, 0 };
constexpr Point<int> DroneSlotsFirst{ 64,-41 };
constexpr Point<int> SlotDelta{ 97, 0 };

constexpr Point<int> CloseAllDoorsButton{ 1141, 617 };
constexpr Point<int> OpenAllDoorsButton{ 1141, 640 };
constexpr Point<int> BatteryButton{ 1192, 628 };

}

Input::Input(const StateReader& reader)
	: reader(reader)
	, impl(new Impl)
{

}

Input::~Input()
{

}

Point<int> Input::systemPos(SystemType which)
{
	Point<int> pos = FirstSystemButton;
	auto& curr = this->reader.getState();
	auto& sys = curr.player.systems;

	for (auto& i : SystemOrder)
	{
		if (i == which)
			break;

		if (sys.get(i))
		{
			switch (i)
			{
			case SystemType::Weapons:
				pos += WeaponSlotsPadding * 2 + SlotDelta * sys.weapons.slots;
				break;
			case SystemType::Drones:
				pos += DroneSlotsPadding * 2 + SlotDelta * sys.drones.slots;
				break;
			case SystemType::Teleporter: [[fallthrough]];
			case SystemType::Cloaking: [[fallthrough]];
			case SystemType::MindControl: [[fallthrough]];
			case SystemType::Hacking:
				pos += WideSystemButtonDelta;
				break;
			default:
				pos += SystemButtonDelta;
			}
		}
	}

	return pos;
}

Point<int> Input::crewPos(int which)
{
	auto& curr = this->reader.getState();

	if (which >= curr.playerCrewCount)
		return {};

	return FirstCrewButton + CrewButtonDelta * which;
}

Point<int> Input::weaponSlotPos(int which)
{
	auto w = this->systemPos(SystemType::Weapons);
	return w + WeaponSlotsFirst + SlotDelta * which;
}

Point<int> Input::droneSlotPos(int which)
{
	return this->systemPos(SystemType::Drones) + DroneSlotsFirst + SlotDelta * which;
}

Point<int> Input::roomPos(int which, bool ownShip, const Point<float>& offset)
{
	auto& curr = this->reader.getState();

	Point<int> pos, size;
	Point<float> off = offset;

	if (ownShip)
	{
		auto& r = curr.player.rooms.at(which);
		pos = r.position + curr.shipPosition;
		size = r.size;
	}
	else
	{
		auto& r = curr.target.rooms.at(which);
		pos = r.position + curr.targetShipPosition;
		size = r.size;
	}

	if (off.x < 0.f || off.x > float(size.x))
		off.x = float(size.x) / 2.f;

	if (off.y < 0.f || off.y > float(size.y))
		off.y = float(size.y) / 2.f;

	pos.x += int(float(RoomTileLength) * off.x);
	pos.y += int(float(RoomTileLength) * off.y);

	return pos;
}

void Input::iterate()
{
	auto& curr = this->reader.getState();
	if (!curr.focused || curr.minimized)
		return;

	if(!this->impl->idle())
		this->impl->iterate();

	int cap = 1;

	while (!this->queue.empty() && cap > 0)
	{
		auto& cmd = this->queue.front();

		switch (cmd.type)
		{
		case FTLCommand::Nothing:
		{
			if (cmd.wait-- > 0)
				return;

			break;
		}
		case FTLCommand::OpenJumpMenu:
		{
			this->impl->clickAt(JumpButton);
			break;
		}
		case FTLCommand::OpenUpgradeMenu:
		{
			this->impl->clickAt(UpgradeButton);
			break;
		}
		case FTLCommand::OpenStore:
		{
			if (curr.store)
				this->impl->clickAt(StoreButton);

			break;
		}
		case FTLCommand::SelectCrew:
		{
			auto& [crew, shift] = cmd.crewSelection;

			Point<int> pos = this->crewPos(crew);

			if (pos.x > 0)
				this->impl->clickAt(this->crewPos(crew), { .shift = shift });

			break;
		}
		case FTLCommand::CrewToRoom:
		{
			auto& [room, ownShip] = cmd.roomSelection;

			if (ownShip)
			{
				auto& r = curr.player.rooms.at(room);
				this->impl->clickAt(curr.shipPosition + r.position + r.size / 2);
			}
			else
			{
				// To do: target ship position
				auto& r = curr.player.rooms.at(room);
				this->impl->clickAt(r.position + r.size / 2);
			}

			break;
		}
		case FTLCommand::SaveStations:
		{
			Point<int> pos = FirstCrewButton + CrewButtonDelta * curr.playerCrewCount + SaveStationsButton;
			this->impl->clickAt(pos);
			break;
		}
		case FTLCommand::ReturnToStations:
		{
			Point<int> pos = FirstCrewButton + CrewButtonDelta * curr.playerCrewCount + ReturnToStationsButton;
			this->impl->clickAt(pos);
			break;
		}
		case FTLCommand::CrystalAbility:
		{
			auto& crew = cmd.crewSelection.id;
			Point<int> pos = this->crewPos(crew);
			this->impl->mouseMove(pos);
			this->impl->clickAt(pos + CrystalAbilityDelta);
			break;
		}
		case FTLCommand::ChangePower:
		{
			auto& [which, amount, set] = cmd.powerChange;

			auto* sys = curr.player.systems.get(which);
			if (!sys)
				break;

			if (set)
				amount -= sys->power.total;

			if (amount > 0)
			{
				if (which == SystemType::Shields)
					amount = (amount + 1) / 2;

				auto& [used, max] = curr.player.reactor;
				auto& limit = curr.player.reactorLimit;
				amount = std::min(amount, std::min(max - used, limit));

				for (int i = 0; i < amount; ++i)
					this->impl->clickAt(this->systemPos(which));
			}
			else if (amount < 0)
			{
				if (which == SystemType::Shields)
					amount = (amount - 1) / 2;

				for (int i = 0; i > amount; --i)
					this->impl->clickAt(this->systemPos(which), { .right = true });
			}

			break;
		}
		case FTLCommand::ToggleWeapon:
		{
			auto& [which, on] = cmd.slotPowerChange;

			if (!curr.player.systems.weapons.present)
				break;

			auto& weapons = curr.player.systems.weapons.list;

			if (size_t(which) >= weapons.size() || weapons.at(which).powered == on)
				break;

			this->impl->clickAt(this->weaponSlotPos(which), { .right = !on });
			break;
		}
		case FTLCommand::ToggleDrone:
		{
			auto& [which, on] = cmd.slotPowerChange;

			if (!curr.player.systems.drones.present)
				break;

			auto& drones = curr.player.systems.drones.list;

			if (size_t(which) >= drones.size() || drones.at(which).powered == on)
				break;

			this->impl->clickAt(this->droneSlotPos(which), { .right = !on });
			break;
		}
		case FTLCommand::MoveWeapon:
		{
			auto& [from, to] = cmd.slotSwap;

			if (from == to)
				break;

			this->impl->mouseMove(this->weaponSlotPos(from));
			this->impl->mouseDown();
			this->impl->mouseMove(this->weaponSlotPos(to));
			this->impl->mouseUp();
			break;
		}
		case FTLCommand::MoveDrone:
		{
			auto& [from, to] = cmd.slotSwap;

			if (from == to)
				break;

			this->impl->mouseMove(this->droneSlotPos(from));
			this->impl->mouseDown();
			this->impl->mouseMove(this->droneSlotPos(to));
			this->impl->mouseUp();
			break;
		}
		case FTLCommand::FireAtRoom:
		{
			auto& [which, roomA, roomB, ownShip, autoFire, offsetA, offsetB] = cmd.weaponFire;


			this->impl->clickAt(this->weaponSlotPos(which), { .ctrl = autoFire });

			this->impl->clickAt(this->roomPos(roomA, ownShip, offsetA), { .ctrl = autoFire });

			if(roomB >= 0)
				this->impl->clickAt(this->roomPos(roomB, ownShip, offsetB), { .ctrl = autoFire });

			break;
		}
		case FTLCommand::ToggleDoor:
		{
			auto& [door, open] = cmd.doorChange;
			auto& d = curr.player.doors.at(door);

			if (open != d.open)
				this->impl->clickAt(d.position + curr.shipPosition);

			break;
		}
		case FTLCommand::OpenAllDoors:
		{
			auto& [airlocks] = cmd.allDoorChange;

			for (int i = 0; i < (airlocks ? 2 : 1); ++i)
				this->impl->clickAt(OpenAllDoorsButton);
			break;
		}
		case FTLCommand::CloseAllDoors:
		{
			this->impl->clickAt(CloseAllDoorsButton);
			break;
		}
		case FTLCommand::Teleport:
		{
			if (!curr.targetPresent)
				break;

			auto& [room, retrieve] = cmd.teleportation;

			Point<int> pos = this->systemPos(SystemType::Teleporter);
			pos += retrieve ? SystemLowerButton : SystemUpperButton;
			this->impl->clickAt(pos);
			this->impl->clickAt(this->roomPos(room, false));
			break;
		}
		case FTLCommand::Cloak:
		{
			this->impl->clickAt(this->systemPos(SystemType::Cloaking) + SystemSmallButton);
			break;
		}
		case FTLCommand::Battery:
		{
			this->impl->clickAt(BatteryButton);
			break;
		}
		case FTLCommand::Mind:
		{
			if (!curr.targetPresent)
				break;

			auto& [room, ownShip] = cmd.roomSelection;

			this->impl->clickAt(this->systemPos(SystemType::MindControl) + SystemMediumButton);
			this->impl->clickAt(this->roomPos(room, ownShip));
			break;
		}
		case FTLCommand::Hack:
		{
			this->impl->clickAt(this->systemPos(SystemType::Hacking) + SystemSmallButton);
			break;
		}
		case FTLCommand::DeployHackingDrone:
		{
			if (!curr.targetPresent)
				break;

			auto& [system, artilleryIndex] = cmd.hackingDroneDeployment;

			auto* sys = curr.target.systems.get(system, artilleryIndex);
			if (!sys)
				break;

			this->impl->clickAt(this->systemPos(SystemType::Hacking) + SystemSmallButton);
			this->impl->clickAt(this->roomPos(sys->room, false));
			break;
		}
		}

		this->queue.pop();
		--cap;
	}
}

void Input::openJumpMenu()
{
	this->queue.push({ .type = FTLCommand::OpenJumpMenu });
}

void Input::openUpgradeMenu()
{
	this->queue.push({ .type = FTLCommand::OpenJumpMenu });
}

void Input::openStore()
{
	this->queue.push({ .type = FTLCommand::OpenStore });
}

void Input::selectCrew(int crew, bool shift)
{
	this->queue.push({
		.type = FTLCommand::SelectCrew,
		.crewSelection = {.id = crew, .shift = shift}
	});
}

void Input::crewToRoom(int room, bool ownShip)
{
	this->queue.push({
		.type = FTLCommand::SelectCrew,
		.roomSelection = {.id = room, .ownShip = ownShip}
	});
}

void Input::saveStations()
{
	this->queue.push({ .type = FTLCommand::SaveStations });
}

void Input::returnToStations()
{
	this->queue.push({ .type = FTLCommand::ReturnToStations });
}

void Input::crystalAbility(int crew)
{
	this->queue.push({
		.type = FTLCommand::CrystalAbility,
		.crewSelection = {
			.id = crew
		}
	});
}

void Input::changePower(SystemType which, int amount, bool set)
{
	this->queue.push({
		.type = FTLCommand::ChangePower,
		.powerChange = {
			.system = which,
			.amount = amount,
			.set = set
		}
	});
}

void Input::toggleWeapon(int which, bool on)
{
	this->queue.push({
		.type = FTLCommand::ToggleWeapon,
		.slotPowerChange = {
			.slot = which,
			.on = on
		}
	});
}

void Input::toggleDrone(int which, bool on)
{
	this->queue.push({
		.type = FTLCommand::ToggleDrone,
		.slotPowerChange = {
			.slot = which,
			.on = on
		}
	});
}

void Input::moveWeapon(int from, int to)
{
	this->queue.push({
		.type = FTLCommand::MoveWeapon,
		.slotSwap = {
			.from = from,
			.to = to
		}
	});
}

void Input::moveDrone(int from, int to)
{
	this->queue.push({
		.type = FTLCommand::MoveDrone,
		.slotSwap = {
			.from = from,
			.to = to
		}
	});
}

void Input::fireAtRoom(const WeaponFireOptions& options)
{
	this->queue.push({
		.type = FTLCommand::FireAtRoom,
		.weaponFire = options
	});
}

void Input::toggleDoor(int door, bool open)
{
	this->queue.push({
		.type = FTLCommand::ToggleDoor,
		.doorChange = {
			.id = door,
			.open = open
		}
	});
}

void Input::openAllDoors(bool airlocks)
{
	this->queue.push({
		.type = FTLCommand::OpenAllDoors,
		.allDoorChange = {
			.airlocks = airlocks
		}
	});
}

void Input::closeAllDoors()
{
	this->queue.push({
		.type = FTLCommand::CloseAllDoors,
		.allDoorChange = {}
	});
}

void Input::teleport(int room, bool retrieve)
{
	this->queue.push({
		.type = FTLCommand::Teleport,
		.teleportation = {
			.room = room,
			.retrieve = retrieve
		}
	});
}

void Input::cloak()
{
	this->queue.push({ .type = FTLCommand::Cloak });
}

void Input::battery()
{
	this->queue.push({ .type = FTLCommand::Battery });
}

void Input::mind(int room, bool ownShip)
{
	this->queue.push({
		.type = FTLCommand::Mind,
		.roomSelection = {
			.id = room,
			.ownShip = ownShip
		}
	});
}

void Input::deployHackingDrone(SystemType system, int artilleryIndex)
{
	this->queue.push({
		.type = FTLCommand::DeployHackingDrone,
		.hackingDroneDeployment = {
			.system = system,
			.artilleryIndex = artilleryIndex
		}
	});
}

void Input::hack()
{
	this->queue.push({.type = FTLCommand::Hack});
}

void Input::wait(int iterations)
{
	this->queue.push({
		.type = FTLCommand::Nothing,
		.wait = iterations
	});
}

bool Input::idle() const
{
	return this->queue.empty() && this->impl->idle();
}