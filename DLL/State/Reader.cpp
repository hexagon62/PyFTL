#include "Reader.hpp"
#include "../Player/Input.hpp"
#include "../Utility/Memory.hpp"

#include <algorithm>
#include <unordered_map>

Duration Reader::delay{ std::chrono::microseconds(16666) };
TimePoint Reader::nextPoll{ Clock::now() };
raw::State Reader::rs;
State Reader::state;
uintptr_t Reader::base = 0;
std::jthread Reader::thread;
bool Reader::reloading;
extern bool g_quit;

namespace
{

int calculatePowerCap(int max, int cap, int loss, int divide)
{
	int capQuotient = max / std::clamp(divide, 1, 2);
	int capDiff = std::max(0, max - loss);
	return std::min(std::min(capQuotient, capDiff), cap);
}

void readPower(Power& power, const raw::ShipSystem& raw)
{
	power.normal = raw.powerState.first;
	power.zoltan = raw.iBonusPower;
	power.battery = raw.iBatteryPower;

	power.cap = calculatePowerCap(
		raw.powerState.second,
		raw.iTempPowerCap,
		raw.iTempPowerLoss,
		raw.iTempDividePower);

	power.ionLevel = raw.iLockCount;
	power.ionTimer.first = raw.lockTimer.currTime;
	power.ionTimer.second = raw.lockTimer.currGoal;

	power.restoreTo = raw.lastUserPower;

	power.total.first = power.normal + power.zoltan + power.battery;
	power.total.second = std::min(raw.healthState.first, power.cap);
}

void readSystem(System& system, const raw::ShipSystem& raw)
{
	readPower(system.power, raw);

	system.type = SystemType(raw.iSystemType);
	system.room = raw.roomId;

	system.health = raw.healthState;

	system.level.first = raw.powerState.second;
	system.level.second = raw.maxLevel;

	system.manningLevel = raw.iActiveManned;
	system.needsManning = raw.bNeedsManned;
	system.occupied = raw.bFriendlies;

	system.onFire = raw.bOnFire;
	system.breached = raw.bBreached;
	system.boardersAttacking = raw.bUnderAttack;

	system.damageProgress = raw.fDamageOverTime / 100.f;
	system.repairProgress = raw.fRepairOverTime / 100.f;

	// figuring out if we're actually manned is more complicated...
	//bool limited = system.power.cap < system.level.current;
	//bool manningBlocked = raw.iHackEffect != 0 || system.boardersAttacking || limited;
	//bool needsRepairing = system.onFire || system.breached || !system.health.capped();
	//bool manned = raw.bManned && raw.bBoostable && !manningBlocked && !needsRepairing;
	//
	//system.manningLevel = manned ? raw.iActiveManned : 0;
}

void readShieldSystem(ShieldSystem& shields, const raw::Shields& raw)
{
	readSystem(shields, raw);
	shields.blueprint = Reader::getState().blueprints.systemBlueprints.at("shields");

	shields.boundary.center.x = raw.baseShield.center.x;
	shields.boundary.center.y = raw.baseShield.center.y;
	shields.boundary.a = int(raw.baseShield.a);
	shields.boundary.b = int(raw.baseShield.b);

	shields.bubbles.first = raw.shields.power.first;
	shields.bubbles.second = raw.shields.power.second;

	shields.charge.first = raw.shields.charger;
	shields.charge.second = raw.chargeTime;
}

void readEngineSystem(EngineSystem& engines, const raw::EngineSystem& raw)
{
	readSystem(engines, raw);
	engines.blueprint = Reader::getState().blueprints.systemBlueprints.at("engines");

	engines.boostFTL = raw.bBoostFTL;
}

// Needs list of rooms to find blocked slot
void readMedbaySystem(
	MedbaySystem& medbay,
	const raw::MedbaySystem& raw,
	const raw::gcc::vector<raw::Room*>& rooms)
{
	readSystem(medbay, raw);

	// Assuming room id matches index, which *seems* to be true...
	medbay.slot = rooms[medbay.room]->primarySlot;
}

// Needs list of all crew
void readClonebaySystem(
	ClonebaySystem& clonebay,
	const raw::CloneSystem& raw,
	std::vector<Crew>& crew)
{
	readSystem(clonebay, raw);
	clonebay.blueprint = Reader::getState().blueprints.systemBlueprints.at("clonebay");

	clonebay.cloneTimer.first = raw.fTimeToClone;
	clonebay.cloneTimer.second = raw.fTimeGoal;

	clonebay.deathTimer.first = raw.fDeathTime;
	clonebay.deathTimer.second = ClonebaySystem::HARDCODED_DEATH_TIME;

	clonebay.slot = raw.slot;

	clonebay.queue.clear();

	// Insert clones into queue, sorted by death id
	for (size_t i = 0; i < crew.size(); i++)
	{
		if (crew[i].readyToClone)
		{
			for (size_t j = 0; j < clonebay.queue.size(); j++)
			{
				if (clonebay.queue[j]->deathId > crew[i].deathId)
				{
					auto it = clonebay.queue.cbegin();
					std::advance(it, j);
					clonebay.queue.insert(it, &crew[i]);
				}
			}
		}
	}

	// Set queue positions
	for (size_t i = 0; i < clonebay.queue.size(); i++)
	{
		clonebay.queue[i]->cloneQueuePosition = i;
	}
}

void readOxygenSystem(OxygenSystem& oxygen, const raw::OxygenSystem& raw)
{
	readSystem(oxygen, raw);
	oxygen.blueprint = Reader::getState().blueprints.systemBlueprints.at("oxygen");

	// nothing else interesting?
}

// Read teleporter info that is shared on both player and enemy ships
void readTeleporterSystemGeneric(
	TeleporterSystem& teleporter,
	const raw::TeleportSystem& raw,
	std::vector<Crew>& crew)
{
	readSystem(teleporter, raw);
	teleporter.blueprint = Reader::getState().blueprints.systemBlueprints.at("teleporter");

	teleporter.slots = raw.iNumSlots;
	teleporter.crewPresent.clear();

	for (size_t i = 0; i < crew.size(); i++)
	{
		if (teleporter.room == crew[i].room && !crew[i].moving)
			teleporter.crewPresent.push_back(&crew[i]);
	}
}

void readBlueprint(Blueprint& blueprint, const raw::Blueprint& raw)
{
	blueprint.name = raw.name.str;
	blueprint.cost = raw.desc.cost;
	blueprint.rarity = raw.desc.rarity;
	blueprint.baseRarity = raw.desc.baseRarity;
}

void readAugment(Augment& augment, const raw::AugmentBlueprint& raw)
{
	readBlueprint(augment, raw);

	augment.value = raw.value;
	augment.stacking = raw.stacking;
}

void readDamage(Damage& damage, const raw::Damage& raw)
{
	damage.normal = raw.iDamage;
	damage.ion = raw.iIonDamage;
	damage.system = raw.iSystemDamage;
	damage.crew = raw.iPersDamage*Damage::HARDCODED_CREW_DAMAGE_FACTOR;
	damage.fireChance = raw.fireChance;
	damage.breachChance = raw.breachChance;
	damage.stunChance = raw.stunChance;
	damage.pierce = raw.iShieldPiercing;
	damage.stunTime = raw.iStun;
	damage.hullBonus = raw.bHullBuster;
	damage.friendlyFire = raw.bFriendlyFire;
}

void readWeaponBlueprint(WeaponBlueprint& blueprint, const raw::WeaponBlueprint& raw)
{
	readBlueprint(blueprint, raw);
	readDamage(blueprint.damage, raw.damage);

	blueprint.shots = raw.shots;
	blueprint.missiles = raw.missiles;
	blueprint.cooldown = raw.cooldown;
	blueprint.power = raw.power;
	blueprint.beamLength = raw.length;
	blueprint.burstRadius = raw.radius;
	blueprint.chargeLevels = raw.chargeLevels;
	blueprint.boost.type = raw.boostPower.type;
	blueprint.boost.amount = raw.boostPower.amount;
	blueprint.boost.count = raw.boostPower.count;
	
	std::string type = raw.typeName.str;

	blueprint.projectilesFake = 0;
	blueprint.projectiles = 1;

	if (type == "LASER") blueprint.type = WeaponType::Laser;
	else if (type == "BEAM")
	{
		blueprint.type = WeaponType::Beam;
		blueprint.projectiles = 0;
	}
	else if (type == "BURST")
	{
		blueprint.type = WeaponType::Burst;

		// mini projectile stuffs
		blueprint.projectilesFake = 0;
		blueprint.projectiles = 0;

		for (size_t i = 0; i < raw.miniProjectiles.size(); i++)
		{
			auto&& mini = raw.miniProjectiles[i];
			if (mini.fake) blueprint.projectilesFake++;
			else blueprint.projectiles++;
		}
	}
	else if (type == "MISSILES") blueprint.type = WeaponType::Missiles;
	else if (type == "BOMB") blueprint.type = WeaponType::Bomb;
	else blueprint.type = WeaponType::Invalid;

	blueprint.projectilesTotal = blueprint.projectiles * blueprint.shots;
}

void readWeapon(Weapon& weapon, const raw::ProjectileFactory& raw)
{
	readWeaponBlueprint(weapon.blueprint, *raw.blueprint);

	weapon.cooldown = raw.cooldown;

	weapon.autoFire = raw.autoFiring;
	weapon.fireWhenReady = raw.fireWhenReady;
	weapon.powered = raw.powered;
	weapon.artillery = raw.isArtillery;
	weapon.targetingPlayer = raw.currentShipTarget == 0;

	weapon.firingAngle = raw.currentFiringAngle;
	weapon.entryAngle = raw.currentEntryAngle;

	weapon.mount.x = raw.mount.position.x+raw.localPosition.x;
	weapon.mount.y = raw.mount.position.y+raw.localPosition.y;

	weapon.zoltanPower = raw.iBonusPower;

	weapon.hackLevel = HackLevel(raw.iHackLevel);

	weapon.boost.first = raw.boostLevel;
	weapon.boost.second = weapon.blueprint.boost.count;
	weapon.charge.first = raw.chargeLevel;
	weapon.charge.second = raw.goalChargeLevel;

	weapon.shotTimer.first = raw.weaponVisual.anim.tracker.current_time;
	weapon.shotTimer.second = raw.weaponVisual.anim.tracker.time;

	weapon.targetPoints.clear();

	// Target points
	for (size_t i = 0; i < raw.targets.size(); i++)
	{
		auto&& pos = raw.targets[i];
		weapon.targetPoints.emplace_back(pos.x, pos.y);
	}
}

void readDroneBlueprint(DroneBlueprint& blueprint, const raw::DroneBlueprint& raw)
{
	readBlueprint(blueprint, raw);

	std::string type = raw.typeName.str;

	if (type == "COMBAT") blueprint.type = DroneType::Combat;
	else if (type == "DEFENSE") blueprint.type = DroneType::Defense;
	else if (type == "BATTLE") blueprint.type = DroneType::AntiBoarder;
	else if (type == "BOARDER") blueprint.type = DroneType::Boarder;
	else if (type == "SHIP_REPAIR") blueprint.type = DroneType::HullRepair;
	else if (type == "REPAIR") blueprint.type = DroneType::SystemRepair;
	else if (type == "HACKING") blueprint.type = DroneType::Hacking;
	else if (type == "SHIELD") blueprint.type = DroneType::Shield;

	blueprint.power = raw.power;
	blueprint.cooldown = raw.cooldown;
	blueprint.speed = raw.speed;
}

void readDrone(Drone& drone, const raw::Drone& raw, int superShieldBubbles = 0)
{
	readDroneBlueprint(drone.blueprint, *raw.blueprint);

	drone.player = raw.iShipId == 0;
	drone.powered = raw.powered;
	drone.dead = raw.bDead;
	drone.zoltanPower = raw.iBonusPower;
	drone.hackLevel = HackLevel(raw.iHackLevel);
	drone.hackTime = raw.hackTime;
	drone.destroyTimer.first = raw.destroyedTimer;
	drone.destroyTimer.second = drone.HARDCODED_REBUILD_TIME;

	auto&& type = drone.blueprint.type;

	bool spaceDrone =
		type == DroneType::Combat ||
		type == DroneType::Defense ||
		type == DroneType::HullRepair ||
		type == DroneType::Hacking ||
		type == DroneType::Shield;

	if (spaceDrone)
	{
		auto&& casted = static_cast<const raw::SpaceDrone&>(raw);
		drone.space = SpaceDroneInfo{};
		auto&& info = *drone.space;

		info.playerSpace = casted.currentSpace == 0;
		info.playerSpaceIsDestination = casted.destinationSpace == 0;
		info.moving = casted.pause > 0.f;
		
		info.position.x = casted.currentLocation.x;
		info.position.y = casted.currentLocation.y;
		info.positionLast.x = casted.lastLocation.x;
		info.positionLast.y = casted.lastLocation.y;
		info.destination.x = casted.destinationLocation.x;
		info.destination.y = casted.destinationLocation.y;
		info.speed.x = casted.speedVector.x;
		info.speed.y = casted.speedVector.y;

		info.pause = casted.pause;

		if (type == DroneType::Shield)
		{
			constexpr int SHIELD_COOLDOWN_COUNT = DroneBlueprint::HARDCODED_SHIELD_COOLDOWNS.size();

			int idx = std::clamp(superShieldBubbles, 0, SHIELD_COOLDOWN_COUNT-1);
			info.cooldown.second = DroneBlueprint::HARDCODED_SHIELD_COOLDOWNS[idx];
		}
		else
		{
			info.cooldown.second = drone.blueprint.cooldown;
		}

		info.angle = casted.aimingAngle;
		info.angleDesired = casted.desiredAimingAngle;
		info.angleMalfunction = casted.hackAngle;
		info.ionTime = casted.ionStun;

		if (casted.weaponBlueprint)
		{
			info.weapon = WeaponBlueprint{};
			readWeaponBlueprint(*info.weapon, *casted.weaponBlueprint);
		}
		else
		{
			info.weapon = std::nullopt;
		}

		// This is weird in that it's -1 if the weapon's not on cooldown
		// Then it decreases to 0
		// Normal weapons count *up* towards the cooldown number
		if (casted.weaponCooldown < 0.f)
		{
			info.cooldown.first = info.cooldown.second;
		}
		else
		{
			info.cooldown.first = info.cooldown.second - casted.weaponCooldown;
		}

		bool extraMovementInfo =
			type == DroneType::Combat ||
			type == DroneType::HullRepair;

		if (extraMovementInfo)
		{
			auto&& casted2 = static_cast<const raw::CombatDrone&>(casted);
			info.extraMovement = SpaceDroneMovementExtra{};
			auto&& movement = *info.extraMovement;
			
			movement.destinationLast.x = casted2.lastDestination.x;
			movement.destinationLast.y = casted2.lastDestination.y;
			movement.progress = casted2.progressToDestination;
			movement.heading = casted2.heading;
			movement.headingLast = casted2.oldHeading;
		}
		else
		{
			info.extraMovement = std::nullopt;
		}
	}
	else
	{
		drone.space = std::nullopt;
	}
}

void readSystemBlueprint(SystemBlueprint& blueprint, const raw::SystemBlueprint& raw)
{
	readBlueprint(blueprint, raw);
	blueprint.powerStart = raw.startPower;
	blueprint.powerMax = raw.maxPower;
	blueprint.upgradeCosts.clear();

	for (size_t i = 0; i < raw.upgradeCosts.size(); i++)
	{
		blueprint.upgradeCosts.emplace_back(raw.upgradeCosts[i]);
	}
}

void readWeaponSystem(WeaponSystem& weapons, const raw::WeaponSystem& raw)
{
	readSystem(weapons, raw);
	weapons.blueprint = Reader::getState().blueprints.systemBlueprints.at("weapons");

	weapons.slotCount = raw.slot_count;
	weapons.weapons.clear();
	weapons.userPowered.clear();
	weapons.repower.clear();

	for (size_t i = 0; i < raw.weapons.size(); i++)
	{
		auto& weapon = weapons.weapons.emplace_back();
		readWeapon(weapon, *raw.weapons[i]);
	}

	for (size_t i = 0; i < raw.userPowered.size(); i++)
	{
		weapons.userPowered.emplace_back(raw.userPowered[i]);
	}

	for (size_t i = 0; i < raw.repowerList.size(); i++)
	{
		weapons.repower.emplace_back(raw.repowerList[i]);
	}
}

void readDroneSystem(DroneSystem& drones, const raw::DroneSystem& raw, int superShieldBubbles = 0)
{
	readSystem(drones, raw);
	drones.blueprint = Reader::getState().blueprints.systemBlueprints.at("drones");

	drones.slotCount = raw.slot_count;
	drones.drones.clear();
	drones.userPowered.clear();
	drones.repower.clear();

	for (size_t i = 0; i < raw.drones.size(); i++)
	{
		auto& drone = drones.drones.emplace_back();
		readDrone(drone, *raw.drones[i], superShieldBubbles);
	}

	for (size_t i = 0; i < raw.userPowered.size(); i++)
	{
		drones.userPowered.emplace_back(raw.userPowered[i]);
	}

	for (size_t i = 0; i < raw.repowerList.size(); i++)
	{
		drones.repower.emplace_back(raw.repowerList[i]);
	}
}

void readCloakingSystem(CloakingSystem& cloaking, const raw::CloakingSystem& raw)
{
	readSystem(cloaking, raw);
	cloaking.blueprint = Reader::getState().blueprints.systemBlueprints.at("cloaking");

	cloaking.on = raw.bTurnedOn;
	cloaking.timer.first = raw.timer.currTime;
	cloaking.timer.second = raw.timer.currGoal;
}

void readPilotingSystem(PilotingSystem& piloting, const raw::ShipSystem& raw)
{
	readSystem(piloting, raw);
	piloting.blueprint = Reader::getState().blueprints.systemBlueprints.at("pilot");

	// Probably no extra fields?
}

void readSensorsSystem(SensorSystem& sensors, const raw::ShipSystem& raw)
{
	readSystem(sensors, raw);
	sensors.blueprint = Reader::getState().blueprints.systemBlueprints.at("sensors");

	// Probably no extra fields?
}

void readDoorSystem(DoorSystem& doors, const raw::ShipSystem& raw)
{
	readSystem(doors, raw);
	doors.blueprint = Reader::getState().blueprints.systemBlueprints.at("doors");

	// Probably no extra fields?
}

void readArtillerySystem(ArtillerySystem& artillery, const raw::ArtillerySystem& raw)
{
	readSystem(artillery, raw);
	artillery.blueprint = Reader::getState().blueprints.systemBlueprints.at("artillery");

	if (raw.projectileFactory)
	{
		readWeapon(artillery.weapon, *raw.projectileFactory);
	}
}

void readBatterySystem(BatterySystem& battery, const raw::BatterySystem& raw)
{
	readSystem(battery, raw);
	battery.blueprint = Reader::getState().blueprints.systemBlueprints.at("battery");

	battery.on = raw.bTurnedOn;
	battery.timer.first = raw.timer.currTime;
	battery.timer.second = raw.timer.currGoal;
	battery.provides = raw.powerState.first * 2;
	battery.providing = battery.on ? battery.provides : 0;
}

void readMindControlSystem(MindControlSystem& mindControl, const raw::MindSystem& raw)
{
	readSystem(mindControl, raw);
	mindControl.blueprint = Reader::getState().blueprints.systemBlueprints.at("mind");

	mindControl.on = raw.controlTimer.first < raw.controlTimer.second;
	mindControl.timer = raw.controlTimer;
	mindControl.targetRoom = raw.iQueuedTarget;
	mindControl.targetingPlayerShip = raw.iQueuedShip == 0;
}

void readHackingSystem(HackingSystem& hacking, const raw::HackingSystem& raw)
{
	readSystem(hacking, raw);
	hacking.blueprint = Reader::getState().blueprints.systemBlueprints.at("hacking");

	hacking.on = raw.bHacking;
	hacking.timer = raw.effectTimer;
	hacking.target = SystemType(raw.currentSystem ? raw.currentSystem->iSystemType : -1);
	hacking.queued = SystemType(raw.queuedSystem ? raw.queuedSystem->iSystemType : -1);
	hacking.drone.start = { raw.drone.startingPosition.x, raw.drone.startingPosition.y };
	hacking.drone.goal = { raw.drone.finalDestination.x, raw.drone.finalDestination.y };
	hacking.drone.arrived = raw.drone.arrive;
	hacking.drone.setUp = raw.drone.finishedSetup;
	hacking.drone.room = raw.drone.prefRoom;

	readDrone(hacking.drone, raw.drone);
}

// Calculate evasion level (not conveniently stored anywhere by the game it seems)
int calculateEvasion(
	std::optional<EngineSystem>& engines,
	std::optional<PilotingSystem>& piloting,
	std::optional<CloakingSystem>& cloaking)
{
	int result = 0;

	// Both systems must be present
	if (engines && piloting)
	{
		bool enginesUnpowered = engines->power.total.first == 0;
		bool pilotingUnpowered = piloting->power.total.first == 0;
		bool noPilot = piloting->power.total.first <= 1 && !piloting->occupied;
		bool trivial0 = enginesUnpowered || pilotingUnpowered || noPilot;

		if (!trivial0)
		{
			int base = EngineSystem::HARDCODED_EVASION_VALUES[engines->power.total.first];
			int pilotBonus = EngineSystem::HARDCODED_EVASION_SKILL_BOOST[piloting->manningLevel];
			int engineBonus = PilotingSystem::HARDCODED_EVASION_SKILL_BOOST[engines->manningLevel];
			result += base + pilotBonus + engineBonus;

			if (noPilot)
			{
				switch (piloting->power.total.first)
				{
				case 2: result /= 2; break; // auto 50% evasion
				case 3: result *= 4; result /= 5; break; // auto 80% evasion
				default: result *= 0; break; // should not reach this case
				}
			}
		}
	}

	if (cloaking && cloaking->on) result += CloakingSystem::HARDCODED_EVASION_BONUS;

	return result;
}

void readRoom(Room& room, std::vector<Crew>& crew, std::vector<Crew>& enemyCrew, const raw::Room& raw)
{
	room.player = raw.iShipId == 0;
	room.id = raw.iRoomId;
	room.primarySlot = raw.primarySlot;
	room.primaryDirection = raw.primaryDirection;
	
	room.rect.x = raw.rect.x;
	room.rect.y = raw.rect.y;
	room.rect.w = raw.rect.w;
	room.rect.h = raw.rect.h;

	room.tiles.x = raw.rect.w / Room::HARDCODED_TILE_SIZE;
	room.tiles.y = raw.rect.h / Room::HARDCODED_TILE_SIZE;

	room.stunning = raw.bStunning;
	room.oxygen = raw.lastO2/100.f;
	room.hackLevel = HackLevel(raw.iHackLevel);

	room.fireRepair = 0.f;
	room.breachRepair = 0.f;

	room.crewMoving.clear();
	room.crew.clear();
	room.intruders.clear();

	// Crew
	auto iterate = [&](auto& v) {
		for (auto&& c : v)
		{
			if (c.room != room.id) continue;

			if (c.moving)
			{
				room.crewMoving.push_back(&c);
				continue;
			}

			if (c.intruder)
			{
				room.intruders.push_back(&c);
			}
			else
			{
				room.crew.push_back(&c);
			}
		}
	};

	iterate(crew);
	iterate(enemyCrew);
}

// Only call after room is read
void readRoomSlots(
	Room& room,
	const raw::Room& raw,
	const raw::Spreader<raw::Fire>& fires,
	const raw::gcc::vector<raw::OuterHull*>& hull)
{
	int slotCount = room.tiles.x * room.tiles.y;
	room.slots.resize(slotCount, Slot{});

	for (auto&& slot : room.slots)
	{
		slot.crewMoving.clear();
		slot.crew = nullptr;
		slot.intruder = nullptr;
		slot.fire = std::nullopt;
		slot.breach = std::nullopt;
		slot.occupiable = true;
	}

	room.slotsOccupiable = slotCount;

	for (size_t i = 0; i < raw.filledSlots.size(); i++)
	{
		int filled = raw.filledSlots[i];

		if (filled >= 0 && filled < slotCount && room.slots[filled].occupiable)
		{
			room.slots[filled].occupiable = false;
			room.slotsOccupiable--;
		}
	}

	for (int i = 0; i < slotCount; i++)
	{
		auto&& slot = room.slots[i];
		slot.id = i;
		slot.position.x = i % room.tiles.x;
		slot.position.y = i / room.tiles.x;
		slot.rect.x = room.rect.x + slot.position.x * Room::HARDCODED_TILE_SIZE;
		slot.rect.y = room.rect.y + slot.position.y * Room::HARDCODED_TILE_SIZE;
		slot.rect.w = Room::HARDCODED_TILE_SIZE;
		slot.rect.h = Room::HARDCODED_TILE_SIZE;

		// Crew
		for (auto&& c : room.crewMoving)
		{
			if (slot.rect.contains(c->position))
			{
				c->slot = slot.id;
				slot.crewMoving.push_back(c);
			}
		}

		for (auto&& c : room.crew)
		{
			if (slot.rect.contains(c->position))
			{
				c->slot = slot.id;
				slot.crew = c;
			}
		}

		for (auto&& c : room.intruders)
		{
			if (slot.rect.contains(c->position))
			{
				c->slot = slot.id;
				slot.intruder = c;
			}
		}
	}

	// Fires
	for (size_t xt = 0; xt < fires.grid.size(); xt++)
	{
		auto&& row = fires.grid[xt];

		for (size_t yt = 0; yt < row.size(); yt++)
		{
			auto&& cell = row[yt];
			if (cell.fDamage <= 0.f || cell.roomId != room.id) continue;
			auto&& slot = room.slotAt({ cell.pLoc.x, cell.pLoc.y });

			room.fireRepair += cell.fDamage / 100.f;

			slot.fire = Fire{};
			slot.fire->repairProgress = (100.f - cell.fDamage) / 100.f;
			slot.fire->position.x = cell.pLoc.x;
			slot.fire->position.y = cell.pLoc.y;
			slot.fire->room = cell.roomId;
			slot.fire->slot = slot.id;
			slot.fire->deathTimer = cell.fDeathTimer;
		}
	}

	// Breaches
	for (size_t i = 0; i < hull.size(); i++)
	{
		auto&& cell = *hull[i];
		if (cell.fDamage <= 0.f || cell.roomId != room.id) continue;
		auto&& slot = room.slotAt({ cell.pLoc.x, cell.pLoc.y });

		room.breachRepair += cell.fDamage / 100.f;

		slot.breach = Breach{};
		slot.breach->repairProgress = (100.f - cell.fDamage) / 100.f;
		slot.breach->position = { cell.pLoc.x, cell.pLoc.y };
		slot.breach->room = cell.roomId;
		slot.breach->slot = slot.id;
	}
}

void readDoor(Door& door, const raw::Door& raw, bool airlock = false, int id = -1)
{
	door.id = airlock ? id : raw.iDoorId; // iDoorId is set to -1 for airlocks, which is weird and useless for us
	door.rooms = { raw.iRoom1, raw.iRoom2 };
	door.level = raw.iBlast;
	door.health = { raw.health, raw.baseHealth };
	door.hackLevel = HackLevel(raw.iHacked);
	door.open = raw.bOpen;
	door.openFake = raw.bFakeOpen;
	door.ioned = raw.bIoned;
	door.vertical = raw.bVertical;
	door.airlock = airlock; // game stores airlocks in different array rather than flagging doors
	door.position = { raw.x, raw.y };
	door.dimensions = { raw.width, raw.height };
}

void readGenericShipStuff(
	Ship& ship,
	std::vector<Crew>& crew,
	std::vector<Crew>& enemyCrew,
	const raw::ShipManager& raw,
	const raw::PowerManager& power)
{
	ship.player = raw.iShipId == 0;
	ship.destroyed = raw.bDestroyed;
	ship.automated = raw.bAutomated;
	ship.jumping = raw.bJumping;
	ship.canJump = raw.lastJumpReady;
	ship.canInventory = false; // TO DO

	ship.jumpTimer = raw.jump_timer;

	ship.hull = raw.ship.hullIntegrity;
	ship.superShields = raw.shieldSystem->shields.power.super;

	ship.reactor.cap = calculatePowerCap(
		power.currentPower.second,
		power.iTempPowerCap,
		power.iTempPowerLoss,
		power.iTempDividePower);

	ship.reactor.level = power.currentPower.second;
	ship.reactor.normal.first = ship.reactor.cap-power.currentPower.first;
	ship.reactor.normal.second = ship.reactor.cap;
	ship.reactor.battery.first = power.batteryPower.second - power.batteryPower.first;
	ship.reactor.battery.second = power.batteryPower.second;

	ship.reactor.total.first =
		ship.reactor.normal.first + ship.reactor.battery.first;
	ship.reactor.total.second =
		ship.reactor.normal.second + ship.reactor.battery.second;

	// Clear systems
	ship.shields = std::nullopt;
	ship.engines = std::nullopt;
	ship.medbay = std::nullopt;
	ship.clonebay = std::nullopt;
	ship.oxygen = std::nullopt;
	ship.teleporter = std::nullopt;
	ship.cloaking = std::nullopt;
	ship.mindControl = std::nullopt;
	ship.hacking = std::nullopt;
	ship.weapons = std::nullopt;
	ship.drones = std::nullopt;
	ship.piloting = std::nullopt;
	ship.sensors = std::nullopt;
	ship.doorControl = std::nullopt;
	ship.battery = std::nullopt;
	ship.artillery.clear();

	// Also clear total oxygen since that's read from the system
	ship.totalOxygen = 0.f;

	// Room stuffs
	ship.rooms.clear();
	for (size_t i = 0; i < raw.ship.vRoomList.size(); i++)
	{
		auto&& room = ship.rooms.emplace_back();
		auto&& rawRoom = *raw.ship.vRoomList[i];

		readRoom(room, crew, enemyCrew, rawRoom);
		readRoomSlots(room, rawRoom, raw.fireSpreader, raw.ship.vOuterWalls);
	}

	// Doors
	ship.doors.clear();

	for (size_t i = 0; i < raw.ship.vDoorList.size(); i++)
	{
		readDoor(ship.doors.emplace_back(), *raw.ship.vDoorList[i]);
	}

	for (size_t i = 0; i < raw.ship.vOuterAirlocks.size(); i++)
	{
		// airlocks are special and need to have an id assigned to them from here
		size_t begin = raw.ship.vDoorList.size();
		readDoor(ship.doors.emplace_back(), *raw.ship.vOuterAirlocks[i], true, begin+i);
	}

	// Read systems
	for (size_t i = 0; i < raw.vSystemList.size(); i++)
	{
		auto current = raw.vSystemList[i];
		SystemType sys = SystemType(current->iSystemType);
		ship.rooms[current->roomId].system = sys;

		switch (sys)
		{
		case SystemType::Shields:
			ship.shields = ShieldSystem{};
			readShieldSystem(*ship.shields, *static_cast<raw::Shields*>(current));
			break;
		case SystemType::Engines:
			ship.engines = EngineSystem{};
			readEngineSystem(*ship.engines, *static_cast<raw::EngineSystem*>(current));
			break;
		case SystemType::Oxygen:
		{
			auto& oxygen = *static_cast<raw::OxygenSystem*>(current);
			ship.oxygen = OxygenSystem{};
			readOxygenSystem(*ship.oxygen, oxygen);
			ship.totalOxygen = oxygen.fTotalOxygen;
			break;
		}
		case SystemType::Weapons:
			ship.weapons = WeaponSystem{};
			readWeaponSystem(*ship.weapons, *static_cast<raw::WeaponSystem*>(current));
			break;
		case SystemType::Drones:
			ship.drones = DroneSystem{};
			readDroneSystem(*ship.drones, *static_cast<raw::DroneSystem*>(current), ship.superShields.first);
			break;
		case SystemType::Medbay:
			ship.medbay = MedbaySystem{};
			readMedbaySystem(
				*ship.medbay,
				*static_cast<raw::MedbaySystem*>(current),
				raw.ship.vRoomList);
			break;
		case SystemType::Piloting:
			ship.piloting = PilotingSystem{};
			readPilotingSystem(*ship.piloting, *current);
			break;
		case SystemType::Sensors:
			ship.sensors = SensorSystem{};
			readSensorsSystem(*ship.sensors, *current);
			break;
		case SystemType::Doors:
			ship.doorControl = DoorSystem{};
			readDoorSystem(*ship.doorControl, *current);
			break;
		case SystemType::Teleporter:
			ship.teleporter = TeleporterSystem{};
			readTeleporterSystemGeneric(
				*ship.teleporter, 
				*static_cast<raw::TeleportSystem*>(current),
				crew);
			break;
		case SystemType::Cloaking:
			ship.cloaking = CloakingSystem{};
			readCloakingSystem(*ship.cloaking, *static_cast<raw::CloakingSystem*>(current));
			break;
		case SystemType::Artillery:
		{
			auto& newArtillery = ship.artillery.emplace_back();
			readArtillerySystem(newArtillery, *static_cast<raw::ArtillerySystem*>(current));
			break;
		}
		case SystemType::battery:
			ship.battery = BatterySystem{};
			readBatterySystem(*ship.battery, *static_cast<raw::BatterySystem*>(current));
			break;
		case SystemType::Clonebay:
			ship.clonebay = ClonebaySystem{};
			readClonebaySystem(
				*ship.clonebay,
				*static_cast<raw::CloneSystem*>(current),
				crew);
			break;
		case SystemType::MindControl:
			ship.mindControl = MindControlSystem{};
			readMindControlSystem(*ship.mindControl, *static_cast<raw::MindSystem*>(current));
			break;
		case SystemType::Hacking:
			ship.hacking = HackingSystem{};
			readHackingSystem(*ship.hacking, *static_cast<raw::HackingSystem*>(current));
			break;
		}
	}

	ship.evasion = calculateEvasion(ship.engines, ship.piloting, ship.cloaking);
}

// A lot of the ship info comes from the CommandGui, oddly enough
void readPlayerShip(
	Ship& ship,
	std::vector<Crew>& crew,
	std::vector<Crew>& enemyCrew,
	const raw::CommandGui& gui,
	const raw::PowerManager& power)
{
	if (!gui.shipStatus.ship) return;

	auto&& rawReactor = 

	ship.cargo.scrap = gui.shipStatus.lastScrap;
	ship.cargo.fuel = gui.shipStatus.lastFuel;
	ship.cargo.missiles = gui.shipStatus.lastMissiles;
	ship.cargo.droneParts = gui.shipStatus.lastDrones;

	ship.cargo.weapons.clear();
	ship.cargo.drones.clear();
	ship.cargo.augments.clear();

	auto&& boxes = gui.equipScreen.vEquipmentBoxes;
	size_t weaponSlots = gui.shipStatus.ship->myBlueprint.weaponSlots;
	size_t droneSlots = gui.shipStatus.ship->myBlueprint.droneSlots;
	size_t storageStart = weaponSlots + droneSlots;

	// We start after the equipped items, only counting those in storage
	for (size_t i = storageStart; i < boxes.size(); i++)
	{
		auto&& equipment = boxes[i]->item;

		if (equipment.pWeapon) // weapon
		{
			auto&& weapon = ship.cargo.weapons.emplace_back();

			readWeapon(weapon, *equipment.pWeapon);
		}

		if (equipment.pDrone) // drone
		{
			auto&& drone = ship.cargo.drones.emplace_back();

			readDrone(drone, *equipment.pDrone);
		}

		if (equipment.augment) // augment
		{
			auto&& augment = ship.cargo.augments.emplace_back();

			readAugment(augment, *equipment.augment);
		}
	}

	readGenericShipStuff(ship, crew, enemyCrew, *gui.shipStatus.ship, power);

	// Read player teleporter stuff
	if (ship.teleporter)
	{
		auto&& teleporter = *ship.teleporter;

		teleporter.targetRoom = gui.combatControl.teleportCommand.first;
		teleporter.sending = gui.combatControl.teleportCommand.second == 1;
		teleporter.receiving = gui.combatControl.teleportCommand.second == 2;
	}
}

void readEnemyShip(
	Ship& ship,
	std::vector<Crew>& crew,
	std::vector<Crew>& enemyCrew,
	const raw::CompleteShip& raw,
	const raw::PowerManager& power)
{
	// Don't care about these values for enemies
	ship.cargo.scrap = 0;
	ship.cargo.fuel = 0;

	// Only care if respective systems are present
	// Plus I only know how to retrieve the values in these cases
	ship.cargo.missiles = raw.shipManager->weaponSystem
		? raw.shipManager->weaponSystem->missile_count
		: 0;
	ship.cargo.droneParts = raw.shipManager->droneSystem
		? raw.shipManager->droneSystem->drone_count
		: 0;

	ship.cargo.augments.clear();

	// AI only carries augments in its cargo...
	// and unlike the player, we can actually access it through the blueprint?
	// FTL sure is amazing on the inside...
	auto&& augList = raw.shipManager->myBlueprint.augments;
	auto&& augMap = Reader::getState().blueprints.augmentBlueprints;

	for (size_t i = 0; i < augList.size(); i++)
	{
		std::string augName = augList[i].str;

		if (!augMap.count(augName)) continue;

		ship.cargo.augments.emplace_back(augMap.at(augName));
	}

	readGenericShipStuff(ship, crew, enemyCrew, *raw.shipManager, power);

	if (ship.teleporter)
	{
		auto&& teleporter = *ship.teleporter;

		teleporter.targetRoom = raw.teleTargetRoom;
		teleporter.sending = raw.teleTargetRoom >= 0 && (!teleporter.crewPresent.empty() || !raw.leavingParty.empty());
		teleporter.receiving = !raw.arrivingParty.empty();
	}
}

void readCrewBlueprint(CrewBlueprint& blueprint, const raw::CrewBlueprint& raw)
{
	readBlueprint(blueprint, raw);

	blueprint.name = raw.crewName.data.str;
	blueprint.nameLong = raw.crewNameLong.data.str;
	blueprint.species = raw.name.str;
	blueprint.male = raw.male;
	blueprint.skillPiloting = raw.skillLevel[0];
	blueprint.skillEngines = raw.skillLevel[1];
	blueprint.skillShields = raw.skillLevel[2];
	blueprint.skillWeapons = raw.skillLevel[3];
	blueprint.skillRepair = raw.skillLevel[4];
	blueprint.skillCombat = raw.skillLevel[5];
}

void readCrew(Crew& crew, const raw::CrewMember& raw)
{
	readCrewBlueprint(crew.blueprint, raw.blueprint);

	crew.position.x = raw.x;
	crew.position.y = raw.y;
	crew.goal.x = raw.goal_x;
	crew.goal.y = raw.goal_y;
	crew.speed.x = raw.speed_x;
	crew.speed.y = raw.speed_y;

	crew.health = raw.health;

	crew.path.start.x = raw.path.start.x;
	crew.path.start.y = raw.path.start.y;
	crew.path.finish.x = raw.path.finish.x;
	crew.path.finish.y = raw.path.finish.y;
	crew.path.distance = raw.path.distance;
	crew.path.doors.clear();

	for (size_t i = 0; i < raw.path.doors.size(); i++)
	{
		auto&& door = crew.path.doors.emplace_back();

		door = raw.path.doors[i]->iDoorId; // assuming will never use airlock doors
	}

	crew.player = raw.iShipId == 0;
	crew.onPlayerShip = raw.currentShipId == 0;
	crew.newPath = raw.new_path;
	crew.suffocating = raw.bSuffocating;
	crew.repairing = raw.currentRepair != nullptr;
	crew.intruder = raw.intruder;
	crew.fighting = raw.bFighting;
	crew.dead = raw.bDead;
	crew.manning = raw.bActiveManning;
	crew.healing = raw.fMedbay > 0.f;
	crew.onFire = raw.iOnFire;

	crew.room = raw.iRoomId;
	crew.mannedSystem = SystemType(raw.iManningId);
	crew.roomGoal = raw.currentSlot.roomId;
	crew.slotGoal = raw.currentSlot.slotId;
	crew.roomSaved = raw.savedPosition.roomId;
	crew.slotSaved = raw.savedPosition.slotId;

	crew.deathId = raw.iDeathNumber;
	crew.cloneDeathProgress = raw.fCloneDying;
	crew.readyToClone = raw.clone_ready;
	crew.moving = crew.path.distance > 0.f;

	crew.mindControlled = raw.bMindControlled;
	crew.mindControlHealthBoost = raw.healthBoost;
	crew.mindControlDamageMultiplier = raw.fMindDamageBoost;

	crew.stunTime = raw.fStunTime;

	crew.drone = raw.crewAnim->bDrone;
}

void readCrewTeleportInfo(
	Crew& crew,
	const raw::CrewMember* ptr,
	const raw::gcc::vector<raw::CrewMember*>& arriving,
	const raw::gcc::vector<raw::CrewMember*>& leaving)
{

	crew.leaving = false;
	crew.arriving = false;

	// Check if teleporting
	for (size_t j = 0; j < arriving.size(); j++)
	{
		if (arriving[j] == ptr)
		{
			crew.arriving = true;
			break;
		}
	}

	for (size_t j = 0; j < leaving.size(); j++)
	{
		if (leaving[j] == ptr)
		{
			crew.leaving = true;
			break;
		}
	}

	crew.teleporting = crew.leaving || crew.arriving;

	// Get animation info
	if (crew.teleporting)
	{
		auto&& anim = ptr->crewAnim->anims[0][6].tracker;

		crew.teleportTimer.first = anim.reverse
			? anim.time*2.f - anim.current_time
			: anim.current_time;

		crew.teleportTimer.second = anim.time*2.f;
	}
}

using OptionalRawCrewVectorRef = std::optional<const raw::gcc::vector<raw::CrewMember*>&>;

void readPlayerCrewList(
	std::vector<Crew>& crew,
	const raw::gcc::vector<raw::CrewMember*>& list,
	const raw::gcc::vector<raw::CrewBox*>* crewBoxes,
	const raw::gcc::vector<raw::CrewMember*>* arriving = nullptr,
	const raw::gcc::vector<raw::CrewMember*>* leaving = nullptr)
{
	crew.clear();

	for (size_t i = 0; i < list.size(); i++)
	{
		if (list[i]->bDead && !list[i]->clone_ready) // permanently dead
			continue;

		if (list[i]->iShipId == 0) // must belong to player
		{
			readCrew(crew.emplace_back(), *list[i]);

			// Search UI boxes for this crew member
			for (size_t j = 0; j < crewBoxes->size(); j++)
			{
				if ((*crewBoxes)[j]->pCrew == list[i])
				{
					crew.back().uiBox = int(j);
					break;
				}
			}

			if (arriving && leaving)
			{
				readCrewTeleportInfo(
					crew.back(),
					list[i],
					*arriving, *leaving
				);
			}
		}
	}
}

void readEnemyCrewList(
	std::vector<Crew>& crew,
	const raw::gcc::vector<raw::CrewMember*>& list,
	const raw::gcc::vector<raw::CrewMember*>* arriving = nullptr,
	const raw::gcc::vector<raw::CrewMember*>* leaving = nullptr)
{
	crew.clear();

	for (size_t i = 0; i < list.size(); i++)
	{
		if (list[i]->bDead && !list[i]->clone_ready) // permanently dead
			continue;

		if (list[i]->iShipId == 1) // must belong to enemy
		{
			readCrew(crew.emplace_back(), *list[i]);

			if (arriving && leaving)
			{
				readCrewTeleportInfo(
					crew.back(),
					list[i],
					*arriving, *leaving
				);
			}
		}
	}
}

void readProjectile(Projectile& projectile, const raw::Projectile& raw)
{
	projectile.type = ProjectileType(raw.getType());
	projectile.position = { raw.position.x, raw.position.y };
	projectile.positionLast = { raw.last_position.x, raw.last_position.y };
	projectile.target = { raw.target.x, raw.target.y };
	projectile.speed = { raw.speed.x, raw.speed.y };
	projectile.lifespan = raw.lifespan;
	projectile.heading = raw.heading;
	projectile.entryAngle = raw.entryAngle;
	projectile.angle = 0.f;
	projectile.spinSpeed = 0.f;
	projectile.player = raw.ownerId == 0;
	projectile.playerSpace = raw.currentSpace == 0;
	projectile.playerSpaceIsDestination = raw.destinationSpace == 0;
	projectile.dead = raw.dead || raw.startedDeath;
	projectile.missed = raw.missed;
	projectile.hit = raw.hitTarget;
	projectile.passed = raw.passedTarget;

	// FTL fragments its data in weird ways yet again!
	// Checking misses is so hard for some reason...
	switch (projectile.type)
	{
	case ProjectileType::Laser:
	{
		auto&& laser = static_cast<const raw::LaserBlast&>(raw);
		projectile.angle = laser.spinAngle;
		projectile.spinSpeed = laser.spinSpeed;
		break;
	}
	case ProjectileType::Asteroid:
	{
		auto&& asteroid = static_cast<const raw::Asteroid&>(raw);
		projectile.angle = asteroid.angle;
		projectile.spinSpeed = 10.f; // hardcoded?
		break;
	}
	case ProjectileType::Bomb:
	{
		auto&& bomb = static_cast<const raw::BombProjectile&>(raw);
		projectile.bomb = Bomb{};
		projectile.missed = bomb.bMissed;
		projectile.hit = !bomb.bMissed;
		projectile.bomb->explosionTimer = bomb.explosiveDelay;
		projectile.bomb->damagedSuperShield = bomb.bSuperShield;
		projectile.bomb->bypassedSuperShield = bomb.superShieldBypass;
		break;
	}
	case ProjectileType::Beam:
	{
		auto&& beam = static_cast<const raw::BeamWeapon&>(raw);
		projectile.beam = Beam{};
		projectile.beam->begin = { beam.target2.x, beam.target2.y };
		projectile.beam->end = { beam.target3.x, beam.target3.y };
		projectile.beam->length = beam.length;
		projectile.beam->pierced = beam.piercedShield;
		projectile.beam->damagedSuperShield = beam.bDamageSuperShield;
		projectile.lifespan = beam.lifespan;
		projectile.missed = false;
		projectile.hit = true;
		break;
	}
	}
}

void readSpace(Space& space, const raw::SpaceManager& raw)
{
	// Projectile are hard
	space.projectiles.clear();

	for (size_t i = 0; i < raw.projectiles.size(); i++)
	{
		readProjectile(space.projectiles.emplace_back(), *raw.projectiles[i]);
	}

	// Hazard stuffs
	if (raw.sunLevel) space.environment = EnvironmentType::CloseToSun;
	if (raw.pulsarLevel) space.environment = EnvironmentType::Pulsar;
	if (raw.bPDS) space.environment = EnvironmentType::ASB;
	if (raw.bNebula) space.environment = EnvironmentType::Nebula;
	if (raw.bStorm) space.environment = EnvironmentType::IonStorm;
	space.environmentTargetingEnemy = raw.envTarget == 1;
	space.hazardTimer = { raw.flashTracker.current_time, raw.flashTracker.time };
	space.asteroids = std::nullopt;

	if (raw.asteroidGenerator.bRunning)
	{
		space.environment = EnvironmentType::Asteroids;

		auto&& gen = raw.asteroidGenerator;
		space.asteroids = AsteroidInfo{};
		auto&& info = *space.asteroids;

		for (size_t i = 0; i < 3; i++)
		{
			auto&& rate = info.spawnRates[i];
			auto&& len = info.stateLengths[i];
			auto&& rawRate = gen.spawnRate[i];
			auto&& rawLen = gen.stateLength[i];

			rate.min = float(rawRate.min)/1000.f;
			rate.max = float(rawRate.max)/1000.f;
			rate.chanceNone = rawRate.chanceNone;
			len.min = float(rawLen.min);
			len.max = float(rawLen.max);
			len.chanceNone = rawLen.chanceNone;
		}

		info.shipCount = gen.numberOfShips;
		info.state = gen.iState;
		info.playerSpace = gen.currentSpace == 0;
		info.nextDirection = gen.iNextDirection;
		info.stateTimer = gen.fStateTimer;
		info.running = gen.bRunning;
		info.shieldLevel = gen.initShields;
	}
}

void readStore(Store& store, const raw::Store& raw)
{
	int sections = raw.sectionCount;
	int boxes = sections * Store::HARDCODED_BOXES_PER_SECTION;

	store.boxes.clear();
	store.sections.clear();

	for (int i = 0; i < sections; i++)
	{
		store.sections.push_back(StoreBoxType(raw.types[i]));
	}

	for (int i = 0; i < boxes; i++)
	{
		int section = i / Store::HARDCODED_BOXES_PER_SECTION;
		auto&& rawBox = *raw.vStoreBoxes[i];

		auto&& box = store.boxes.emplace_back();
		box.type = store.sections[section];
		box.actualPrice = rawBox.desc.cost;
		box.id = i;
		box.page2 = section >= 2;

		switch (box.type)
		{
		case StoreBoxType::Weapon:
			box.weapon = WeaponBlueprint{};
			readWeaponBlueprint(*box.weapon, static_cast<const raw::WeaponBlueprint&>(*rawBox.pBlueprint));
			break;
		case StoreBoxType::Drone:
			box.drone = DroneBlueprint{};
			readDroneBlueprint(*box.drone, static_cast<const raw::DroneBlueprint&>(*rawBox.pBlueprint));
			break;
		case StoreBoxType::Augment:
			box.augment = Augment{};
			readAugment(*box.augment, static_cast<const raw::AugmentBlueprint&>(*rawBox.pBlueprint));
			break;
		case StoreBoxType::Crew:
			box.crew = CrewBlueprint{};
			readCrewBlueprint(*box.crew, static_cast<const raw::CrewBlueprint&>(*rawBox.pBlueprint));
			break;
		case StoreBoxType::System:
			box.system = SystemBlueprint{};
			readSystemBlueprint(*box.system, static_cast<const raw::SystemBlueprint&>(*rawBox.pBlueprint));
			break;
		}
	}

	store.fuel = raw.vStoreBoxes[boxes]->count;
	store.fuelCost = raw.vStoreBoxes[boxes]->desc.cost;
	store.missiles = raw.vStoreBoxes[boxes + 1]->count;
	store.missileCost = raw.vStoreBoxes[boxes + 1]->desc.cost;
	store.droneParts = raw.vStoreBoxes[boxes + 2]->count;
	store.dronePartCost = raw.vStoreBoxes[boxes + 2]->desc.cost;
	store.repairCost = raw.vStoreBoxes[boxes + 3]->desc.cost;

	auto&& hull = Reader::getState().game->playerShip->hull;
	int lost = hull.second - hull.first;

	store.repairCostFull = store.repairCost * lost;

	store.page2 = raw.bShowPage2;
}

void readResourceEvent(ResourceEvent& event, const raw::ResourceEvent& raw)
{
	event.missiles = raw.missiles;
	event.fuel = raw.fuel;
	event.droneParts = raw.drones;
	event.scrap = raw.scrap;
	event.crew = raw.crew;
	event.traitor = raw.traitor;
	event.cloneable = raw.cloneable;
	event.steal = raw.steal;
	event.intruders = raw.intruders;

	if (raw.weapon)
	{
		event.weapon = WeaponBlueprint{};
		readWeaponBlueprint(*event.weapon, *raw.weapon);
	}

	if (raw.drone)
	{
		event.drone = DroneBlueprint{};
		readDroneBlueprint(*event.drone, *raw.drone);
	}

	if (raw.augment)
	{
		event.augment = Augment{};
		readAugment(*event.augment, *raw.augment);
	}

	event.crewType = raw.crewType.str;
	event.fleetDelay = raw.fleetDelay;
	event.hullDamage = raw.hullDamage;
	event.system = SystemType(raw.upgradeId);
	event.upgradeAmount = raw.upgradeAmount;
	event.removeAugment = raw.removeItem.str;
}

// preserveStore stops the store data from being cleared
// This is just for the edge case where you cheat in a store event
// and then cheat in another event that removes the store from the current event
// but the store still exists and needs accessing somehow!
void readLocationEvent(LocationEvent& event, const raw::LocationEvent& raw, bool preserveStore = false)
{
	if (!preserveStore) event._storePtr = nullptr;

	event.environment = EnvironmentType(raw.environment);
	event.environmentTargetsEnemy = raw.environmentTarget == 1;
	event.exit = raw.beacon;
	event.distress = raw.distressBeacon;
	event.revealMap = raw.reveal_map;
	event.repair = raw.repair;
	event.unlockShip = raw.unlockShip;

	event.ship = std::nullopt;
	readResourceEvent(event.resources, raw.stuff);
	readResourceEvent(event.reward, raw.reward);
	if(!event._storePtr) event.store = std::nullopt;
	event.damage.clear();
	event.choices.clear();

	if (raw.ship.present)
	{
		event.ship = ShipEvent{};
		event.ship->hostile = raw.ship.hostile;
		event.ship->surrenderThreshold = {
			raw.ship.surrenderThreshold.min,
			raw.ship.surrenderThreshold.max,
			raw.ship.surrenderThreshold.chanceNone
		};
		event.ship->escapeThreshold = {
			raw.ship.escapeThreshold.min,
			raw.ship.escapeThreshold.max,
			raw.ship.escapeThreshold.chanceNone
		};
	}

	event.boarders.crewType = raw.boarders.type.str;
	event.boarders.min = raw.boarders.min;
	event.boarders.max = raw.boarders.max;
	event.boarders.amount = raw.boarders.amount;
	event.boarders.breach = raw.boarders.breach;

	if (raw.pStore)
	{
		event._storePtr = raw.pStore; // update store pointer
		event.store = Store{};
		readStore(*event.store, *raw.pStore);
	}

	for (size_t i = 0; i < raw.damage.size(); i++)
	{
		event.damage.push_back({
			.system = SystemType(raw.damage[i].system),
			.amount = raw.damage[i].amount,
			.effect = raw.damage[i].effect
		});
	}
	
	for (size_t i = 0; i < raw.choices.size(); i++)
	{
		auto&& choice = event.choices.emplace_back();
		auto&& rawChoice = raw.choices[i];
	
		// NOTE: this makes an assumption that events are not circular
		// which I believe the game also makes,
		// as circular events crash it upon loading
		if (rawChoice.event)
		{
			choice.event = std::make_shared<LocationEvent>();
			readLocationEvent(*choice.event, *rawChoice.event);
		}
	
		choice.blue = rawChoice.requirement.blue;
	}
}

// id & neighbors are read in the readStarMap function
void readLocation(Location& location, const raw::Location& raw)
{
	location.position = { raw.loc.x, raw.loc.y };
	location.visits = raw.visited;
	location.known = raw.known;
	location.exit = raw.beacon;
	location.hazard = raw.dangerZone;
	location.nebula = raw.nebula;
	location.flagshipPresent = raw.boss;
	location.quest = raw.questLoc;
	location.fleetOvertaking = raw.fleetChanging;
	location.enemyShip = raw.event && raw.event->ship.present && raw.event->ship.hostile;
	if(raw.event) readLocationEvent(location.event, *raw.event);
}

// id & neighbors are read in the readStarMap function
void readSector(Sector& sector, const raw::Sector& raw)
{
	sector.type = SectorType(raw.type);
	sector.name = raw.description.name.data.str;
	sector.visited = raw.visited;
	sector.reachable = raw.reachable;
	sector.position = { raw.location.x, raw.location.y };
	sector.level = raw.level;
	sector.unique = raw.description.unique;
}

void readStarMap(StarMap& map, const raw::StarMap& raw)
{
	map.lastStand = raw.bossLevel;
	map.flagshipJumping = raw.bossJumping;
	map.mapRevealed = raw.bMapRevealed;
	map.secretSector = raw.secretSector;
	map.translation = { raw.translation.x, raw.translation.y };
	map.dangerZone.center = { float(raw.dangerZone.x), float(raw.dangerZone.y) };
	map.dangerZone.a = raw.dangerZoneRadius;
	map.dangerZone.b = raw.dangerZoneRadius;
	map.pursuitDelay = raw.pursuitDelay;
	map.turnsLeft = raw.arrivedAtBase == 0 ? -1 : 4 - raw.arrivedAtBase;
	map.choosingNewSector = raw.bChoosingNewSector;
	map.infiniteMode = raw.bInfiniteMode;
	map.nebulaSector = raw.bNebulaMap;
	map.distressBeacon = raw.distressAnim.running;
	map.sectorNumber = raw.worldLevel;

	map.locations.clear();
	map.flagshipPath.clear();
	map.sectors.clear();

	std::unordered_map<raw::Location*, size_t> locIndex;
	std::unordered_map<raw::Sector*, size_t> secIndex;

	// read locations
	for (size_t i = 0; i < raw.locations.size(); i++)
	{
		readLocation(map.locations.emplace_back(), *raw.locations[i]);
		map.locations.back().id = int(i);
		locIndex[raw.locations[i]] = i;
	}

	// read location neighbors
	for (size_t i = 0; i < raw.locations.size(); i++)
	{
		auto&& loc = *raw.locations[i];

		for (size_t j = 0; j < loc.connectedLocations.size(); j++)
		{
			size_t idx = locIndex.at(loc.connectedLocations[j]);
			map.locations[i].neighbors.push_back(&map.locations[idx]);
		}
	}

	// read flagship path
	for (size_t i = 0; i < raw.boss_path.size(); i++)
	{
		auto&& loc = *raw.boss_path[i];
		size_t idx = locIndex.at(raw.boss_path[i]);
		map.flagshipPath.push_back(&map.locations[idx]);
	}

	// read current location
	{
		auto it = locIndex.find(raw.currentLoc);

		if (it == locIndex.end())
		{
			map.currentLocation = nullptr;
		}
		else
		{
			size_t idx = it->second;
			map.currentLocation = &map.locations[idx];
		}
	}

	// read sectors
	for (size_t i = 0; i < raw.sectors.size(); i++)
	{
		readSector(map.sectors.emplace_back(), *raw.sectors[i]);
		map.sectors.back().id = int(i);
		secIndex[raw.sectors[i]] = i;
	}

	// read sector neighbors
	for (size_t i = 0; i < raw.sectors.size(); i++)
	{
		auto&& sec = *raw.sectors[i];

		for (size_t j = 0; j < sec.neighbors.size(); j++)
		{
			size_t idx = secIndex.at(sec.neighbors[j]);
			map.sectors[i].neighbors.push_back(&map.sectors[idx]);
		}
	}

	// read current sector
	{
		auto it = secIndex.find(raw.currentSector);

		if (it == secIndex.end())
		{
			map.currentSector = nullptr;
		}
		else
		{
			size_t idx = it->second;
			map.currentSector = &map.sectors[idx];
		}

	}
}

void readSettings(Settings& settings, const raw::SettingValues& raw)
{
	settings.fullscreen = Settings::FullscreenMode(raw.fullscreen);
	settings.soundVolume = raw.sound;
	settings.musicVolume = raw.music;
	settings.difficulty = Settings::Difficulty(raw.difficulty);
	settings.consoleEnabled = raw.commandConsole;
	settings.pauseOnFocusLoss = raw.altPause;
	settings.touchPause = raw.touchAutoPause;
	settings.noDynamicBackgrounds = raw.lowend;
	settings.achievementPopups = raw.achPopups;
	settings.verticalSync = raw.vsync;
	settings.frameLimit = raw.frameLimit;
	settings.showBeaconPathsOnHover = raw.showPaths;
	settings.colorblindMode = raw.colorblind;
	settings.advancedEditionEnabled = raw.bDlcEnabled;
	settings.language = raw.language.str;
	settings.screenSize.x = raw.screenResolution.x;
	settings.screenSize.y = raw.screenResolution.y;
	settings.eventChoiceSelection = Settings::EventChoiceSelection(raw.dialogKeys);

	settings.hotkeys.clear();

	for (size_t i = 0; i < raw.hotkeys.size(); i++)
	{
		auto&& page = raw.hotkeys[i];

		for (size_t j = 0; j < page.size(); j++)
		{
			auto&& hotkey = settings.hotkeys.emplace(
				page[j].name.str,
				Key(page[j].key)
			);
		}
	}
}

}

bool Reader::init()
{
	base = reinterpret_cast<uintptr_t>(GetModuleHandle(L"FTLGame.exe"));;

	memcpy_s(
		&rs.app, sizeof(raw::CApp*),
		&mem::get<raw::CApp*>(base + raw::CAppPtr), sizeof(raw::CApp*));

	if (!rs.app) return false;

	rs.crewMemberFactory = &mem::get<raw::CrewMemberFactory>(base + raw::CrewMemberFactoryPtr);
	rs.settingValues = &mem::get<raw::SettingValues>(base + raw::SettingValuesPtr);
	rs.blueprints = &mem::get<raw::BlueprintManager>(base + raw::BlueprintManagerPtr);
	rs.powerManagerContainer = &mem::get<raw::PowerManagerContainer>(base + raw::PowerManagerContainerPtr);


	// Read blueprints...
	state.blueprints.weaponBlueprints.clear();
	rs.blueprints->weaponBlueprints.dfs([](const raw::gcc::string& key, const raw::WeaponBlueprint& value) {
		std::string str = key.str;
		auto&& [it, succ] = state.blueprints.weaponBlueprints.emplace(str, WeaponBlueprint{});
		auto&& weapon = it->second;
		readWeaponBlueprint(weapon, value);
	});

	state.blueprints.droneBlueprints.clear();
	rs.blueprints->droneBlueprints.dfs([](const raw::gcc::string& key, const raw::DroneBlueprint& value) {
		std::string str = key.str;
		auto&& [it, succ] = state.blueprints.droneBlueprints.emplace(str, DroneBlueprint{});
		auto&& drone = it->second;
		readDroneBlueprint(drone, value);
	});

	state.blueprints.augmentBlueprints.clear();
	rs.blueprints->augmentBlueprints.dfs([](const raw::gcc::string& key, const raw::AugmentBlueprint& value) {
		std::string str = key.str;
		auto&& [it, succ] = state.blueprints.augmentBlueprints.emplace(str, Augment{});
		auto&& aug = it->second;
		readAugment(aug, value);
	});

	state.blueprints.crewBlueprints.clear();
	rs.blueprints->crewBlueprints.dfs([](const raw::gcc::string& key, const raw::CrewBlueprint& value) {
		std::string str = key.str;
		auto&& [it, succ] = state.blueprints.crewBlueprints.emplace(str, CrewBlueprint{});
		auto&& crew = it->second;
		readCrewBlueprint(crew, value);
	});

	state.blueprints.systemBlueprints.clear();
	rs.blueprints->systemBlueprints.dfs([](const raw::gcc::string& key, const raw::SystemBlueprint& value) {
		std::string str = key.str;
		auto&& [it, succ] = state.blueprints.systemBlueprints.emplace(str, SystemBlueprint{});
		auto&& system = it->second;
		readSystemBlueprint(system, value);
	});

	readSettings(state.settings, *rs.settingValues);

	nextPoll = Clock::now();
	
	poll();

	return true;
}

void Reader::poll()
{
	state.running = rs.app && rs.app->Running;

	if (!state.running) return;

	if (Input::ready())
	{
		rs.app->focus = true;
		rs.app->inputFocus = true;
	}

	if (state.game)
	{
		auto&& game = *state.game;

		bool prevPause = game.pause.any;

		game.pause.normal = rs.app->gui->bPaused;
		game.pause.automatic = rs.app->gui->bAutoPaused;
		game.pause.menu = rs.app->gui->menu_pause;
		game.pause.event = rs.app->gui->event_pause;
		game.pause.touch = rs.app->gui->touch_pause;

		game.pause.any = game.pause.normal
			|| game.pause.automatic || game.pause.menu
			|| game.pause.event || game.pause.touch;

		game.pause.justPaused = !prevPause && game.pause.any;
		game.pause.justUnpaused = prevPause && !game.pause.any;

		if (game.justLoaded || rs.app->gui->optionsBox.bOpen)
		{
			readSettings(state.settings, *rs.settingValues);
		}

		game.gameOver = rs.app->gui->gameover;

		// Space stuffs
		readSpace(game.space, rs.app->world->space);

		// Crew stuffs
		{
			auto* playerCompleteShip = rs.app->world->playerShip;
			auto* enemyCompleteShip = playerCompleteShip
				? playerCompleteShip->enemyShip
				: nullptr;

			const raw::gcc::vector<raw::CrewMember*>
				*playerArriving = nullptr, *playerLeaving = nullptr,
				*enemyArriving = nullptr, *enemyLeaving = nullptr;

			if (playerCompleteShip)
			{
				playerArriving = &playerCompleteShip->arrivingParty;
				playerLeaving = &playerCompleteShip->leavingParty;
			}

			if (enemyCompleteShip)
			{
				enemyArriving = &enemyCompleteShip->arrivingParty;
				enemyLeaving = &enemyCompleteShip->leavingParty;
			}

			readPlayerCrewList(
				game.playerCrew,
				rs.crewMemberFactory->crewMembers,
				& rs.app->gui->crewControl.crewBoxes,
				playerArriving, playerLeaving);

			readEnemyCrewList(
				game.enemyCrew,
				rs.crewMemberFactory->crewMembers,
				enemyArriving, enemyLeaving);
		}

		auto& shipStatus = rs.app->gui->shipStatus;

		if (shipStatus.ship)
		{
			if (!game.playerShip) game.playerShip = Ship{};

			bool prevJumping = game.playerShip->jumping;

			readPlayerShip(
				*game.playerShip,
				game.playerCrew,
				game.enemyCrew,
				*rs.app->gui,
				rs.powerManagerContainer->powerManagers[0]);

			game.justJumped = prevJumping && !game.playerShip->jumping;
		}
		else
		{
			game.playerShip = std::nullopt;
			game.justJumped = false;
		}

		// For accessing the enemy's CompleteShip instance
		auto& completePlayerShip = rs.app->world->playerShip;

		if (completePlayerShip)
		{
			auto* enemy = completePlayerShip->enemyShip;

			if (enemy)
			{
				if (!game.enemyShip) game.enemyShip = Ship{};

				readEnemyShip(
					*game.enemyShip,
					game.enemyCrew,
					game.playerCrew,
					*enemy,
					rs.powerManagerContainer->powerManagers[1]);
			}
			else
			{
				game.enemyShip = std::nullopt;
			}
		}

		// Read the event stuff
		if (game.pause.event || game.pause.menu || game.justJumped || game.justLoaded)
		{
			auto&& choices = rs.app->world->choiceHistory;
			auto* current = rs.app->world->baseLocationEvent;

			// Game stores only the base event
			// so we need to traverse the event tree using the choice history
			for (size_t i = 0; i < choices.size(); i++)
			{
				if (!current) break;

				if (current->choices[i].event)
				{
					current = current->choices[i].event;
				}
			}

			if (current)
			{
				// Only null store pointer when jumping
				bool preserveStore = game.playerShip && !game.playerShip->jumping;
				readLocationEvent(game.event, *current, preserveStore);
			}
		}

		readStarMap(game.starMap, rs.app->world->starMap);

		if (game.justLoaded) game.justLoaded = false;
	}

	if (rs.app->menu.bOpen) // not in game
	{
		state.game = std::nullopt;
	}
	else if (state.game == std::nullopt) // create game object if it doesn't exist
	{
		state.game = Game{};
		state.game->pause.justUnpaused = true;
		state.game->justLoaded = true;
	}
}

void Reader::wait()
{
	// Only sleep if we polled too soon!
	std::this_thread::sleep_until(nextPoll);
	nextPoll = Clock::now() + delay;
}

void Reader::fullPoll()
{
	Reader::wait();
	Reader::poll();
	if(Input::ready()) Input::iterate();
}

void Reader::setPollDelay(const Duration& d)
{
	if (d < std::chrono::milliseconds(1))
		throw std::invalid_argument("cannot use poll delay less than 1 millisecond");

	delay = d;
}

const Duration& Reader::getPollDelay()
{
	return delay;
}

const State& Reader::getState()
{
	return state;
}

const raw::State& Reader::getRawState()
{
	return rs;
}

raw::State& Reader::getRawState(MutableRawState)
{
	return rs;
}

void* Reader::getMemory(uintptr_t offset, MutableRawState)
{
	return (void*)(base + offset);
}

void Reader::setSeperateThread(bool on)
{
	if (!on && thread.joinable()) // already running
	{
		thread.request_stop();
		thread.join();
	}
	else if(on && !thread.joinable()) // not running
	{
		std::jthread newThread([](std::stop_token stop_token) {
			while (!stop_token.stop_requested())
			{
				Reader::fullPoll();
			}
		});

		thread = std::move(newThread);
	}
}

bool Reader::usingSeperateThread()
{
	return thread.joinable();
}

void Reader::reload()
{
	reloading = true;
}

void Reader::finishReload()
{
	reloading = false;
}

bool Reader::reloadRequested()
{
	return reloading;
}

void Reader::quit()
{
	g_quit = true;
}
