#include "Input.hpp"
#include "../GameState/Reader.hpp"
#include "../Utility/ProcessReader.hpp"
#include "../Utility/Geometry.hpp"

#include <cmath>

#include <winuser.h>
#include <dwmapi.h>

class Input::Impl
{
public:
	explicit Impl(const ProcessReader& processReader)
		: window(processReader.getWindow())
	{
	}

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
				Pointi p = this->gameToScreen(cmd.position.x, cmd.position.y);
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

	void mouseMove(const Pointi& pos)
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

	void clickAt(const Pointi& pos, const MouseOptions& options = {})
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
	HWND window = nullptr;
	std::queue<InputCommand> queue;

	void mouseInput(const InputCommand::MouseClick& cmd)
	{
		auto& [down, up, options] = cmd;
		auto& [right, shift, ctrl] = options;

		if (!down && !up)
			return;

		INPUT inputs[8];
		ZeroMemory(inputs, sizeof(inputs));
		size_t i = 0;

		if (shift)
		{
			inputs[i].type = INPUT_KEYBOARD;
			inputs[i].ki.wVk = VK_SHIFT;
			++i;
		}

		if (ctrl)
		{
			inputs[i].type = INPUT_KEYBOARD;
			inputs[i].ki.wVk = VK_CONTROL;
			++i;
		}

		if (down)
		{
			inputs[i].type = INPUT_MOUSE;
			inputs[i].mi.dwFlags = right ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_LEFTDOWN;
			++i;
		}

		if (up)
		{
			inputs[i].type = INPUT_MOUSE;
			inputs[i].mi.dwFlags = right ? MOUSEEVENTF_RIGHTUP : MOUSEEVENTF_LEFTUP;
			++i;
		}

		if (ctrl)
		{
			inputs[i].type = INPUT_KEYBOARD;
			inputs[i].ki.wVk = VK_CONTROL;
			inputs[i].ki.dwFlags = KEYEVENTF_KEYUP;
			++i;
		}

		if (shift)
		{
			inputs[i].type = INPUT_KEYBOARD;
			inputs[i].ki.wVk = VK_SHIFT;
			inputs[i].ki.dwFlags = KEYEVENTF_KEYUP;
			++i;
		}

		this->sendInput(inputs, i);
	}

	Pointi gameToScreen(int x, int y)
	{
		RECT rect;

		if (!GetClientRect(this->window, &rect))
			throw std::runtime_error("GetClientRect error. Code: " + std::to_string(GetLastError()));

		Pointf scale{
			float(rect.right)/float(Input::GameWidth),
			float(rect.bottom)/float(Input::GameHeight)
		};

		scale.x *= float(x);
		scale.y *= float(y);

		Pointi result = { int(rect.left) + int(scale.x), int(rect.top) + int(scale.y) };

		if (result.x < rect.left) result.x = rect.left;
		if (result.y < rect.top) result.y = rect.top;
		if (result.x >= rect.right) result.x = rect.right - 1;
		if (result.y >= rect.bottom) result.y = rect.bottom - 1;

		POINT pos{ result.x, result.y };
		if (!ClientToScreen(this->window, &pos))
			throw std::runtime_error("ClientToScreen error. Code: " + std::to_string(GetLastError()));

		return { int(pos.x), int(pos.y) };
	}

	void setCursorPos(int x, int y)
	{
		if(!SetCursorPos(x, y))
			throw std::runtime_error("SetCursorPos error. Code: " + std::to_string(GetLastError()));
	}

	void sendInput(INPUT* inputs, size_t amount)
	{
		UINT sent = SendInput(amount, inputs, sizeof(INPUT));

		if (sent != amount)
			throw std::runtime_error("SendInput error. Code: " + std::to_string(GetLastError()));
	}
};

// Thanks windows
#undef min

namespace
{

// Misc buttons
constexpr Pointi JumpButton{ 572, 49 };
constexpr Pointi UpgradeButton{ 661, 49 };
constexpr Pointi StoreButton{ 753, 49 };


// Crew stuff
constexpr Pointi FirstCrewButton{ 53, 168 };
constexpr Pointi CrewButtonDelta{ 0, 30 };
constexpr Pointi CrystalAbilityDelta{ 86, 0 };
constexpr Pointi SaveStationsButton{ -19, 33 };
constexpr Pointi ReturnToStationsButton{ 21, 33 };


// System row positioning
constexpr Pointi FirstSystemButton{ 89, 683 };
constexpr Pointi SystemButtonDelta{ 36, 0 };
constexpr Pointi WideSystemButtonDelta{ 54, 0 };

constexpr Pointi SystemSmallButton{ 24, -25 };
constexpr Pointi SystemMediumButton{ 24, -32 };
constexpr Pointi SystemBigButton{ 24, -37 };
constexpr Pointi SystemLowerButton{ 24, -24 };
constexpr Pointi SystemUpperButton{ 24, -48 };

constexpr Pointi WeaponSlotsPadding{ 24, 0 };
constexpr Pointi WeaponSlotsFirst{ 68, -41 };
constexpr Pointi WeaponAutoFire{ -9, -40 };
constexpr Pointi DroneSlotsPadding{ 16, 0 };
constexpr Pointi DroneSlotsFirst{ 64,-41 };
constexpr Pointi SlotDelta{ 97, 0 };

constexpr Pointi CloseAllDoorsButton{ 1141, 617 };
constexpr Pointi OpenAllDoorsButton{ 1141, 640 };
constexpr Pointi BatteryButton{ 1192, 628 };

}

Input::Input(const StateReader& reader)
	: reader(reader)
	, impl(new Impl{ reader.getProcessReader() })
{

}

Input::~Input()
{

}

Pointi Input::systemPos(SystemID which)
{
	Pointi pos = FirstSystemButton;
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
			case SystemID::Weapons:
				pos += WeaponSlotsPadding * 2 + SlotDelta * sys.weapons.slots;
				break;
			case SystemID::Drones:
				pos += DroneSlotsPadding * 2 + SlotDelta * sys.drones.slots;
				break;
			case SystemID::Teleporter: [[fallthrough]];
			case SystemID::Cloaking: [[fallthrough]];
			case SystemID::Mind: [[fallthrough]];
			case SystemID::Hacking:
				pos += WideSystemButtonDelta;
				break;
			default:
				pos += SystemButtonDelta;
			}
		}
	}

	return pos;
}

Pointi Input::crewPos(int which)
{
	auto& curr = this->reader.getState();

	if (which >= curr.playerCrewCount)
		return {};

	return FirstCrewButton + CrewButtonDelta * which;
}

Pointi Input::weaponSlotPos(int which)
{
	auto w = this->systemPos(SystemID::Weapons);
	return w + WeaponSlotsFirst + SlotDelta * which;
}

Pointi Input::droneSlotPos(int which)
{
	return this->systemPos(SystemID::Drones) + DroneSlotsFirst + SlotDelta * which;
}

Pointi Input::roomPos(int which, bool ownShip, const Pointf& offset)
{
	auto& curr = this->reader.getState();

	Pointi pos, size;
	Pointf off = offset;

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

			Pointi pos = this->crewPos(crew);

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
			Pointi pos = FirstCrewButton + CrewButtonDelta * curr.playerCrewCount + SaveStationsButton;
			this->impl->clickAt(pos);
			break;
		}
		case FTLCommand::ReturnToStations:
		{
			Pointi pos = FirstCrewButton + CrewButtonDelta * curr.playerCrewCount + ReturnToStationsButton;
			this->impl->clickAt(pos);
			break;
		}
		case FTLCommand::CrystalAbility:
		{
			auto& crew = cmd.crewSelection.id;
			Pointi pos = this->crewPos(crew);
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
				if (which == SystemID::Shields)
					amount = (amount + 1) / 2;

				auto& [used, max] = curr.player.reactor;
				auto& limit = curr.player.reactorLimit;
				amount = std::min(amount, std::min(max - used, limit));

				for (int i = 0; i < amount; ++i)
					this->impl->clickAt(this->systemPos(which));
			}
			else if (amount < 0)
			{
				if (which == SystemID::Shields)
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

			Pointi pos = this->systemPos(SystemID::Teleporter);
			pos += retrieve ? SystemLowerButton : SystemUpperButton;
			this->impl->clickAt(pos);
			this->impl->clickAt(this->roomPos(room, false));
			break;
		}
		case FTLCommand::Cloak:
		{
			this->impl->clickAt(this->systemPos(SystemID::Cloaking) + SystemSmallButton);
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

			this->impl->clickAt(this->systemPos(SystemID::Mind) + SystemMediumButton);
			this->impl->clickAt(this->roomPos(room, ownShip));
			break;
		}
		case FTLCommand::Hack:
		{
			this->impl->clickAt(this->systemPos(SystemID::Hacking) + SystemSmallButton);
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

			this->impl->clickAt(this->systemPos(SystemID::Hacking) + SystemSmallButton);
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

void Input::changePower(SystemID which, int amount, bool set)
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

void Input::deployHackingDrone(SystemID system, int artilleryIndex)
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