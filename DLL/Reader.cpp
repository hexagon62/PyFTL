#include "State/Point.hpp"
#include "State/Rect.hpp"
#include "State/Ellipse.hpp"

#include "Reader.hpp"
#include "Input.hpp"
#include "Utility/Memory.hpp"
#include "Utility/Exceptions.hpp"

#include <algorithm>
#include <unordered_map>
#include <concepts>

Reader::TimePoint Reader::start = Reader::Clock::now();
raw::State Reader::rs;
State Reader::state;
uintptr_t Reader::base = 0;
bool Reader::started = false;
bool Reader::reloading = false;
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
	power.required = raw.iRequiredPower;
	power.normal = raw.powerState.first;
	power.zoltan = raw.iBonusPower;
	power.battery = raw.iBatteryPower;

	power.cap = calculatePowerCap(
		raw.powerState.second,
		raw.iTempPowerCap,
		raw.iTempPowerLoss,
		raw.iTempDividePower);

	power.ionLevel = raw.iLockCount;
	power.ionTimer.first = raw.lockTimer.currGoal-raw.lockTimer.currTime;
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

	system.player = raw._shipObj.iShipId == 0;
	system.onFire = raw.bOnFire;
	system.breached = raw.bBreached;
	system.boardersAttacking = raw.bUnderAttack;

	system.damageProgress = raw.fDamageOverTime / 100.f;
	system.repairProgress = raw.fRepairOverTime / 100.f;

	system.hackLevel = HackLevel(raw.iHackEffect);

	// figuring out if we're actually manned is more complicated...
	//bool limited = system.power.cap < system.level.current;
	//bool manningBlocked = raw.iHackEffect != 0 || system.boardersAttacking || limited;
	//bool needsRepairing = system.onFire || system.breached || !system.health.capped();
	//bool manned = raw.bManned && raw.bBoostable && !manningBlocked && !needsRepairing;
	//
	//system.manningLevel = manned ? raw.iActiveManned : 0;
}

void readShieldSystem(ShieldSystem& shields, const raw::Shields& raw, const Point<int>& offset)
{
	readSystem(shields, raw);
	shields.blueprint = Reader::getState().blueprints.systemBlueprints.at("shields");

	shields.boundary = raw.baseShield;

	shields.bubbles = {
		raw.shields.power.first,
		 raw.shields.power.second
	};

	shields.charge = {
		raw.shields.charger,
		shields.charge.second
	};
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

	clonebay.cloneTimer = {
		raw.fTimeToClone,
		raw.fTimeGoal
	};

	clonebay.deathTimer = {
		raw.fDeathTime,
		ClonebaySystem::HARDCODED_DEATH_TIME
	};

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

	teleporter.canSend = raw.bCanSend;
	teleporter.canReceive = raw.bCanReceive;
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

void readWeapon(Weapon& weapon, const raw::ProjectileFactory& raw, const Point<int>& offset)
{
	readWeaponBlueprint(weapon.blueprint, *raw.blueprint);
	weapon.player = raw.iShipId == 0;

	weapon.power.total.first = !raw.powered
		? raw.iBonusPower
		: raw.requiredPower;
	weapon.power.total.second = raw.requiredPower;
	weapon.power.required = raw.requiredPower;
	weapon.power.normal = weapon.power.total.first - raw.iBonusPower;
	weapon.power.zoltan = raw.iBonusPower;
	weapon.power.cap = weapon.power.total.second;

	// read these in weapon system function
	weapon.power.battery = 0; 
	weapon.power.ionLevel = 0;
	weapon.power.ionTimer = { 0.f, 0.f };
	weapon.power.restoreTo = 0;

	weapon.cooldown.first = raw.cooldown.second - raw.cooldown.first;
	weapon.cooldown.second = raw.cooldown.second;

	weapon.autofire = raw.autoFiring;
	weapon.fireWhenReady = raw.fireWhenReady;
	weapon.artillery = raw.isArtillery;
	weapon.targetingPlayer = raw.currentShipTarget == 0;
	weapon.cargo = false; // set when reading player cargo

	weapon.firingAngle = raw.currentFiringAngle;
	weapon.entryAngle = raw.currentEntryAngle;

	weapon.mount = raw.mount.position + raw.localPosition;

	weapon.hackLevel = HackLevel(raw.iHackLevel);

	weapon.boost.first = raw.boostLevel;
	weapon.boost.second = weapon.blueprint.boost.count;
	weapon.charge.first = raw.chargeLevel;
	weapon.charge.second = raw.goalChargeLevel;

	weapon.shotTimer.first = raw.weaponVisual.anim.tracker.time-raw.weaponVisual.anim.tracker.current_time;
	weapon.shotTimer.second = raw.weaponVisual.anim.tracker.time;

	weapon.targetPoints.clear();

	// Target points
	for (size_t i = 0; i < raw.targets.size(); i++)
	{
		auto&& pos = raw.targets[i];
		weapon.targetPoints.emplace_back(pos.x, pos.y);
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

void readCrew(Crew& crew, const raw::CrewMember& raw, const Point<int>& offset)
{
	readCrewBlueprint(crew.blueprint, raw.blueprint);

	crew.position = offset + Point<int>{raw.x, raw.y};
	crew.goal = offset + Point<int>{raw.goal_x, raw.goal_y};
	crew.speed = Point<int>{ raw.speed_x, raw.speed_y };

	crew.health = raw.health;

	crew.path.start = offset + raw.path.start;
	crew.path.finish = offset + raw.path.finish;
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
	crew.dying = !raw.bDead && crew.health.first <= 0.f;
	crew.dead = raw.bDead || crew.dying;
	crew.manning = raw.bActiveManning;
	crew.healing = raw.fMedbay > 0.f;
	crew.onFire = raw.iOnFire;
	crew.selectionId = 0;

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

void readDrone(
	Drone& drone,
	const raw::Drone& raw,
	const Point<int>& playerShipPos,
	const Point<int>& enemyShipPos,
	int superShieldBubbles = 0)
{
	readDroneBlueprint(drone.blueprint, *raw.blueprint);

	drone.player = raw.iShipId == 0;

	drone.power.total.first = !raw.powered
		? raw.iBonusPower
		: raw.powerRequired;
	drone.power.total.second = raw.powerRequired;
	drone.power.required = raw.powerRequired;
	drone.power.normal = drone.power.total.first - raw.iBonusPower;
	drone.power.zoltan = raw.iBonusPower;
	drone.power.cap = drone.power.total.second;

	// read these in drone system function
	drone.power.battery = 0;
	drone.power.ionLevel = 0;
	drone.power.ionTimer = { 0.f, 0.f };
	drone.power.restoreTo = 0;

	drone.hackLevel = HackLevel(raw.iHackLevel);
	drone.hackTime = raw.hackTime;
	drone.destroyTimer.first = drone.HARDCODED_REBUILD_TIME-raw.destroyedTimer;
	drone.destroyTimer.second = drone.HARDCODED_REBUILD_TIME;
	drone.deployed = raw.deployed;
	drone.dead = drone.destroyTimer.first < drone.destroyTimer.second;
	drone.dying = false;
	drone.cargo = false; // set when reading player cargo
	drone.powerUpTimer = { 0.f, 0.f };
	drone.powerDownTimer = { 0.f, 0.f };

	auto&& type = drone.blueprint.type;

	if (drone.crewDrone())
	{
		auto&& casted = static_cast<const raw::CrewDrone&>(raw);

		Point<int> position = casted.currentShipId == 0
			? playerShipPos
			: enemyShipPos;

		readCrew(drone.crew.emplace(), casted, position);

		// can't be dead if not deployed
		drone.crew->dead = (drone.deployed && drone.crew->dead) || drone.dead;

		drone.dying = drone.crew->dying;
		drone.dead = drone.crew->dead;

		auto&& upTracker = casted.powerUp.tracker;
		auto&& downTracker = casted.powerDown.tracker;

		drone.powerUpTimer = { upTracker.current_time, upTracker.time };
		drone.powerDownTimer = { downTracker.current_time, downTracker.time };
	}

	if (drone.spaceDrone())
	{
		auto&& casted = static_cast<const raw::SpaceDrone&>(raw);
		drone.space = SpaceDroneInfo{};
		auto&& info = *drone.space;

		drone.dying = casted.explosion.tracker.current_time > 0.f;
		drone.dead = drone.dead || drone.dying;

		info.playerSpace = casted.currentSpace == 0;
		info.playerSpaceIsDestination = casted.destinationSpace == 0;
		info.moving = casted.pause > 0.f;

		Point<int> position = info.playerSpace
			? playerShipPos
			: enemyShipPos;

		Point<int> destination = info.playerSpaceIsDestination
			? playerShipPos
			: enemyShipPos;
		
		info.position = casted.currentLocation + position;
		info.positionLast = casted.lastLocation + position;
		info.destination = casted.destinationLocation + destination;
		info.speed = casted.speedVector;

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
			info.weapon.emplace();
			readWeaponBlueprint(*info.weapon, *casted.weaponBlueprint);
		}
		else
		{
			info.weapon.reset();
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
			info.extraMovement.emplace();
			auto&& movement = *info.extraMovement;
			
			movement.destinationLast = casted2.lastDestination + destination;
			movement.progress = casted2.progressToDestination;
			movement.heading = casted2.heading;
			movement.headingLast = casted2.oldHeading;
		}
		else
		{
			info.extraMovement.reset();
		}
	}
	else
	{
		drone.space.reset();
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

void readWeaponSystem(WeaponSystem& weapons, const raw::WeaponSystem& raw, const Point<int>& offset)
{
	readSystem(weapons, raw);
	weapons.blueprint = Reader::getState().blueprints.systemBlueprints.at("weapons");

	weapons.list.clear();
	weapons.autoFire = false; // set when reading player ship

	for (size_t i = 0; i < raw.weapons.size(); i++)
	{
		auto& weapon = weapons.list.emplace_back();
		readWeapon(weapon, *raw.weapons[i], offset);
		weapon.slot = int(i);
		weapon.power.ionLevel = weapons.power.ionLevel;
		weapon.power.ionTimer = weapons.power.ionTimer;
	}

	// set battery power
	int dist = weapons.power.battery;
	auto it = weapons.list.rbegin();
	while (dist > 0 && it != weapons.list.rend())
	{
		int del = std::min(dist, it->power.required);
		it->power.normal -= del;
		it->power.battery = del;
		dist -= del;
		it++;
	}

	for (size_t i = 0; i < raw.repowerList.size(); i++)
	{
		if (raw.repowerList[i])
		{
			weapons.list[i].power.restoreTo = weapons.list[i].power.required;
		}
	}
}

void readDroneSystem(
	DroneSystem& drones,
	const raw::DroneSystem& raw,
	const Point<int>& playerShipPos,
	const Point<int>& enemyShipPos,
	int superShieldBubbles = 0)
{
	readSystem(drones, raw);
	drones.blueprint = Reader::getState().blueprints.systemBlueprints.at("drones");

	drones.list.clear();
	for (size_t i = 0; i < raw.drones.size(); i++)
	{
		auto& drone = drones.list.emplace_back();
		readDrone(drone, *raw.drones[i], playerShipPos, enemyShipPos, superShieldBubbles);
		drone.slot = int(i);
		drone.power.ionLevel = drones.power.ionLevel;
		drone.power.ionTimer = drones.power.ionTimer;
	}

	// set battery power
	int dist = drones.power.battery;
	auto it = drones.list.rbegin();
	while (dist > 0 && it != drones.list.rend())
	{
		int del = std::min(dist, it->power.required);
		it->power.normal -= del;
		it->power.battery = del;
		dist -= del;
		it++;
	}

	for (size_t i = 0; i < raw.repowerList.size(); i++)
	{
		if (raw.repowerList[i])
		{
			drones.list[i].power.restoreTo = drones.list[i].power.required;
		}
	}
}

void readCloakingSystem(CloakingSystem& cloaking, const raw::CloakingSystem& raw)
{
	readSystem(cloaking, raw);
	cloaking.blueprint = Reader::getState().blueprints.systemBlueprints.at("cloaking");

	cloaking.on = raw.bTurnedOn;
	cloaking.timer.first = raw.timer.currGoal - raw.timer.currTime;
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

void readArtillerySystem(ArtillerySystem& artillery, const raw::ArtillerySystem& raw, const Point<int>& offset)
{
	readSystem(artillery, raw);
	artillery.blueprint = Reader::getState().blueprints.systemBlueprints.at("artillery");

	if (raw.projectileFactory)
	{
		readWeapon(artillery.weapon, *raw.projectileFactory, offset);
	}
}

void readBatterySystem(BatterySystem& battery, const raw::BatterySystem& raw)
{
	readSystem(battery, raw);
	battery.blueprint = Reader::getState().blueprints.systemBlueprints.at("battery");

	battery.on = raw.bTurnedOn;
	battery.timer.first = raw.timer.currGoal - raw.timer.currTime;
	battery.timer.second = raw.timer.currGoal;
	battery.provides = raw.powerState.first * 2;
	battery.providing = battery.on ? battery.provides : 0;
}

void readMindControlSystem(MindControlSystem& mindControl, const raw::MindSystem& raw)
{
	readSystem(mindControl, raw);
	mindControl.blueprint = Reader::getState().blueprints.systemBlueprints.at("mind");

	mindControl.on = raw.controlTimer.first < raw.controlTimer.second;
	mindControl.timer.first = raw.controlTimer.second - raw.controlTimer.first;
	mindControl.timer.second = raw.controlTimer.second;
	mindControl.targetRoom = raw.iQueuedTarget;
	mindControl.targetingPlayerShip = raw.iQueuedShip == 0;
	mindControl.canUse = raw.bCanUse;
}

void readHackingSystem(
	HackingSystem& hacking,
	const raw::HackingSystem& raw,
	const Point<int>& playerShipPos,
	const Point<int>& enemyShipPos)
{
	readSystem(hacking, raw);
	hacking.blueprint = Reader::getState().blueprints.systemBlueprints.at("hacking");

	readDrone(hacking.drone, raw.drone, playerShipPos, enemyShipPos);

	hacking.on = raw.bHacking;
	hacking.canUse = raw.bCanHack;
	hacking.timer.first = raw.effectTimer.second - raw.effectTimer.first;
	hacking.timer.second = raw.effectTimer.second;
	hacking.target = SystemType(raw.currentSystem ? raw.currentSystem->iSystemType : -1);
	hacking.queued = SystemType(raw.queuedSystem ? raw.queuedSystem->iSystemType : -1);
	hacking.drone.start = raw.drone.startingPosition;
	hacking.drone.goal = raw.drone.finalDestination;
	hacking.drone.arrived = raw.drone.arrive;
	hacking.drone.setUp = raw.drone.finishedSetup;
	hacking.drone.room = raw.drone.prefRoom;

	hacking.drone.start += hacking.drone.player ? enemyShipPos : playerShipPos;
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

void readRoom(
	Room& room,
	std::vector<Crew>& crew,
	std::vector<Crew>& enemyCrew,
	const raw::Room& raw,
	const Point<int>& offset)
{
	room.player = raw.iShipId == 0;
	room.id = raw.iRoomId;
	room.primarySlot = raw.primarySlot;
	room.primaryDirection = raw.primaryDirection;
	
	room.rect.x = raw.rect.x + offset.x;
	room.rect.y = raw.rect.y + offset.y;
	room.rect.w = raw.rect.w;
	room.rect.h = raw.rect.h;

	room.tiles.x = raw.rect.w / Room::HARDCODED_TILE_SIZE;
	room.tiles.y = raw.rect.h / Room::HARDCODED_TILE_SIZE;

	room.visible = !raw.bBlackedOut;
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
	const raw::gcc::vector<raw::OuterHull*>& hull,
	const Point<int>& offset)
{
	int slotCount = room.tiles.x * room.tiles.y;
	room.slots.resize(slotCount, Slot{});

	for (auto&& slot : room.slots)
	{
		slot.room = &room;
		slot.crewMoving.clear();
		slot.crew = nullptr;
		slot.intruder = nullptr;
		slot.fire.reset();
		slot.breach.reset();
		slot.occupiable = true;
		slot.player = room.player;
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
			auto&& slot = room.slotAt(offset + Point<int>{ cell.pLoc.x, cell.pLoc.y });

			room.fireRepair += cell.fDamage / 100.f;

			slot.fire = Fire{};
			slot.fire->repairProgress = (100.f - cell.fDamage) / 100.f;
			slot.fire->position = offset + Point<int>{cell.pLoc.x, cell.pLoc.y};
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
		auto&& slot = room.slotAt(offset + Point<int>{ cell.pLoc.x, cell.pLoc.y });

		room.breachRepair += cell.fDamage / 100.f;

		slot.breach = Breach{};
		slot.breach->repairProgress = (100.f - cell.fDamage) / 100.f;
		slot.breach->position = offset + Point<int>{ cell.pLoc.x, cell.pLoc.y };
		slot.breach->room = cell.roomId;
		slot.breach->slot = slot.id;
	}
}

void readDoor(
	Door& door,
	const raw::Door& raw,
	const Point<int>& offset,
	bool airlock = false,
	int id = -1)
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
	door.player = raw.iShipId == 0;
	door.rect.w = raw.width;
	door.rect.h = raw.height;
	door.rect.x = offset.x + raw.x - raw.width / 2;
	door.rect.y = offset.y + raw.y - raw.height / 2;
}

void readGenericShipStuff(
	Ship& ship,
	std::vector<Crew>& crew,
	std::vector<Crew>& enemyCrew,
	const raw::ShipManager& raw,
	const raw::PowerManager& power,
	const Point<int>& playerShipPos,
	const Point<int>& enemyShipPos)
{
	ship.player = raw.iShipId == 0;
	ship.destroyed = raw.bDestroyed;
	ship.automated = raw.bAutomated;
	ship.jumping = raw.bJumping;
	ship.canJump = raw.lastJumpReady;
	ship.canInventory = false; // TO DO

	Point<int> position = ship.player
		? playerShipPos
		: enemyShipPos;

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
	ship.shields.reset();
	ship.engines.reset();
	ship.medbay.reset();
	ship.clonebay.reset();
	ship.oxygen.reset();
	ship.teleporter.reset();
	ship.cloaking.reset();
	ship.mindControl.reset();
	ship.hacking.reset();
	ship.weapons.reset();
	ship.drones.reset();
	ship.piloting.reset();
	ship.sensors.reset();
	ship.doorControl.reset();
	ship.battery.reset();
	ship.artillery.clear();

	// Also clear total oxygen since that's read from the system
	ship.totalOxygen = 0.f;

	// Room stuffs
	ship.rooms.clear();
	for (size_t i = 0; i < raw.ship.vRoomList.size(); i++)
	{
		auto&& room = ship.rooms.emplace_back();
		auto&& rawRoom = *raw.ship.vRoomList[i];

		readRoom(room, crew, enemyCrew, rawRoom, position);
		readRoomSlots(room, rawRoom, raw.fireSpreader, raw.ship.vOuterWalls, position);
	}

	// Doors
	ship.doors.clear();
	for (size_t i = 0; i < raw.ship.vDoorList.size(); i++)
	{
		readDoor(ship.doors.emplace_back(), *raw.ship.vDoorList[i], position);
	}

	for (size_t i = 0; i < raw.ship.vOuterAirlocks.size(); i++)
	{
		// airlocks are special and need to have an id assigned to them from here
		size_t begin = raw.ship.vDoorList.size();
		readDoor(ship.doors.emplace_back(), *raw.ship.vOuterAirlocks[i], position, true, begin+i);
	}

	auto&& sysBoxes = Reader::getRawState().app->gui->sysControl.sysBoxes;

	// Read systems
	for (size_t i = 0; i < raw.vSystemList.size(); i++)
	{
		if (ship.player && i >= sysBoxes.size()) break;
		auto current = ship.player ? sysBoxes[i]->pSystem : raw.vSystemList[i];
		SystemType sys = SystemType(current->iSystemType);
		ship.rooms[current->roomId].system = sys;

		switch (sys)
		{
		case SystemType::Shields:
			ship.shields.emplace();
			readShieldSystem(*ship.shields, *static_cast<raw::Shields*>(current), position);
			if (ship.player) ship.shields->uiBox = int(i);
			break;
		case SystemType::Engines:
			ship.engines.emplace();
			readEngineSystem(*ship.engines, *static_cast<raw::EngineSystem*>(current));
			if (ship.player) ship.engines->uiBox = int(i);
			break;
		case SystemType::Oxygen:
		{
			auto& oxygen = *static_cast<raw::OxygenSystem*>(current);
			ship.oxygen.emplace();
			readOxygenSystem(*ship.oxygen, oxygen);
			ship.totalOxygen = oxygen.fTotalOxygen;
			if (ship.player) ship.oxygen->uiBox = int(i);
			break;
		}
		case SystemType::Weapons:
			ship.weapons.emplace();
			readWeaponSystem(*ship.weapons, *static_cast<raw::WeaponSystem*>(current), position);
			if (ship.player) ship.weapons->uiBox = int(i);
			break;
		case SystemType::Drones:
			ship.drones.emplace();
			readDroneSystem(
				*ship.drones,
				*static_cast<raw::DroneSystem*>(current),
				playerShipPos, enemyShipPos,
				ship.superShields.first);
			if (ship.player) ship.drones->uiBox = int(i);
			break;
		case SystemType::Medbay:
			ship.medbay.emplace();
			readMedbaySystem(
				*ship.medbay,
				*static_cast<raw::MedbaySystem*>(current),
				raw.ship.vRoomList);
			if (ship.player) ship.medbay->uiBox = int(i);
			break;
		case SystemType::Piloting:
			ship.piloting.emplace();
			readPilotingSystem(*ship.piloting, *current);
			if (ship.player) ship.piloting->uiBox = int(i);
			break;
		case SystemType::Sensors:
			ship.sensors.emplace();
			readSensorsSystem(*ship.sensors, *current);
			if (ship.player) ship.sensors->uiBox = int(i);
			break;
		case SystemType::Doors:
			ship.doorControl.emplace();
			readDoorSystem(*ship.doorControl, *current);
			if (ship.player) ship.doorControl->uiBox = int(i);
			break;
		case SystemType::Teleporter:
			ship.teleporter.emplace();
			readTeleporterSystemGeneric(
				*ship.teleporter, 
				*static_cast<raw::TeleportSystem*>(current),
				crew);
			if (ship.player) ship.teleporter->uiBox = int(i);
			break;
		case SystemType::Cloaking:
			ship.cloaking.emplace();
			readCloakingSystem(*ship.cloaking, *static_cast<raw::CloakingSystem*>(current));
			if (ship.player) ship.cloaking->uiBox = int(i);
			break;
		case SystemType::Artillery:
		{
			auto& newArtillery = ship.artillery.emplace_back();
			newArtillery.discriminator = int(ship.artillery.size()-1);
			readArtillerySystem(newArtillery, *static_cast<raw::ArtillerySystem*>(current), position);
			if (ship.player) newArtillery.uiBox = int(i);
			break;
		}
		case SystemType::Battery:
			ship.battery.emplace();
			readBatterySystem(*ship.battery, *static_cast<raw::BatterySystem*>(current));
			if (ship.player) ship.battery->uiBox = int(i);
			break;
		case SystemType::Clonebay:
			ship.clonebay.emplace();
			readClonebaySystem(
				*ship.clonebay,
				*static_cast<raw::CloneSystem*>(current),
				crew);
			if (ship.player) ship.clonebay->uiBox = int(i);
			break;
		case SystemType::MindControl:
			ship.mindControl.emplace();
			readMindControlSystem(*ship.mindControl, *static_cast<raw::MindSystem*>(current));
			if (ship.player) ship.mindControl->uiBox = int(i);
			break;
		case SystemType::Hacking:
			ship.hacking.emplace();
			readHackingSystem(
				*ship.hacking,
				*static_cast<raw::HackingSystem*>(current),
				playerShipPos, enemyShipPos);
			if (ship.player) ship.hacking->uiBox = int(i);
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
	const raw::PowerManager& power,
	const Point<int>& playerShipPos,
	const Point<int>& enemyShipPos)
{
	if (!gui.shipStatus.ship) return;

	ship.cargo.scrap = gui.shipStatus.lastScrap;
	ship.cargo.fuel = gui.shipStatus.lastFuel;
	ship.cargo.missiles = gui.shipStatus.lastMissiles;
	ship.cargo.droneParts = gui.shipStatus.lastDrones;

	ship.cargo.weapons.clear();
	ship.cargo.drones.clear();
	ship.cargo.augments.clear();
	ship.cargo.overCapacity.clear();

	auto&& boxes = gui.equipScreen.vEquipmentBoxes;
	size_t weaponSlots = gui.shipStatus.ship->myBlueprint.weaponSlots;
	size_t droneSlots = gui.shipStatus.ship->myBlueprint.droneSlots;
	size_t storageStart = weaponSlots + droneSlots;

	// Iterate over cargo boxes (not equipped weapons/drones)
	for (size_t i = storageStart; i < boxes.size(); i++)
	{
		auto&& equipment = boxes[i]->item;

		if (equipment.pWeapon) // weapon
		{
			auto&& weapon = boxes[i]->slot >= 4
				? std::get<Weapon>(ship.cargo.overCapacity.emplace_back(Weapon{}))
				: ship.cargo.weapons.emplace_back();

			readWeapon(weapon, *equipment.pWeapon, { 0, 0 });
			weapon.cargo = true;
			weapon.slot = boxes[i]->slot;
		}

		if (equipment.pDrone) // drone
		{
			auto&& drone = boxes[i]->slot >= 4
				? std::get<Drone>(ship.cargo.overCapacity.emplace_back(Drone{}))
				: ship.cargo.drones.emplace_back();

			readDrone(drone, *equipment.pDrone, { 0, 0 }, { 0, 0 });
			drone.cargo = true;
			drone.slot = boxes[i]->slot;
		}

		if (equipment.augment) // augment
		{
			auto&& augment = boxes[i]->slot >= 4
				? std::get<Augment>(ship.cargo.overCapacity.emplace_back(Augment{}))
				: ship.cargo.augments.emplace_back();

			readAugment(augment, *equipment.augment);
			augment.slot = boxes[i]->slot;
		}
	}

	readGenericShipStuff(
		ship,
		crew, enemyCrew,
		*gui.shipStatus.ship, power,
		playerShipPos, enemyShipPos
	);

	// Read player teleporter stuff
	if (ship.teleporter)
	{
		auto&& teleporter = *ship.teleporter;

		teleporter.targetRoom = gui.combatControl.teleportCommand.first;
		teleporter.sending = gui.combatControl.teleportCommand.second == 1;
		teleporter.receiving = gui.combatControl.teleportCommand.second == 2;
	}

	// Read player weapon stuff
	if (ship.weapons)
	{
		ship.weapons->autoFire = gui.combatControl.weapControl.autoFiring;
	}
}

void readEnemyShip(
	Ship& ship,
	std::vector<Crew>& crew,
	std::vector<Crew>& enemyCrew,
	const raw::CompleteShip& raw,
	const raw::PowerManager& power,
	const Point<int>& playerShipPos,
	const Point<int>& enemyShipPos)
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

	// AI carries only augments in its cargo...
	// and unlike the player, we can actually access it through the blueprint?
	// FTL sure is amazing on the inside...
	auto&& augList = raw.shipManager->myBlueprint.augments;
	auto&& augMap = Reader::getState().blueprints.augmentBlueprints;

	for (size_t i = 0; i < augList.size(); i++)
	{
		std::string augName = augList[i].str;

		if (!augMap.count(augName)) continue;

		ship.cargo.augments.emplace_back(augMap.at(augName));
		ship.cargo.augments.back().slot = i;
	}

	readGenericShipStuff(
		ship,
		crew, enemyCrew,
		*raw.shipManager, power,
		playerShipPos, enemyShipPos
	);

	if (ship.teleporter)
	{
		auto&& teleporter = *ship.teleporter;

		teleporter.targetRoom = raw.teleTargetRoom;
		teleporter.sending = raw.teleTargetRoom >= 0 && (!teleporter.crewPresent.empty() || !raw.leavingParty.empty());
		teleporter.receiving = !raw.arrivingParty.empty();
	}
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

		crew.teleportTimer.first = crew.teleportTimer.second -
			anim.reverse
				? anim.time*2.f - anim.current_time
				: anim.current_time;

		crew.teleportTimer.second = anim.time*2.f;
	}
}

template<typename T>
concept CrewListInput =
	std::same_as<T, raw::CrewMember> ||
	std::same_as<T, raw::CrewBox>;

// Will read the list of crew so that
// - permanently dead crew are filtered out
// - crew on other side are filtered out
// - crew drones are filtered out
// - sorted in order of ui boxes, if applicable
// If you pass crew boxes, then the function will assume you're parsing the player ship
// since that's the only thing that'd make sense to pass crewBoxes with
template<CrewListInput T>
void readCrewList(
	std::vector<Crew>& crew,
	const raw::gcc::vector<T*>& list,
	const Point<int>& playerShipPos, const Point<int>& enemyShipPos,
	const raw::gcc::vector<raw::CrewMember*>* arriving = nullptr,
	const raw::gcc::vector<raw::CrewMember*>* leaving = nullptr,
	const raw::CrewControl* control = nullptr)
{
	constexpr bool player = std::same_as<T, raw::CrewBox>;

	crew.clear();

	std::unordered_map<raw::CrewMember*, size_t> selectionIndices;
	std::vector<raw::CrewMember*> ptrs;

	if constexpr (player) // use crew boxes so they're sorted by ui box
	{
		for (size_t i = 0; i < list.size(); i++)
		{
			auto* crew = list[i]->pCrew;

			if (crew->bDead && !crew->clone_ready) // permanently dead
				continue;

			ptrs.push_back(crew);
		}

		// Get position in selection
		if (control)
		{
			auto&& selection = control->selectedCrew;
			for (size_t i = 0; i < selection.size(); i++)
			{
				selectionIndices[selection[i]] = i;
			}
		}
	}
	else
	{
		for (size_t i = 0; i < list.size(); i++)
		{
			if (list[i]->bDead && !list[i]->clone_ready) // permanently dead
				continue;

			if (list[i]->iShipId == player ? 1 : 0) // wrong owner
				continue;

			bool drone = list[i]->crewAnim->bDrone;

			if (!drone)
				ptrs.push_back(list[i]);
		}
	}

	for (size_t i = 0; i < ptrs.size(); i++)
	{
		Point<int> offset = ptrs[i]->currentShipId == 0 // on player ship
			? playerShipPos
			: enemyShipPos;

		readCrew(crew.emplace_back(), *ptrs[i], offset);
		if constexpr (player)
		{
			crew.back().id = int(i);
			auto it = selectionIndices.find(ptrs[i]);
			crew.back().selectionId = it != selectionIndices.end() ? it->second : -1;
		}

		if (arriving && leaving)
		{
			readCrewTeleportInfo(
				crew.back(),
				ptrs[i],
				*arriving, *leaving
			);
		}
	}
}

void readProjectile(
	Projectile& projectile,
	const raw::Projectile& raw,
	const Point<int>& playerShipPos,
	const Point<int>& enemyShipPos)
{
	projectile.type = ProjectileType(raw.getType());
	projectile.position = raw.position;
	projectile.positionLast = raw.last_position;
	projectile.target = raw.target;
	projectile.speed = raw.speed;
	projectile.lifespan = raw.lifespan;
	projectile.heading = raw.heading;
	projectile.entryAngle = raw.entryAngle;
	projectile.angle = 0.f;
	projectile.spinSpeed = 0.f;
	projectile.player = raw.ownerId == 0;
	projectile.playerSpace = raw.currentSpace == 0;
	projectile.playerSpaceIsDestination = raw.destinationSpace == 0;
	projectile.dying = raw.dead || raw.startedDeath;
	projectile.missed = raw.missed;
	projectile.hit = raw.hitTarget;
	projectile.passed = raw.passedTarget;

	if (projectile.playerSpace)
	{
		projectile.position += playerShipPos;
		projectile.positionLast += playerShipPos;
	}
	else
	{
		projectile.position += enemyShipPos;
		projectile.positionLast += enemyShipPos;
	}

	if (projectile.playerSpaceIsDestination)
	{
		projectile.target += playerShipPos;
	}
	else
	{
		projectile.target += enemyShipPos;
	}

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

void readSpace(
	Space& space,
	const raw::SpaceManager& raw,
	const Point<int>& playerShipPos,
	const Point<int>& enemyShipPos)
{
	// Projectile are hard
	space.projectiles.clear();

	for (size_t i = 0; i < raw.projectiles.size(); i++)
	{
		readProjectile(
			space.projectiles.emplace_back(),
			*raw.projectiles[i],
			playerShipPos,
			enemyShipPos);
	}

	// Hazard stuffs
	if (raw.sunLevel) space.environment = EnvironmentType::CloseToSun;
	if (raw.pulsarLevel) space.environment = EnvironmentType::Pulsar;
	if (raw.bPDS) space.environment = EnvironmentType::ASB;
	if (raw.bNebula) space.environment = EnvironmentType::Nebula;
	if (raw.bStorm) space.environment = EnvironmentType::IonStorm;
	space.environmentTargetingEnemy = raw.envTarget == 1;
	space.hazardTimer = { raw.flashTracker.current_time, raw.flashTracker.time };
	space.asteroids.reset();

	if (raw.asteroidGenerator.bRunning)
	{
		space.environment = EnvironmentType::Asteroids;

		auto&& gen = raw.asteroidGenerator;
		space.asteroids.emplace();
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
	store.confirming = nullptr;

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

		if (&rawBox == raw.confirmBuy) store.confirming = &box;

		switch (box.type)
		{
		case StoreBoxType::Weapon:
		{
			WeaponBlueprint item;
			readWeaponBlueprint(item, static_cast<const raw::WeaponBlueprint&>(*rawBox.pBlueprint));
			box.item = item;
			break;
		}
		case StoreBoxType::Drone:
		{
			DroneBlueprint item;
			readDroneBlueprint(item, static_cast<const raw::DroneBlueprint&>(*rawBox.pBlueprint));
			box.item = item;
			break;
		}
		case StoreBoxType::Augment:
		{
			Augment item;
			readAugment(item, static_cast<const raw::AugmentBlueprint&>(*rawBox.pBlueprint));
			box.item = item;
			break;
		}
		case StoreBoxType::Crew:
		{
			CrewBlueprint item;
			readCrewBlueprint(item, static_cast<const raw::CrewBlueprint&>(*rawBox.pBlueprint));
			box.item = item;
			break;
		}
		case StoreBoxType::System:
		{
			auto&& casted = static_cast<const raw::SystemStoreBox&>(rawBox);
			SystemBlueprint item;
			readSystemBlueprint(item, static_cast<const raw::SystemBlueprint&>(*rawBox.pBlueprint));
			box.item = item;

			// Check for free drone
			if (SystemType(casted.type) == SystemType::Drones)
			{
				auto&& blueprints = Reader::getState().blueprints.droneBlueprints;
				auto it = blueprints.find(casted.freeBlueprint.str);

				if (it != blueprints.end())
				{
					box.extra = it->second;
				}
			}

			break;
		}
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

	event.ship.reset();
	readResourceEvent(event.resources, raw.stuff);
	readResourceEvent(event.reward, raw.reward);
	if(!event._storePtr) event.store.reset();
	event.damage.clear();
	event.choices.clear();

	if (raw.ship.present)
	{
		event.ship.emplace();
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
		readStore(event.store.emplace(), *raw.pStore);
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
void readLocation(Location& location, const raw::Location& raw, const Point<int>& offset)
{
	location.hitbox.x = int(raw.loc.x) + offset.x - Location::HARDCODED_SIZE/2;
	location.hitbox.y = int(raw.loc.y) + offset.y - Location::HARDCODED_SIZE/2;
	location.hitbox.w = Location::HARDCODED_SIZE;
	location.hitbox.h = Location::HARDCODED_SIZE;

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
void readSector(Sector& sector, const raw::Sector& raw, const Point<int>& offset)
{
	sector.hitbox.x = int(raw.location.x) + offset.x - Sector::HARDCODED_SIZE / 2;
	sector.hitbox.y = int(raw.location.y) + offset.y - Sector::HARDCODED_SIZE / 2;
	sector.hitbox.w = Sector::HARDCODED_SIZE;
	sector.hitbox.h = Sector::HARDCODED_SIZE;

	sector.type = SectorType(raw.type);
	sector.name = raw.description.name.data.str;
	sector.visited = raw.visited;
	sector.reachable = raw.reachable;
	sector.level = raw.level;
	sector.unique = raw.description.unique;
}

void readStarMap(StarMap& map, const raw::StarMap& raw)
{
	map.lastStand = raw.bossLevel;
	map.flagshipJumping = raw.bossJumping;
	map.mapRevealed = raw.bMapRevealed;
	map.secretSector = raw.secretSector;
	map.dangerZone.center = raw.dangerZone;
	map.dangerZone.a = raw.dangerZoneRadius*2.f;
	map.dangerZone.b = raw.dangerZoneRadius*2.f;
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
		auto offset = raw.position + raw.translation;
		readLocation(map.locations.emplace_back(), *raw.locations[i], offset);
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
		readSector(map.sectors.emplace_back(), *raw.sectors[i], raw.sectorMapOffset);
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
	settings.fullscreen = FullscreenMode(raw.fullscreen);
	settings.soundVolume = raw.sound;
	settings.musicVolume = raw.music;
	settings.difficulty = Difficulty(raw.difficulty);
	settings.consoleEnabled = raw.commandConsole;
	settings.pauseOnFocusLoss = raw.altPause;
	settings.touchPause = raw.touchAutoPause;
	settings.noDynamicBackgrounds = raw.lowend;
	settings.achievementPopups = raw.achPopups;
	settings.verticalSync = raw.vsync;
	settings.frameLimit = raw.frameLimit;
	settings.showBeaconPathsOnHover = raw.showPaths;
	settings.colorblindMode = raw.colorblind;
	settings.aeEnabled = raw.bDlcEnabled;
	settings.language = raw.language.str;
	settings.screenSize = raw.screenResolution;
	settings.eventChoiceSelection = EventChoiceSelection(raw.dialogKeys);

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

void readUI(State& state, const raw::State& raw)
{
	auto&& ui = state.ui;

	if (raw.mouseControl)
	{
		auto&& mouse = *raw.mouseControl;
		ui.mouse.position = mouse.position;
		ui.mouse.positionLast = mouse.lastPosition;
		ui.mouse.dragFrom = std::nullopt;
		ui.mouse.aiming = std::monostate{};
	}

	if (state.game && state.game->playerShip)
	{
		if (!ui.game) ui.game.emplace();

		auto&& ship = *state.game->playerShip;

		auto&& gui = *raw.app->gui;

		ui.game->ftl = gui.ftlButton.hitbox;
		ui.game->shipButton = gui.upgradeButton.hitbox;
		ui.game->menuButton = gui.optionsButton.hitbox;

		auto&& crewBoxes = gui.crewControl.crewBoxes;
		ui.game->crewBoxes.clear();

		for (size_t i = 0; i < crewBoxes.size(); i++)
		{
			ui.game->crewBoxes.push_back({
				crewBoxes[i]->box,
				crewBoxes[i]->skillBox
			});
		}

		ui.game->saveStations = gui.crewControl.saveStations.hitbox;
		ui.game->loadStations = gui.crewControl.returnStations.hitbox;

		ui.game->reactor = gui.sysControl.SystemPower;
		ui.game->reactor.x += gui.sysControl.position.x;
		ui.game->reactor.y += gui.sysControl.position.y;

		ui.game->shields.reset();
		ui.game->engines.reset();
		ui.game->oxygen.reset();
		ui.game->weapons.reset();
		ui.game->drones.reset();
		ui.game->medbay.reset();
		ui.game->piloting.reset();
		ui.game->sensors.reset();
		ui.game->doorControl.reset();
		ui.game->teleporter.reset();
		ui.game->cloaking.reset();
		ui.game->artillery.clear();
		ui.game->battery.reset();
		ui.game->clonebay.reset();
		ui.game->mindControl.reset();
		ui.game->hacking.reset();

		ui.game->weaponBoxes.clear();
		ui.game->autofire.reset();
		ui.game->droneBoxes.clear();
		ui.game->openAllDoors.reset();
		ui.game->closeAllDoors.reset();
		ui.game->teleportSend.reset();
		ui.game->teleportReturn.reset();
		ui.game->startCloak.reset();
		ui.game->startBattery.reset();
		ui.game->startMindControl.reset();
		ui.game->startHack.reset();

		auto&& sysBoxes = gui.sysControl.sysBoxes;
		auto&& sysPos = gui.sysControl.position;

		for (size_t i = 0; i < sysBoxes.size(); i++)
		{
			auto&& box = *sysBoxes[i];
			auto&& sys = box.pSystem;
			auto type = SystemType(sys->iSystemType);

			switch (type)
			{
			case SystemType::Shields:
				ui.game->shields = box.hitBox + sysPos;
				break;
			case SystemType::Engines:
				ui.game->engines = box.hitBox + sysPos;
				break;
			case SystemType::Oxygen:
				ui.game->oxygen = box.hitBox + sysPos;
				break;
			case SystemType::Weapons:
			{
				ui.game->weapons = box.hitBox + sysPos;

				auto&& weapControl = gui.combatControl.weapControl;
				for (size_t i = 0; i < weapControl.boxes.size(); i++)
				{
					auto&& box = *weapControl.boxes[i];
					ui.game->weaponBoxes.emplace_back(
						weapControl.location + box.location,
						GameUIState::HARDCODED_ARMAMENT_BOX_SIZE);
				}

				ui.game->autofire = weapControl.autoFireButton.hitbox + weapControl.location;

				break;
			}
			case SystemType::Drones:
			{
				ui.game->drones = box.hitBox + sysPos;

				auto&& droneControl = gui.combatControl.droneControl;
				for (size_t i = 0; i < droneControl.boxes.size(); i++)
				{
					auto&& box = *droneControl.boxes[i];
					ui.game->droneBoxes.emplace_back(
						droneControl.location + box.location,
						GameUIState::HARDCODED_ARMAMENT_BOX_SIZE);
				}

				break;
			}
			case SystemType::Medbay:
				ui.game->medbay = box.hitBox + sysPos;
				break;
			case SystemType::Piloting:
				ui.game->piloting = box.hitBox + sysPos;
				break;
			case SystemType::Sensors:
				ui.game->sensors = box.hitBox + sysPos;
				break;
			case SystemType::Doors:
			{
				auto&& doorBox = static_cast<raw::DoorBox&>(box);
				ui.game->doorControl = box.hitBox + sysPos;
				ui.game->openAllDoors = doorBox.openDoors.hitbox + sysPos + doorBox.buttonOffset;
				ui.game->closeAllDoors = doorBox.closeDoors.hitbox + sysPos + doorBox.buttonOffset;
				break;
			}
			case SystemType::Teleporter:
			{
				auto&& teleBox = static_cast<raw::TeleportBox&>(box);
				ui.game->teleporter.emplace(box.hitBox + sysPos);
				ui.game->teleportSend = teleBox.teleportLeave.hitbox + sysPos + teleBox.buttonOffset;
				ui.game->teleportReturn = teleBox.teleportArrive.hitbox + sysPos + teleBox.buttonOffset;
				break;
			}
			case SystemType::Cloaking:
			{
				auto&& cloakBox = static_cast<raw::CloakingBox&>(box);
				ui.game->cloaking = box.hitBox + sysPos;
				ui.game->startCloak = cloakBox.currentButton->hitbox + sysPos + cloakBox.buttonOffset;
				break;
			}
			case SystemType::Artillery:
				ui.game->artillery.emplace_back(box.hitBox + sysPos);
				break;
			case SystemType::Battery:
			{
				auto&& batteryBox = static_cast<raw::BatteryBox&>(box);
				ui.game->battery = box.hitBox + sysPos;
				ui.game->startBattery = batteryBox.batteryButton.hitbox + sysPos + batteryBox.buttonOffset;
				break;
			}
			case SystemType::Clonebay:
				ui.game->clonebay = box.hitBox + sysPos;
				break;
			case SystemType::MindControl:
			{
				auto&& mindBox = static_cast<raw::MindBox&>(box);
				ui.game->mindControl = box.hitBox + sysPos;
				ui.game->startMindControl = mindBox.mindControl.hitbox + sysPos + mindBox.buttonOffset;
				break;
			}
			case SystemType::Hacking:
			{
				auto&& hackBox = static_cast<raw::HackBox&>(box);
				ui.game->hacking = box.hitBox + sysPos;
				ui.game->startHack = hackBox.hackButton.hitbox + sysPos + hackBox.buttonOffset;
				break;
			}
			}
		}

		// Misc menus
		if (raw.app->gui->shipScreens.bOpen)
		{
			auto&& screens = raw.app->gui->shipScreens;

			ui.game->upgradesTab = screens.buttons[0]->hitbox;
			ui.game->crewTab = screens.buttons[1]->hitbox;
			ui.game->cargoTab = screens.buttons[2]->hitbox;
		}
		else
		{
			ui.game->upgradesTab.reset();
			ui.game->crewTab.reset();
			ui.game->cargoTab.reset();
		}

		if (raw.app->gui->upgradeScreen.bOpen)
		{
			auto&& upgrades = raw.app->gui->upgradeScreen;

			ui.game->upgrades.emplace();

			for (size_t i = 0; i < upgrades.vUpgradeBoxes.size(); i++)
			{
				auto&& box = *upgrades.vUpgradeBoxes[i];
				auto type = SystemType(box.system->iSystemType);
				auto&& sys = Reader::getState().game->playerShip->getSystem(type);

				SystemUpgradeUIState s{box.boxButton.hitbox, sys.level.first + box.tempUpgrade};

				switch (type)
				{
				case SystemType::Shields: ui.game->upgrades->shields = s; break;
				case SystemType::Engines: ui.game->upgrades->engines = s; break;
				case SystemType::Oxygen: ui.game->upgrades->oxygen = s; break;
				case SystemType::Weapons: ui.game->upgrades->weapons = s; break;
				case SystemType::Drones: ui.game->upgrades->drones = s; break;
				case SystemType::Medbay: ui.game->upgrades->medbay = s; break;
				case SystemType::Piloting: ui.game->upgrades->piloting = s; break;
				case SystemType::Sensors: ui.game->upgrades->sensors = s; break;
				case SystemType::Doors: ui.game->upgrades->doorControl = s; break;
				case SystemType::Teleporter: ui.game->upgrades->teleporter = s; break;
				case SystemType::Cloaking: ui.game->upgrades->cloaking = s; break;
				case SystemType::Artillery: ui.game->upgrades->artillery.push_back(s); break;
				case SystemType::Battery: ui.game->upgrades->battery = s; break;
				case SystemType::Clonebay: ui.game->upgrades->clonebay = s; break;
				case SystemType::MindControl: ui.game->upgrades->mindControl = s; break;
				case SystemType::Hacking: ui.game->upgrades->hacking = s; break;
				}
			}

			ui.game->upgrades->reactor.box = upgrades.reactorButton.hitbox;
			ui.game->upgrades->reactor.upgradeTo = upgrades.reactorButton.tempUpgrade;

			ui.game->upgrades->undo = upgrades.undoButton.hitbox;
			ui.game->upgrades->accept = raw.app->gui->shipScreens.doneButton.hitbox;
		}
		else
		{
			ui.game->upgrades.reset();
		}

		if (raw.app->gui->crewScreen.bOpen)
		{
			auto&& crew = raw.app->gui->crewScreen;

			ui.game->crewMenu.emplace();

			for (size_t i = 0; i < crew.crewBoxes.size(); i++)
			{
				auto&& box = crew.crewBoxes[i];

				ui.game->crewMenu->boxes.push_back({
					.box = box->hitBox,
					.rename = box->renameButton.hitbox,
					.dismiss = box->deleteButton.hitbox
				});
			}

			if (crew.confirmingDelete)
			{
				ui.game->crewMenu->confirm.emplace();
				ui.game->crewMenu->confirm->yes = crew.deleteDialog.yesButton.hitbox;
				ui.game->crewMenu->confirm->no = crew.deleteDialog.noButton.hitbox;
			}

			ui.game->crewMenu->accept = raw.app->gui->shipScreens.doneButton.hitbox;
		}
		else
		{
			ui.game->crewMenu.reset();
		}

		if (raw.app->gui->leaveCrewDialog.bOpen)
		{
			auto&& dialog = raw.app->gui->leaveCrewDialog;

			ui.game->leaveCrew.emplace();
			ui.game->leaveCrew->yes = dialog.yesButton.hitbox;
			ui.game->leaveCrew->no = dialog.noButton.hitbox;
		}
		else
		{
			ui.game->leaveCrew.reset();
		}

		if (raw.app->gui->equipScreen.bOpen)
		{
			auto&& cargo = raw.app->gui->equipScreen;

			ui.game->cargo.emplace();

			auto&& boxes = cargo.vEquipmentBoxes;
			const size_t weaponSlots = cargo.shipManager->myBlueprint.weaponSlots;
			const size_t droneSlots = cargo.shipManager->myBlueprint.droneSlots;
			const size_t weaponStart = 0;
			const size_t droneStart = weaponStart + weaponSlots;
			const size_t augStart = droneStart + droneSlots;
			const size_t storageStart = augStart + 3;
			const size_t overCapacity = storageStart + 4;

			for (size_t i = weaponStart; i < droneStart; i++)
				ui.game->cargo->weapons.push_back(boxes[i]->hitBox);

			for (size_t i = droneStart; i < augStart; i++)
				ui.game->cargo->drones.push_back(boxes[i]->hitBox);

			for (size_t i = augStart; i < storageStart; i++)
				ui.game->cargo->augments.push_back(boxes[i]->hitBox);

			for (size_t i = storageStart; i < overCapacity; i++)
				ui.game->cargo->storage.push_back(boxes[i]->hitBox);

			if (cargo.bStoreMode)
			{
				auto&& store = raw.app->gui->storeScreens;
				ui.game->cargo->discard.emplace();
				ui.game->cargo->discard->x = cargo.sellBox.position.x;
				ui.game->cargo->discard->y = cargo.sellBox.position.y;
				ui.game->cargo->discard->w = cargo.sellBox.boxImage[0]->width_;
				ui.game->cargo->discard->h =
					cargo.sellBox.boxImage[0]->height_ +
					cargo.sellBox.titleInsert +
					cargo.sellBox.insertHeight;
				
				ui.game->cargo->accept = store.doneButton.hitbox;
			}
			else
			{
				if (cargo.bOverCapacity)
				{
					ui.game->cargo->discard = cargo.overcapacityBox->hitBox;
				}
				else if (cargo.bOverAugCapacity)
				{
					ui.game->cargo->discard = cargo.overAugBox->hitBox;
				}

				ui.game->cargo->accept = raw.app->gui->shipScreens.doneButton.hitbox;
			}

		}
		else
		{
			ui.game->cargo.reset();
		}

		if (raw.app->gui->starMap->bOpen)
		{
			auto&& map = raw.app->gui->starMap;

			ui.game->starMap.emplace();

			ui.game->starMap->back = map->bChoosingNewSector
				? map->closeButton.hitbox
				: map->closeSectorButton.hitbox;
			ui.game->starMap->back += map->position;

			if (map->waitButton.bActive)
			{
				ui.game->starMap->wait = map->waitButton.hitbox;
				*ui.game->starMap->wait += map->position;
			}
			else ui.game->starMap->wait.reset();

			if (map->distressButton.bActive)
			{
				ui.game->starMap->distress = map->distressButton.hitbox;
				*ui.game->starMap->distress += map->position;
			}
			else ui.game->starMap->distress.reset();

			if (map->endButton.bActive)
			{
				ui.game->starMap->nextSector = map->endButton.hitbox;
				*ui.game->starMap->nextSector += map->position;
			}
			else ui.game->starMap->nextSector.reset();
		}
		else
		{
			ui.game->starMap.reset();
		}

		if (state.game->event && state.game->pause.event)
		{
			auto&& choiceBox = raw.app->gui->choiceBox;
			auto&& mainText = choiceBox.mainText;
			auto&& text = choiceBox.choices;
			auto&& boxes = choiceBox.choiceBoxes;
			auto&& openTime = choiceBox.openTime;

			ui.game->event.emplace();
			ui.game->event->text = mainText.str;

			for (size_t i = 0; i < text.size(); i++)
			{
				auto&& choice = ui.game->event->choices.emplace_back();
				choice.text = text[i].text.str;
				if(i < boxes.size()) choice.box = boxes[i];
			}

			switch (state.settings.eventChoiceSelection)
			{
			case EventChoiceSelection::NoDelay:
				ui.game->event->openTime = { 0.f, 0.f };
				break;
			case EventChoiceSelection::BriefDelay:
				ui.game->event->openTime = {
					EventUIState::HARDCODED_BRIEF_DELAY_TIME-openTime,
					EventUIState::HARDCODED_BRIEF_DELAY_TIME
				};
				break;
			default: // no hotkeys
				ui.game->event->openTime = {
					std::numeric_limits<float>::infinity(),
					std::numeric_limits<float>::infinity()
				};
				break;
			}

			if (state.game->event->store)
			{
				auto&& store = state.game->event->store;
				auto&& rawStore = *state.game->event->_storePtr;
				auto&& screen = raw.app->gui->storeScreens;

				ui.game->store.emplace();
				ui.game->store->close = screen.doneButton.hitbox;
				ui.game->store->buy = screen.buttons[0]->hitbox;
				ui.game->store->sell = screen.buttons[1]->hitbox;
				
				int pageCount = int(store->sections.size() / 2);

				// We only support 2 pages right now
				// I am generalizing this for future Hyperspace support (hopefully)
				// And yes, the buttons in FTL are reversed for some reason
				if (pageCount >= 1) ui.game->store->pages.push_back(rawStore.page2.hitbox);
				if (pageCount >= 2) ui.game->store->pages.push_back(rawStore.page1.hitbox);
				ui.game->store->currentPage = rawStore.bShowPage2 ? 1 : 0;

				int items = rawStore.sectionCount * Store::HARDCODED_BOXES_PER_SECTION;
				ui.game->store->fuel = rawStore.vItemBoxes[items + 0]->button.hitbox;
				ui.game->store->missiles = rawStore.vItemBoxes[items + 1]->button.hitbox;
				ui.game->store->droneParts = rawStore.vItemBoxes[items + 2]->button.hitbox;
				ui.game->store->repair = rawStore.vItemBoxes[items + 3]->button.hitbox;
				ui.game->store->repairAll = rawStore.vItemBoxes[items + 4]->button.hitbox;

				for (int i = 0; i < items; i++)
				{
					ui.game->store->boxes.push_back(rawStore.vItemBoxes[i]->button.hitbox);
				}

				if (store->confirming)
				{
					ui.game->store->confirm.emplace();
					ui.game->store->confirm->yes = rawStore.confirmDialog.yesButton.hitbox;
					ui.game->store->confirm->no = rawStore.confirmDialog.noButton.hitbox;
				}

				ui.game->storeButton = raw.app->gui->storeButton.hitbox;
			}
		}
		else
		{
			ui.game->event.reset();
			ui.game->store.reset();
			ui.game->storeButton.reset();
		}

		if (raw.app->gui->gameOverScreen.bOpen)
		{
			auto&& screen = raw.app->gui->gameOverScreen;
			ui.game->gameOver.emplace();
			ui.game->gameOver->stats = screen.buttons[0]->hitbox;
			ui.game->gameOver->restart = screen.buttons[1]->hitbox;
			ui.game->gameOver->hangar = screen.buttons[2]->hitbox;
			ui.game->gameOver->mainMenu = screen.buttons[3]->hitbox;
			ui.game->gameOver->quit = screen.buttons[4]->hitbox;
		}
		else
		{
			ui.game->gameOver.reset();
		}

		// Pause menu
		if (raw.app->gui->menuBox.bOpen)
		{
			auto&& menu = raw.app->gui->menuBox;
			ui.game->menu.emplace();
			ui.game->menu->continueButton = menu.buttons[0]->hitbox;
			ui.game->menu->mainMenu = menu.buttons[1]->hitbox;
			ui.game->menu->hangar = menu.buttons[2]->hitbox;
			ui.game->menu->restart = menu.buttons[3]->hitbox;
			ui.game->menu->options = menu.buttons[4]->hitbox;
			ui.game->menu->controls = menu.buttons[5]->hitbox;
			ui.game->menu->quit = menu.buttons[6]->hitbox;

			Point<int> offset = menu.statusPosition;

			ui.game->menu->difficulty.x = offset.x;
			ui.game->menu->difficulty.y = offset.y;
			ui.game->menu->difficulty.w = menu.difficultyBox->width_;
			ui.game->menu->difficulty.h = menu.difficultyBox->height_;

			ui.game->menu->aeEnabled.x = ui.game->menu->difficulty.w + offset.x;
			ui.game->menu->aeEnabled.y = offset.y;
			ui.game->menu->aeEnabled.w = menu.dlcBox->width_;
			ui.game->menu->aeEnabled.h = menu.dlcBox->height_;

			for (size_t i = 0; i < menu.shipAchievements.size(); i++)
			{
				auto&& rawAch = menu.shipAchievements[i];
				ui.game->menu->achievements.emplace_back(
					rawAch.position,
					Point<int>{rawAch.dimension, rawAch.dimension}
				);
			}

			ui.game->menu->showControls = menu.bShowControls;
		}
		else
		{
			ui.game->menu.reset();
		}

		// Set in-game cursor info
		if (raw.mouseControl)
		{
			auto&& mouse = *raw.mouseControl;

			ui.mouse.autofire.reset();

			// What is currently being aimed
			if (mouse.aiming_required > 0 && ship.weapons)
			{
				int mod = ship.weapons->autoFire ? 4 : 0;
				ui.mouse.autofire = ship.weapons->autoFire;
				ui.mouse.aiming = std::cref(ship.weapons->list[mouse.aiming_required-1+mod]);
			}
			else if (mouse.iTeleporting != 0 && ship.teleporter)
			{
				ui.mouse.aiming = std::cref(*ship.teleporter);
			}
			else if (mouse.iMindControlling != 0 && ship.mindControl)
			{
				ui.mouse.aiming = std::cref(*ship.mindControl);
			}
			else if (mouse.iHacking != 0 && ship.hacking)
			{
				ui.mouse.aiming = std::cref(*ship.hacking);
			}

			auto&& crew = raw.app->gui->crewControl;

			if (crew.mouseDown)
			{
				ui.mouse.dragFrom = crew.firstMouse;
			}
		}
	}
	else
	{
		ui.game.reset();
	}
}

}

bool Reader::init()
{
	start = Clock::now();

	base = reinterpret_cast<uintptr_t>(GetModuleHandle(L"FTLGame.exe"));;

	memcpy_s(
		&rs.app, sizeof(raw::CApp*),
		&mem::get<raw::CApp*>(base + raw::CAppPtr), sizeof(raw::CApp*));

	if (!rs.app) return false;

	rs.crewMemberFactory = &mem::get<raw::CrewMemberFactory>(base + raw::CrewMemberFactoryPtr);
	rs.settingValues = &mem::get<raw::SettingValues>(base + raw::SettingValuesPtr);
	rs.blueprints = &mem::get<raw::BlueprintManager>(base + raw::BlueprintManagerPtr);
	rs.powerManagerContainer = &mem::get<raw::PowerManagerContainer>(base + raw::PowerManagerContainerPtr);
	rs.mouseControl = &mem::get<raw::MouseControl>(base + raw::MouseControlPtr);

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

	poll();

	started = true;
	return true;
}

void Reader::read()
{
	state.running = rs.app && rs.app->Running;

	if (!state.running) return;

	if (Input::ready())
	{
		rs.app->focus = true;
		rs.app->inputFocus = true;
	}

	readSettings(state.settings, *rs.settingValues);

	if (state.game)
	{
		auto&& game = *state.game;

		bool prevPause = game.pause.any;

		game.pause.normal = rs.app->gui->bPaused;
		game.pause.automatic = rs.app->gui->bAutoPaused;
		game.pause.menu = rs.app->gui->menu_pause;
		game.pause.event = rs.app->gui->choiceBoxOpen; // more accurate for gauging if the event window's open

		game.pause.any =
			game.pause.normal || game.pause.automatic ||
			game.pause.menu || game.pause.event;

		game.pause.justPaused = !prevPause && game.pause.any;
		game.pause.justUnpaused = prevPause && !game.pause.any;

		game.gameOver = rs.app->gui->gameover;

		Point<int> playerShipPos, enemyShipPos;
		{
			auto& playerPos = rs.app->gui->combatControl.playerShipPosition;
			playerShipPos = playerPos;

			Point<int> base = rs.app->gui->combatControl.position;
			Point<int> offset = rs.app->gui->combatControl.targetPosition;
			enemyShipPos = base + offset;

			if (state.ui.game)
			{
				state.ui.game->playerShip = playerShipPos;
				state.ui.game->enemyShip = enemyShipPos;
			}
		}

		// Space stuffs
		readSpace(game.space, rs.app->world->space, playerShipPos, enemyShipPos);

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

			readCrewList(
				game.playerCrew,
				rs.app->gui->crewControl.crewBoxes,
				playerShipPos, enemyShipPos,
				playerArriving, playerLeaving,
				&rs.app->gui->crewControl);

			readCrewList(
				game.enemyCrew,
				rs.crewMemberFactory->crewMembers,
				enemyShipPos, enemyShipPos,
				enemyArriving, enemyLeaving);
		}

		auto& shipStatus = rs.app->gui->shipStatus;

		if (shipStatus.ship)
		{
			if (!game.playerShip) game.playerShip.emplace();

			bool prevJumping = game.playerShip->jumping;

			readPlayerShip(
				*game.playerShip,
				game.playerCrew,
				game.enemyCrew,
				*rs.app->gui,
				rs.powerManagerContainer->powerManagers[0],
				playerShipPos, enemyShipPos
			);

			game.justJumped = prevJumping && !game.playerShip->jumping;
		}
		else
		{
			game.playerShip.reset();
			game.justJumped = false;
		}

		// For accessing the enemy's CompleteShip instance
		auto& completePlayerShip = rs.app->world->playerShip;

		if (completePlayerShip)
		{
			auto* enemy = completePlayerShip->enemyShip;

			if (enemy)
			{
				if (!game.enemyShip) game.enemyShip.emplace();

				readEnemyShip(
					*game.enemyShip,
					game.enemyCrew,
					game.playerCrew,
					*enemy,
					rs.powerManagerContainer->powerManagers[1],
					playerShipPos, enemyShipPos
				);
			}
			else
			{
				game.enemyShip.reset();
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

				auto next = size_t(choices[i]);

				if (next >= current->choices.size())
				{
					current = nullptr;
					break;
				}

				if (current->choices[next].event)
				{
					current = current->choices[next].event;
				}
			}

			if (current)
			{
				// Only null store pointer when jumping
				bool preserveStore = game.event && game.playerShip && !game.playerShip->jumping;
				if (!preserveStore) game.event.emplace();
				readLocationEvent(*game.event, *current, preserveStore);
			}
			else game.event.reset();
		}

		readStarMap(game.starMap, rs.app->world->starMap);

		if (game.justLoaded) game.justLoaded = false;
	}

	readUI(state, rs);

	if (rs.app->menu.bOpen) // not in game
	{
		state.game.reset();
	}
	else if (!state.game) // create game object if it doesn't exist
	{
		state.game.emplace();
		state.game->pause.justUnpaused = true;
		state.game->justLoaded = true;
	}
}

void Reader::iterate()
{
	Reader::read();
	if (Input::ready()) Input::iterate();
}

std::mutex readerMutex;
std::condition_variable readerCV;

void Reader::poll()
{
	iterate();
}

namespace
{

using DoubleDuration = std::chrono::duration<double, std::ratio<1>>;

Reader::Duration fromDouble(double d)
{
	return std::chrono::duration_cast<Reader::Duration>(DoubleDuration{ d });
}

double toDouble(Reader::Duration d)
{
	return std::chrono::duration_cast<DoubleDuration>(d).count();
}

}

double Reader::now()
{
	if (!started) return 0.0;
	return toDouble(Clock::now() - start);
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

uintptr_t Reader::getRealAddress(uintptr_t offset, MutableRawState)
{
	return base + offset;
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
