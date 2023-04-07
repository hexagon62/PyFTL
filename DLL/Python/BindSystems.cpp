#include "Bind.hpp"
#include "../State/Data/Systems.hpp"

namespace python_bindings
{

void bindSystems(py::module_& module)
{
	py::enum_<SystemType>(module, "SystemType", "The type of a system")
		.value("None", SystemType::None)
		.value("Shields", SystemType::Shields)
		.value("Engines", SystemType::Engines)
		.value("Oxygen", SystemType::Oxygen)
		.value("Weapons", SystemType::Weapons)
		.value("Drones", SystemType::Drones)
		.value("Medbay", SystemType::Medbay)
		.value("Piloting", SystemType::Piloting)
		.value("Sensors", SystemType::Sensors)
		.value("Doors", SystemType::Doors)
		.value("Teleporter", SystemType::Teleporter)
		.value("Cloaking", SystemType::Cloaking)
		.value("Artillery", SystemType::Artillery)
		.value("Battery", SystemType::battery)
		.value("Clonebay", SystemType::Clonebay)
		.value("MindControl", SystemType::MindControl)
		.value("Hacking", SystemType::Hacking)
		;

	py::class_<System>(module, "System", "Generic system")
		.def_readonly("type", &System::type, "The type of the system")
		.def_readonly("blueprint", &System::blueprint, "The blueprint of the system")
		.def_readonly("room", &System::room, "The room the system is in")
		.def_readonly("power", &System::power, "The power state of the system")
		.def_readonly("health", &System::health, "The system's health")
		.def_readonly("level", &System::level, "The system's level")
		.def_readonly("manning_level", &System::manningLevel, "The level of whoever is manning the system; 0 if unmanned")
		.def_readonly("needs_manning", &System::needsManning, "If the system needs to be manned to function")
		.def_readonly("occupied", &System::occupied, "If the system's room is occupied, by a friendly crewmember")
		.def_readonly("on_fire", &System::onFire, "If the system is on fire")
		.def_readonly("breached", &System::breached, "If the system is breached")
		.def_readonly("boarders_attacking", &System::boardersAttacking, "If boarders are attacking the system")
		.def_readonly("damage_progress", &System::damageProgress, "The progress towards damaging 1 bar by boarders, fires, etc.")
		.def_readonly("repair_progress", &System::repairProgress, "The progress towards repairing 1 bar")
		;

	py::class_<ShieldSystem, System>(module, "ShieldSystem", "The shield system")
		.def_readonly("boundary", &ShieldSystem::boundary, "The ellipse defining the shield bubble")
		.def_readonly("bubbles", &ShieldSystem::bubbles, "The amount of bubbles the shields have up")
		.def_readonly("charge", &ShieldSystem::charge, "The progress towards charging the next bubble")
		;

	py::class_<EngineSystem, System>(module, "EngineSystem", "The engine system")
		.def_readonly_static("HARDCODED_EVASION_VALUES", &EngineSystem::HARDCODED_EVASION_VALUES, "The evasion value each level of engines provides")
		.def_readonly_static("HARDCODED_EVASION_SKILL_BOOST", &EngineSystem::HARDCODED_EVASION_SKILL_BOOST, "The evasion value each level of piloting skill provides")
		.def_readonly("boost_ftl", &EngineSystem::boostFTL, "If the engines are providing 3x FTL boost (usually if no fight is occuring and there's a hazard)")
		;

	py::class_<MedbaySystem, System>(module, "MedbaySystem", "The medbay system")
		.def_readonly("slot", &MedbaySystem::slot, "The slot that can't be occupied in the medbay, if applicable")
		;

	py::class_<ClonebaySystem, System>(module, "ClonebaySystem", "The clonebay system")
		.def_readonly_static("HARDCODED_DEATH_TIME", &ClonebaySystem::HARDCODED_DEATH_TIME, "The amount of time it takes a clone to die if the system is unpowered")
		.def_readonly("queue", &ClonebaySystem::queue, "The clone queue")
		.def_readonly("clone_timer", &ClonebaySystem::cloneTimer, "The time to create the next clone")
		.def_readonly("death_timer", &ClonebaySystem::deathTimer, "The time to kill the next clone")
		.def_readonly("slot", &ClonebaySystem::slot, "The slot that is taken by the clonebay, if applicable")
		;

	py::class_<OxygenSystem, System>(module, "OxygenSystem", "The oxygen system")
		;

	py::class_<TeleporterSystem, System>(module, "TeleporterSystem", "The teleporter system")
		.def_readonly("slots", &TeleporterSystem::slots, "The number of crew slots the teleporter has")
		.def_readonly("target_room", &TeleporterSystem::targetRoom, "The currently targeted room")
		.def_readonly("crew_present", &TeleporterSystem::crewPresent, "If crew are present in the teleporter room")
		.def_readonly("sending", &TeleporterSystem::sending, "If the teleporter is sending crew")
		.def_readonly("receiving", &TeleporterSystem::receiving, "If the teleporter is returning crew")
		;

	py::class_<CloakingSystem, System>(module, "CloakingSystem", "The cloaking system")
		.def_readonly_static("HARDCODED_EVASION_BONUS", &CloakingSystem::HARDCODED_EVASION_BONUS, "The evasion bonus provided by activated cloaking")
		.def_readonly("on", &CloakingSystem::on, "If cloaking is activated")
		.def_readonly("timer", &CloakingSystem::timer, "The time left before cloaking deactivates")
		;

	py::class_<ArtillerySystem, System>(module, "ArtillerySystem", "An artillery system")
		.def_readonly("weapon", &ArtillerySystem::weapon, "The weapon tied to the artillery system")
		;

	py::class_<MindControlSystem, System>(module, "MindControlSystem", "The mind control system")
		.def_readonly("on", &MindControlSystem::on, "If mind control is activated")
		.def_readonly("timer", &MindControlSystem::timer, "The time left before mind control deactivates")
		.def_readonly("target_room", &MindControlSystem::targetRoom, "The targeted room")
		.def_readonly("targeting_player_ship", &MindControlSystem::targetingPlayerShip, "If targeting the player ship")
		;

	py::enum_<HackLevel>(module, "HackLevel", "Used to store the state of something being hacked")
		.value("Invalid", HackLevel::Invalid, "An invalid state")
		.value("None", HackLevel::None, "Hacking drone is depowered or not attached")
		.value("Passive", HackLevel::Passive, "Hacking drone is attached, but hacking is not activated")
		.value("Active", HackLevel::Active, "Hacking drone is attached and hacking is activated")
		;

	py::class_<HackingSystem, System>(module, "HackingSystem", "The hacking system")
		.def_readonly("on", &HackingSystem::on, "If hacking is activated")
		.def_readonly("timer", &HackingSystem::timer, "The time left before hacking deactivates")
		.def_readonly("target", &HackingSystem::target, "The targeted system")
		.def_readonly("queued", &HackingSystem::queued, "The system that is selected for targeting")
		.def_readonly("drone", &HackingSystem::drone, "The hacking drone")
		;

	py::class_<WeaponSystem, System>(module, "WeaponSystem", "The weapon system")
		.def_readonly("slot_count", &WeaponSystem::slotCount, "The number of slots the system has")
		.def_readonly("weapons", &WeaponSystem::weapons, "The weapons installed")
		.def_readonly("user_powered", &WeaponSystem::userPowered, "Which weapons are powered by the user")
		.def_readonly("repower", &WeaponSystem::repower, "Which weapons will need repowering upon repair/deionization")
		.def_readonly("auto_fire", &WeaponSystem::autoFire, "If auto-fire is toggled to on (player only)")
		;

	py::class_<DroneSystem, System>(module, "DroneSystem")
		.def_readonly("slot_count", &DroneSystem::slotCount, "The number of slots the system has")
		.def_readonly("drones", &DroneSystem::drones, "The drones installed")
		.def_readonly("user_powered", &DroneSystem::userPowered, "Which drones are powered by the user")
		.def_readonly("repower", &DroneSystem::repower, "Which drones will need repowering upon repair/deionization")
		;

	py::class_<PilotingSystem, System>(module, "PilotingSystem", "The piloting system")
		.def_readonly_static("HARDCODED_EVASION_SKILL_BOOST", &PilotingSystem::HARDCODED_EVASION_SKILL_BOOST, "The evasion provided by piloting skills")
		;

	py::class_<SensorSystem, System>(module, "SensorSystem", "The sensor system")
		;

	py::class_<DoorSystem, System>(module, "DoorSystem", "The door system")
		;

	py::class_<BatterySystem, System>(module, "BatterySystem", "The backup battery system")
		.def_readonly("on", &BatterySystem::on, "If the battery is on")
		.def_readonly("timer", &BatterySystem::timer, "The time left before the battery deactivates")
		.def_readonly("provides", &BatterySystem::provides, "The power the battery can provide")
		.def_readonly("providing", &BatterySystem::providing, "The power the battery is currently providing")
		;

	py::class_<Power>(module, "Power", "The power state of a system")
		.def_readonly("total", &Power::total, "Total power")
		.def_readonly("normal", &Power::normal, "Power from reactor")
		.def_readonly("zoltan", &Power::zoltan, "Power from Zoltan crew")
		.def_readonly("battery", &Power::battery, "Power from Backup battery")
		.def_readonly("cap", &Power::cap, "Power cap from event/environment")
		.def_readonly("ion_level", &Power::ionLevel, "The level of ionization")
		.def_readonly("ion_timer", &Power::ionTimer, "The time left to remove one level of ionization")
		.def_readonly("restore_to", &Power::restoreTo, "The power level to restore the system to once repaired/ionization wears off")
		;
}

}
