#include "Reader.hpp"
#include "../Memory.hpp"

// This class handles the actual reading of the structure
class StateReader::Impl
{
public:
	Impl(StateReader& reader)
		: addresses(reader.addresses)
		, state(reader.state)
		, rawState(reader.rawState)
		, blueprints(reader.blueprints)
	{
		this->ptrSize = this->addr("ptr");
		this->base = reinterpret_cast<uintptr_t>(GetModuleHandle(L"FTLGame.exe"));
	}

	void poll()
	{
		static bool wasJumping = false;

		memcpy_s(
			&this->rawState.app, sizeof(raw::CApp*),
			&mem::get<raw::CApp*>(base + raw::CAppBase), sizeof(raw::CApp*));

		this->checkInGame(this->base);

		if (this->state.inGame)
		{
			this->readMisc(this->base);
			this->readGui(this->base);

			auto& jumping = this->state.player.jumping;
			wasJumping = jumping;
			this->readWorld(this->base);
			this->state.justLoaded = !jumping && wasJumping;
		}
	}

private:
	const AddressData& addresses;
	State& state;
	raw::State& rawState;
	KnownBlueprints& blueprints;
	uintptr_t ptrSize = 0;
	uintptr_t base = 0;

	uintptr_t addr(const char* str)
	{
		return this->addresses.get(this->addresses.hash(str), str);
	}

	template<typename T>
	void read(T& object, uintptr_t base, uintptr_t offset = 0)
	{
		memcpy_s(&object, sizeof(T), &mem::get<T>(base + offset), sizeof(T));
	}

	template<typename T>
	void read(T& object, uintptr_t base, const char* offset)
	{
		memcpy_s(&object, sizeof(T), &mem::get<T>(base + this->addr(offset)), sizeof(T));
	}

	// Specific reading functions
	void checkInGame(uintptr_t base)
	{
		constexpr char ptrs[][8]{ "ingame1", "ingame2", "ingame3" };
		bool previouslyInGame = this->state.inGame;

		for (int i = 0; i < 3; ++i)
		{
			this->read(base, base, ptrs[i]);
			this->state.inGame = base != 0;

			if (!this->state.inGame)
				break;
		}

		if (this->state.inGame && !previouslyInGame)
			this->state.justLoaded = true;
	}

	void readMisc(uintptr_t base)
	{
		this->read(this->state.minimized, base, "minimized");
		this->read(this->state.focused, base, "focused");
	}

	void readGui(uintptr_t base)
	{
		uintptr_t ptr = 0;
		this->read(ptr, base, "gui");

		auto& p = this->state.pause;
		this->read(p.normal, ptr, "pause.normal");
		this->read(p.automatic, ptr, "pause.auto");
		this->read(p.menu, ptr, "pause.menu");
		this->read(p.event, ptr, "pause.event");
		this->read(p.touch, ptr, "pause.touch");
		p.any = p.normal || p.automatic || p.menu || p.event || p.touch;

		if (p.any)
			return;

		this->read(ptr, ptr, "gui.world.ptr");
		this->read(this->state.shipPosition.x, ptr, "world.ship.pos.x");
		this->read(this->state.shipPosition.y, ptr, "world.ship.pos.y");

		if (this->state.targetPresent)
		{
			Pointi basePos{ 0, 0 };

			this->read(ptr, base, "world.target.ship.pos.ptr");
			this->read(basePos.x, ptr, "world.target.ship.pos.base.x");
			this->read(basePos.y, ptr, "world.target.ship.pos.base.y");
			this->read(this->state.targetShipPosition.x, ptr, "world.target.ship.pos.x");
			this->read(this->state.targetShipPosition.y, ptr, "world.target.ship.pos.y");
			this->state.targetShipPosition += basePos;
		}
	}

	void readWorld(uintptr_t base)
	{
		if (!this->state.justLoaded && this->state.pause.any)
			return;

		uintptr_t ptr = 0;
		this->read(ptr, base, "gui");
		this->read(ptr, ptr, "gui.world.ptr"); // yup, the world pointer is obtained through the gui
		this->readHazards(this->state.hazards, ptr);
		this->readProjectiles(this->state.projectiles, ptr);
		this->readSpaceDrones(this->state.spaceDrones, ptr);

		this->readCrew(base);

		this->read(ptr, base, "ship.reactor.ptr");
		this->read(this->state.player.reactor.first, ptr, "ship.reactor.used");
		this->read(this->state.player.reactor.second, ptr, "ship.reactor.level");
		this->read(this->state.player.reactorLimit, ptr, "ship.reactor.limit");

		this->read(base, base, "player.ship");
		this->readShip(this->state.player, base);

		this->read(base, base, "target.ship");
		this->state.targetPresent = base != 0;

		if(this->state.targetPresent)
			this->readShip(this->state.target, base);
	}

	void readShip(ShipState& ship, uintptr_t base)
	{
		this->readSystems(ship.systems, base);
		this->readRooms(ship.rooms, base);
		this->readDoors(ship.doors, base);
		this->read(ship.hull.first, base, "ship.hull");
		this->read(ship.hull.second, base, "ship.hull.max");
		this->read(ship.ftl.first, base, "ship.ftl");
		this->read(ship.ftl.second, base, "ship.ftl.max");

		this->read(ship.fuel, base, "ship.fuel");
		this->read(ship.droneParts, base, "ship.drone_parts");
		this->read(ship.scrap, base, "ship.scrap");

		this->read(ship.jumping, base, "ship.jumping");

		uintptr_t ptr = 0;
		this->read(ptr, base, "ship.missiles.ptr");
		if(ptr != 0)
			this->read(ship.missiles, ptr, "ship.missiles.ptr2");

		this->read(ptr, base, "ship.shields.ptr");
		if (ptr != 0)
		{
			this->read(ship.superShield.first, ptr, "ship.shields.super");
			this->read(ship.superShield.second, ptr, "ship.shields.super.max");
		}

		// Read system ids into rooms
		if (this->state.justLoaded)
		{
			for (auto sys : SystemOrder)
			{
				if (sys == SystemID::Artillery)
				{
					for (auto& art : ship.systems.artillery)
						ship.rooms.at(art.room).system = sys;
				}
				else
				{
					auto* s = ship.systems.get(sys);
					if (!s) continue;

					ship.rooms.at(s->room).system = sys;
				}
			}
		}
	}

	void readGenericSystem(SystemState& system, uintptr_t base)
	{
		system.present = true;

		this->read(system.room, base, "system.room");

		auto& [total, reactor, zoltan, battery, limit] = system.power;
		this->read(reactor, base, "system.power.reactor");
		this->read(zoltan, base, "system.power.zoltan");
		this->read(battery, base, "system.power.battery");
		this->read(limit, base, "system.power.limit");

		// Though the game does store and compute the total
		// it seems to be delayed
		total = reactor + zoltan + battery;
		
		this->read(system.level.first, base, "system.level.current");
		this->read(system.level.second, base, "system.level.max");
		this->read(system.ion.first, base, "system.power.ion.minor");
		this->read(system.ion.second, base, "system.power.ion.minor");

		this->read(system.hackLevel, base, "system.hacked");

		this->read(system.health, base, "system.health");
		this->read(system.partialHP.first, base, "system.health.repair");
		this->read(system.partialHP.second, base, "system.health.attack");

		this->read(system.manningLevel, base, "system.manning.level");
	}

	void readShields(ShieldsState& shields, uintptr_t base)
	{
		this->readGenericSystem(shields, base);
		this->read(shields.bubbles, base, "shields.bubbles");
		this->read(shields.charge, base, "shields.charge");
		this->read(shields.center.x, base, "shields.ellipse.x");
		this->read(shields.center.y, base, "shields.ellipse.y");
		this->read(shields.ellipse.first, base, "shields.ellipse.a");
		this->read(shields.ellipse.second, base, "shields.ellipse.b");
	}

	void readEngines(EnginesState& engines, uintptr_t base)
	{
		this->readGenericSystem(engines, base);
		this->read(engines.ftlBoost, base, "engines.ftlboost");
	}

	void readOxygen(OxygenState& oxygen, uintptr_t base)
	{
		this->readGenericSystem(oxygen, base);
		this->read(oxygen.max, base, "oxygen.max");
		this->read(oxygen.total, base, "oxygen.total");
		this->read(oxygen.leaking, base, "oxygen.leaking");

		uintptr_t start = 0, end = 0;
		this->read(start, base, "oxygen.levels.start");
		this->read(end, base, "oxygen.levels.end");

		size_t size = (end - start) / this->ptrSize;
		if (oxygen.levels.size() != size)
			oxygen.levels.resize(size);

		size_t idx = 0;
		for (uintptr_t i = start; i != end; i += this->ptrSize)
		{
			this->read(oxygen.levels[idx], i);
			++idx;
		}
	}

	void readDamage(Damage& damage, uintptr_t base)
	{
		this->read(damage.main, base, "damage.main");
		this->read(damage.pierce, base, "damage.pierce");
		this->read(damage.fireChance, base, "damage.fire.chance");
		this->read(damage.breachChance, base, "damage.breach.chance");
		this->read(damage.stunChance, base, "damage.stun.chance");
		this->read(damage.ion, base, "damage.ion");
		this->read(damage.system, base, "damage.system");
		this->read(damage.crew, base, "damage.crew");
		this->read(damage.stun, base, "damage.stun");
		this->read(damage.hullBonus, base, "damage.hull_bonus");
		this->read(damage.lockdown, base, "damage.lockdown");
		this->read(damage.crystal, base, "damage.crystal");
		this->read(damage.friendlyFire, base, "damage.friendly_fire");
	}

	void readWeapon(WeaponState& weapon, uintptr_t base)
	{
		weapon.ptr = base;
		this->read(weapon.cooldown.first, base, "weapon.cooldown");
		this->read(weapon.cooldown.second, base, "weapon.cooldown.max");
		this->read(weapon.autoFire, base, "weapon.auto_fire");
		this->read(weapon.goingToFire, base, "weapon.fire");
		this->read(weapon.powered, base, "weapon.powered");
		this->read(weapon.powerRequired, base, "weapon.required_power");
		this->read(weapon.angle, base, "weapon.angle");
		this->read(weapon.cooldownModifier, base, "weapon.cooldown_modifier");
		this->read(weapon.charge.first, base, "weapon.charge");
		this->read(weapon.charge.second, base, "weapon.charge.max");

		this->read(weapon.radius, base, "weapon.radius");
		this->read(weapon.artillery, base, "weapon.is_artillery");

		// Blueprint portion
		this->read(base, base, "weapon.blueprint");
		this->read(weapon.blueprint, base, "weapon.blueprint.name");

		// If this blueprint hasn't been read before...
		if (this->blueprints.weapons.count(weapon.blueprint) == 0)
		{
			auto [it, _] = this->blueprints.weapons.emplace(weapon.blueprint, WeaponBlueprint{});
			auto& bp = it->second;

			this->read(bp.name, base, "weapon.blueprint.name");
			this->read(bp.type, base, "weapon.blueprint.type");
			this->read(bp.beamLength, base, "weapon.blueprint.beam.length");
			this->read(bp.speed, base, "weapon.blueprint.speed");
			this->read(bp.missiles, base, "weapon.blueprint.missiles");

			base += this->addr("weapon.blueprint.damage");
			this->readDamage(bp.damage, base);
		}
	}

	void readWeapons(WeaponControlState& weapons, uintptr_t base)
	{
		this->readGenericSystem(weapons, base);

		uintptr_t start = 0, end = 0;
		this->read(start, base, "weapons.list.start");
		this->read(end, base, "weapons.list.end");
		this->read(weapons.slots, base, "weapons.slots");

		size_t size = (end - start) / this->ptrSize;
		if (weapons.list.size() != size)
			weapons.list.resize(size);

		size_t idx = 0;
		for (auto i = start; i != end; i += this->ptrSize)
		{
			uintptr_t ptr = 0;
			this->read(ptr, i);

			this->readWeapon(weapons.list[idx], ptr);
			weapons.list[idx].slot = idx;
			++idx;
		}
	}

	void readDrone(DroneState& drone, uintptr_t base)
	{
		drone.ptr = base;
		this->read(drone.shipAffiliation, base, "drone.ship_affiliation");
		this->read(drone.powered, base, "drone.powered");
		this->read(drone.powerRequired, base, "drone.required_power");
		this->read(drone.deployed, base, "drone.deployed");
		this->read(drone.destroyTimer, base, "drone.destroy_timer");
		
		// Blueprint portion
		this->read(base, base, "drone.blueprint");
		this->read(drone.blueprint, base, "drone.blueprint.name");

		// If this blueprint hasn't been read before...
		if (this->blueprints.drones.count(drone.blueprint) == 0)
		{
			auto [it, _] = this->blueprints.drones.emplace(drone.blueprint, DroneBlueprint{});
			auto& bp = it->second;

			this->read(bp.name, base, "drone.blueprint.name");
			this->read(bp.type, base, "drone.blueprint.type");
		}
	}

	void readDrones(DroneControlState& drones, uintptr_t base)
	{
		this->readGenericSystem(drones, base);
		uintptr_t start = 0, end = 0;
		this->read(start, base, "drones.list.start");
		this->read(end, base, "drones.list.end");
		this->read(drones.slots, base, "drones.slots");

		size_t size = (end - start) / this->ptrSize;
		if (drones.list.size() != size)
			drones.list.resize(size);

		size_t idx = 0;
		for (auto i = start; i != end; i += this->ptrSize)
		{
			uintptr_t ptr = 0;
			this->read(ptr, i);

			this->readDrone(drones.list[idx], ptr);
			drones.list[idx].slot = idx;
			++idx;
		}
	}

	void readTeleporter(TeleporterState& teleporter, uintptr_t base)
	{
		this->readGenericSystem(teleporter, base);
		this->read(teleporter.crew.first, base, "teleporter.crew");
		this->read(teleporter.crew.second, base, "teleporter.slots");
		this->read(teleporter.canSend, base, "teleporter.can_send");
		this->read(teleporter.canReceive, base, "teleporter.can_receive");
	}

	void readCloaking(CloakingState& cloaking, uintptr_t base)
	{
		this->readGenericSystem(cloaking, base);
		this->read(cloaking.on, base, "cloaking.on");
		this->read(cloaking.timer.first, base, "cloaking.timer");
		this->read(cloaking.timer.second, base, "cloaking.timer.max");
	}

	void readArtillery(ArtilleryState& artillery, uintptr_t base)
	{
		this->readGenericSystem(artillery, base);

		uintptr_t ptr = 0;
		this->read(ptr, base, "artillery.weapon");
		this->readWeapon(artillery.weapon, ptr);
	}

	void readBattery(BatteryState& battery, uintptr_t base)
	{
		this->readGenericSystem(battery, base);
		this->read(battery.on, base, "battery.on");
		this->read(battery.timer.first, base, "battery.timer");
		this->read(battery.timer.second, base, "battery.timer.max");
	}

	void readClonebay(ClonebayState& clonebay, uintptr_t base)
	{
		this->readGenericSystem(clonebay, base);
		this->read(clonebay.timer.first, base, "clonebay.timer");
		this->read(clonebay.timer.second, base, "clonebay.timer.max");
		this->read(clonebay.deathTimer, base, "clonebay.timer.death");
	}

	void readMind(MindState& mind, uintptr_t base)
	{
		this->readGenericSystem(mind, base);
		this->read(mind.timer.first, base, "mind.timer");
		this->read(mind.timer.second, base, "mind.timer.max");
		this->read(mind.canUse, base, "mind.can_use");
		this->read(mind.armed, base, "mind.armed");
	}

	void readHacking(HackingState& hacking, uintptr_t base)
	{
		this->readGenericSystem(hacking, base);
		this->read(hacking.deployed, base, "hacking.drone.deployed");
		this->read(hacking.arrived, base, "hacking.drone.arrived");
		this->read(hacking.canUse, base, "hacking.can_use");
		this->read(hacking.timer.first, base, "hacking.timer");
		this->read(hacking.timer.second, base, "hacking.timer.max");
		this->read(hacking.start.x, base, "hacking.drone.start.x");
		this->read(hacking.start.y, base, "hacking.drone.start.y");
		this->read(hacking.destination.x, base, "hacking.drone.destination.x");
		this->read(hacking.destination.y, base, "hacking.drone.destination.y");
	}

	void resetSystemPresence(SystemStates& systems)
	{
		systems.shields.present = false;
		systems.engines.present = false;
		systems.oxygen.present = false;
		systems.weapons.present = false;
		systems.drones.present = false;
		systems.medbay.present = false;
		systems.pilot.present = false;
		systems.sensors.present = false;
		systems.doors.present = false;
		systems.teleporter.present = false;
		systems.cloaking.present = false;
		systems.battery.present = false;
		systems.clonebay.present = false;
		systems.mind.present = false;
		systems.hacking.present = false;

		systems.artillery.clear();
	}

	void readSystems(SystemStates& systems, uintptr_t base)
	{
		static uintptr_t start = 0, end = 0;

		if(this->state.justLoaded) this->resetSystemPresence(systems);

		this->read(start, base, "system.list.start");
		this->read(end, base, "system.list.end");

		// All systems, except artillery (which is skipped)
		for (auto i = start; i != end; i += this->ptrSize)
		{
			uintptr_t ptr = 0;
			this->read(ptr, i);

			SystemID id = SystemID::Invalid;
			this->read(id, ptr + this->addr("system.id"));

			switch (id)
			{
			case SystemID::Shields:
				this->readShields(systems.shields, ptr);
				break;
			case SystemID::Engines:
				this->readEngines(systems.engines, ptr);
				break;
			case SystemID::Oxygen:
				this->readOxygen(systems.oxygen, ptr);
				break;
			case SystemID::Weapons:
				this->readWeapons(systems.weapons, ptr);
				break;
			case SystemID::Drones:
				this->readDrones(systems.drones, ptr);
				break;
			case SystemID::Medbay:
				this->readGenericSystem(systems.medbay, ptr);
				break;
			case SystemID::Pilot:
				this->readGenericSystem(systems.pilot, ptr);
				break;
			case SystemID::Sensors:
				this->readGenericSystem(systems.sensors, ptr);
				break;
			case SystemID::Doors:
				this->readGenericSystem(systems.doors, ptr);
				break;
			case SystemID::Teleporter:
				this->readTeleporter(systems.teleporter, ptr);
				break;
			case SystemID::Cloaking:
				this->readCloaking(systems.cloaking, ptr);
				break;
			case SystemID::Battery:
				this->readBattery(systems.battery, ptr);
				break;
			case SystemID::Clonebay:
				this->readClonebay(systems.clonebay, ptr);
				break;
			case SystemID::Mind:
				this->readMind(systems.mind, ptr);
				break;
			case SystemID::Hacking:
				this->readHacking(systems.hacking, ptr);
				break;
			}
		}

		// Artillery specifically
		this->read(start, base, "artillery.list.start");
		this->read(end, base, "artillery.list.end");

		size_t size = (end - start) / this->ptrSize, idx = 0;
		if (systems.artillery.size() != size)
			systems.artillery.resize(size);

		for (auto i = start; i != end; i += this->ptrSize)
		{
			uintptr_t ptr = 0;
			this->read(ptr, i);
			this->readArtillery(systems.artillery[idx], ptr);
			++idx;
		}
	}

	void readHazards(HazardsState& hazards, uintptr_t base)
	{
		this->read(hazards.target, base, "environment.target");

		// Read hazard presence
		this->read(hazards.asteroids, base, "environment.asteroids");
		this->read(hazards.sun, base, "environment.sun");
		this->read(hazards.pulsar, base, "environment.pulsar");
		this->read(hazards.pds, base, "environment.pds");
		this->read(hazards.nebula, base, "environment.nebula");
		this->read(hazards.storm, base, "environment.nebula.storm");

		// Asteroid specific stuff
		this->read(hazards.asteroidSpawnRates[0].first, base, "environment.asteroids.spawn_rate[0].min");
		this->read(hazards.asteroidSpawnRates[0].second, base, "environment.asteroids.spawn_rate[0].max");
		this->read(hazards.asteroidSpawnRates[1].first, base, "environment.asteroids.spawn_rate[1].min");
		this->read(hazards.asteroidSpawnRates[1].second, base, "environment.asteroids.spawn_rate[1].max");
		this->read(hazards.asteroidSpawnRates[2].first, base, "environment.asteroids.spawn_rate[2].min");
		this->read(hazards.asteroidSpawnRates[2].second, base, "environment.asteroids.spawn_rate[2].max");
		this->read(hazards.asteroidStateLengths[0].first, base, "environment.asteroids.state_length[0].min");
		this->read(hazards.asteroidStateLengths[0].second, base, "environment.asteroids.state_length[0].max");
		this->read(hazards.asteroidStateLengths[1].first, base, "environment.asteroids.state_length[1].min");
		this->read(hazards.asteroidStateLengths[1].second, base, "environment.asteroids.state_length[1].max");
		this->read(hazards.asteroidStateLengths[2].first, base, "environment.asteroids.state_length[2].min");
		this->read(hazards.asteroidStateLengths[2].second, base, "environment.asteroids.state_length[2].max");

		this->read(hazards.shipCount, base, "environment.asteroids.ship_count");
		this->read(hazards.asteroidState, base, "environment.asteroids.state");
		this->read(hazards.asteroidSpace, base, "environment.asteroids.space");
		this->read(hazards.asteroidVolleyTimer, base, "environment.asteroids.timer.volley");
		this->read(hazards.asteroidTimer, base, "environment.asteroids.timer.shot");
		this->read(hazards.shieldInit, base, "environment.asteroids.shield_init");

		// Hazard timer
		this->read(hazards.timer.first, base, "environment.timer");
		this->read(hazards.timer.second, base, "environment.timer.max");
	}

	void readProjectile(ProjectileState& projectile, uintptr_t base)
	{

	}

	void readProjectiles(std::vector<ProjectileState>& projectiles, uintptr_t base)
	{

	}

	void readSpaceDrone(SpaceDroneState& drone, uintptr_t base)
	{

	}

	void readSpaceDrones(std::vector<SpaceDroneState>& drones, uintptr_t base)
	{

	}

	void readCrewMember(CrewState& crew, uintptr_t base)
	{
		this->read(crew.shipAffiliation, base, "crew.ship_affiliation");
		this->read(crew.position.x, base, "crew.position.x");
		this->read(crew.position.y, base, "crew.position.y");
		this->read(crew.roomId, base, "crew.position.room_id");
		this->read(crew.shipId, base, "crew.position.ship_id");
		this->read(crew.manning, base, "crew.manning");
		this->read(crew.repairing, base, "crew.repairing");
		this->read(crew.dead, base, "crew.dead");
		this->read(crew.health.first, base, "crew.health");
		this->read(crew.health.second, base, "crew.health.max");
		this->read(crew.pathStart.x, base, "crew.path.start.x");
		this->read(crew.pathStart.y, base, "crew.path.start.y");
		this->read(crew.pathDestination.x, base, "crew.path.destination.x");
		this->read(crew.pathDestination.y, base, "crew.path.destination.y");
		this->read(crew.destinationRoomId, base, "crew.path.destination.room_id");
		this->read(crew.destinationSlotId, base, "crew.path.destination.slot_id");
		this->read(crew.pathLength, base, "crew.path.length");
		this->read(crew.next.x, base, "crew.path.next.x");
		this->read(crew.next.y, base, "crew.path.next.y");
		this->read(crew.suffocating, base, "crew.suffocating");
		this->read(crew.fighting, base, "crew.fighting");
		this->read(crew.melee, base, "crew.melee");
		this->read(crew.species, base, "crew.species");
		this->read(crew.name, base, "crew.name");
		this->read(crew.mindControlled, base, "crew.mind.controlled");
		this->read(crew.healthBoost, base, "crew.mind.health_boost");
		this->read(crew.damageBoost, base, "crew.mind.damage_boost");
		this->read(crew.cloneDying, base, "crew.clone.dying");
		this->read(crew.cloning, base, "crew.clone.ready");
		this->read(crew.stun, base, "crew.stun");

		uintptr_t doorPtr = 0;
		this->read(doorPtr, base, "crew.last_door");
		crew.inDoor = doorPtr != 0;
	}

	void readCrew(uintptr_t base)
	{
		uintptr_t start = 0, end = 0;

		// Read what can be controlled by the player
		uintptr_t crewBoxBase = 0;
		this->read(crewBoxBase, base, "gui2");
		this->read(start, crewBoxBase, "gui.crewbox.list.start");
		this->read(end, crewBoxBase, "gui.crewbox.list.end");

		std::vector<uintptr_t> crewPtrs;
		size_t size = (end - start) / this->ptrSize;
		if (crewPtrs.size() != size)
			crewPtrs.resize(size);

		size_t idx = 0;
		for (uintptr_t i = start; i != end; i += this->ptrSize)
		{
			uintptr_t ptr = 0;
			this->read(ptr, i);
			this->read(ptr, ptr, "gui.crewbox.crew");

			crewPtrs[idx] = ptr;
			++idx;
		}

		// Then read the rest
		this->read(start, base, "global.crew.list.start");
		this->read(end, base, "global.crew.list.end");
		this->read(this->state.playerCrewCount, base, "player.crew.count");
		this->read(this->state.targetCrewCount, base, "target.crew.count");

		size = (end - start) / this->ptrSize;
		if (this->state.crew.size() != size)
			this->state.crew.resize(size);

		idx = 0;
		for (uintptr_t i = start; i != end; i += this->ptrSize)
		{
			uintptr_t ptr = 0;
			this->read(ptr, i);

			this->readCrewMember(this->state.crew[idx], ptr);

			for (size_t i = 0; i < crewPtrs.size(); ++i)
				if (ptr == crewPtrs[i])
					this->state.crew[idx].guiSlot = int(i);

			++idx;
		}
	}

	void readRoom(RoomState& room, uintptr_t base)
	{
		if (this->state.justLoaded)
		{
			this->read(room.position.x, base, "room.x");
			this->read(room.position.y, base, "room.y");
			this->read(room.size.x, base, "room.width");
			this->read(room.size.y, base, "room.height");
			this->read(room.roomId, base, "room.id");

			room.size.x /= RoomTileLength;
			room.size.y /= RoomTileLength;
		}

		this->read(room.computer, base, "room.computer");
		this->read(room.hackLevel, base, "room.hacked");
	}

	void readRooms(std::unordered_map<int, RoomState>& rooms, uintptr_t base)
	{
		uintptr_t start = 0, end = 0;

		this->read(start, base, "room.list.start");
		this->read(end, base, "room.list.end");

		if (this->state.justLoaded)
			rooms.clear();

		for (uintptr_t i = start; i != end; i += this->ptrSize)
		{
			uintptr_t ptr = 0;
			this->read(ptr, i);

			int id = -1;
			this->read(id, ptr, "room.id");

			if (!rooms.count(id))
				rooms[id] = { .roomId = id };

			this->readRoom(rooms[id], ptr);
		}
	}

	void readDoor(DoorState& door, uintptr_t base)
	{
		if (this->state.justLoaded)
		{
			this->read(door.roomA, base, "door.room.a");
			this->read(door.roomB, base, "door.room.b");
			this->read(door.position.x, base, "door.position.x");
			this->read(door.position.y, base, "door.position.y");
			this->read(door.vertical, base, "door.vertical");
		}

		this->read(door.open, base, "door.open");
		this->read(door.baseHealth, base, "door.health.base");
		this->read(door.health, base, "door.health");
		this->read(door.level, base, "door.level");
		this->read(door.ioned, base, "door.ioned");
		this->read(door.hackLevel, base, "door.hacked");
	}

	void readDoors(std::unordered_map<int, DoorState>& doors, uintptr_t base)
	{
		uintptr_t start = 0, end = 0, start2 = 0, end2 = 0;

		this->read(start, base, "door.list.start");
		this->read(end, base, "door.list.end");
		this->read(start2, base, "door.airlock.list.start");
		this->read(end2, base, "door.airlock.list.end");

		if (this->state.justLoaded)
		{
			doors.clear();
		}

		for (uintptr_t i = start; i != end; i += this->ptrSize)
		{
			uintptr_t ptr = 0;
			this->read(ptr, i);

			// -1 is a valid door id, so we use this instead
			int id = 0xFFFF;
			this->read(id, ptr, "door.id");

			if(!doors.count(id))
				doors[id] = {.doorId = id, .airlock = false};

			this->readDoor(doors[id], ptr);
		}

		// Airlock doors have an id of -1
		// We need to differentiate between them,
		// so let's assign them their own ids
		int airlockId = -1;
		for (uintptr_t i = start2; i != end2; i += this->ptrSize)
		{
			uintptr_t ptr = 0;
			this->read(ptr, i);

			if (!doors.count(airlockId))
				doors[airlockId] = { .doorId = airlockId, .airlock = true };

			this->readDoor(doors[airlockId], ptr);

			--airlockId;
		}
	}
};


StateReader::StateReader(const AddressData& addresses)
	: addresses(addresses)
	, impl(new Impl{ *this })
{
}

StateReader::~StateReader()
{

}

const State& StateReader::getState() const
{
	return this->state;
}

const raw::State& StateReader::getRawState() const
{
	return this->rawState;
}

const KnownBlueprints& StateReader::getBlueprints() const
{
	return this->blueprints;
}

void StateReader::poll()
{
	this->impl->poll();
}
