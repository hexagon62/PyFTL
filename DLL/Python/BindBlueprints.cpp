#include "Bind.hpp"
#include "../State/Blueprints.hpp"

namespace python_bindings
{

void bindBlueprints(py::module_& module)
{
	py::class_<Blueprint>(module, "Blueprint", "A generic blueprint")
		.def_readonly("name", &Blueprint::name, "Internal name of the blueprint")
		.def_readonly("cost", &Blueprint::cost, "Cost of the item")
		.def_readonly("rarity", &Blueprint::rarity, "Rarity of the item")
		.def_readonly("base_rarity", &Blueprint::baseRarity, "Base rarity of the item")
		;

	py::class_<SystemBlueprint, Blueprint>(module, "SystemBlueprint", "Blueprint with general info on a system")
		.def_readonly("power_start", &SystemBlueprint::powerStart, "The amount of power the system starts with")
		.def_readonly("power_max", &SystemBlueprint::powerMax, "The maximum power the system can have")
		.def_readonly("upgrade_costs", &SystemBlueprint::upgradeCosts, "The cost to upgrade the system at each level")
		;

	py::class_<Augment, Blueprint>(module, "Augment", "An augmentation")
		.def_readonly("value", &Augment::value, "The value the game uses to determine the intensity of the effect")
		.def_readonly("slot", &Augment::slot, "The cargo slot the augment is stored in")
		.def_readonly("stacking", &Augment::stacking, "Whether or not having duplicates of the augmenet does anything")
		;

	py::class_<WeaponBlueprint, Blueprint>(module, "WeaponBlueprint", "A weapon blueprint")
		.def_readonly("damage", &WeaponBlueprint::damage, "The damage each projectile does")
		.def_readonly("type", &WeaponBlueprint::type, "The type of the weapon")
		.def_readonly("shots", &WeaponBlueprint::shots, "The number of shots the weapon fires")
		.def_readonly("missiles", &WeaponBlueprint::missiles, "The number of missiles used per shot")
		.def_readonly("cooldown", &WeaponBlueprint::cooldown, "The default time the weapon must wait to fire again")
		.def_readonly("power", &WeaponBlueprint::power, "The amount of power the weapon requires")
		.def_readonly("beam_length", &WeaponBlueprint::beamLength, "The length of the beam in pixels when the game is running at a resolution of 1280x720")
		.def_readonly("burst_radius", &WeaponBlueprint::burstRadius, "The radius of a burst attack (i.e. from flak weapons) when the game is running at a resolution of 1280x720")
		.def_readonly("charge_levels", &WeaponBlueprint::chargeLevels, "The amount of times the weapon can charge at once (ex: charge laser 2)")
		.def_readonly("boost", &WeaponBlueprint::boost, "The boost the weapon gets after it fires")
		.def_readonly("projectiles_fake", &WeaponBlueprint::projectilesFake, "How many fake projectiles the weapon fires in a shot")
		.def_readonly("projectiles", &WeaponBlueprint::projectiles, "The number of real projectiles the weapon fires in a shot")
		.def_readonly("projectiles_total", &WeaponBlueprint::projectilesTotal, "The number of projectiles the weapon fires in a volley, not considering multiple charges")
		;

	py::class_<DroneBlueprint, Blueprint>(module, "DroneBlueprint", "Blueprint for drones")
		.def_readonly_static("HARDCODED_SHIELD_COOLDOWNS", &DroneBlueprint::HARDCODED_SHIELD_COOLDOWNS, "The time the super shield drone takes to add a new bubble, given the amount of bubbles you already have")
		.def_readonly("type", &DroneBlueprint::type, "The type of the drone")
		.def_readonly("power", &DroneBlueprint::power, "The amount of power the drone requires")
		.def_readonly("cooldown", &DroneBlueprint::cooldown, "The amount of time the drone must wait before firing again")
		.def_readonly("speed", &DroneBlueprint::speed, "The speed at which the drone moves")
		;

	py::class_<CrewBlueprint, Blueprint>(module, "CrewBlueprint", "A crewmember's blueprint")
		.def_readonly("name", &CrewBlueprint::name, "The shortened name of the crewmember")
		.def_readonly("name_long", &CrewBlueprint::nameLong, "The full name of the crewmember")
		.def_readonly("species", &CrewBlueprint::species, "The species of the crewmember")
		.def_readonly("male", &CrewBlueprint::male, "If the crewmember is male")
		.def_readonly("skill_piloting", &CrewBlueprint::skillPiloting, "The crewmember's piloting skill")
		.def_readonly("skill_engines", &CrewBlueprint::skillEngines, "The crewmember's engine skill")
		.def_readonly("skill_shields", &CrewBlueprint::skillShields, "The crewmember's shields skill")
		.def_readonly("skill_weapons", &CrewBlueprint::skillWeapons, "The crewmember's weapons skill")
		.def_readonly("skill_repair", &CrewBlueprint::skillRepair, "The crewmember's repair skill")
		.def_readonly("skill_combat", &CrewBlueprint::skillCombat, "The crewmember's combat skill")
		;

	py::class_<Blueprints>(module, "Blueprints", "Blueprint data")
		.def_readonly("weapons", &Blueprints::weaponBlueprints, "Weapon blueprint dictionary")
		.def_readonly("drones", &Blueprints::droneBlueprints, "Drone blueprint dictionary")
		.def_readonly("augments", &Blueprints::augmentBlueprints, "Augment blueprint dictionary")
		.def_readonly("crew", &Blueprints::crewBlueprints, "Crew blueprint dictionary")
		.def_readonly("systems", &Blueprints::systemBlueprints, "System blueprint dictionary")
		;
}

}
