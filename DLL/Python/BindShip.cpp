#include "Bind.hpp"
#include "../State/Ship.hpp"

namespace python_bindings
{

void bindShip(py::module_& module)
{
	py::class_<Cargo>(module, "Cargo", "A ship's inventory")
		.def_readonly("scrap", &Cargo::scrap, "The ship's scrap")
		.def_readonly("fuel", &Cargo::fuel, "The ship's fuel count")
		.def_readonly("missiles", &Cargo::missiles, "The ship's missile count")
		.def_readonly("drone_parts", &Cargo::droneParts, "The ship's drone parts")
		.def_readonly("weapons", &Cargo::weapons, "The ship's weapons in storage")
		.def_readonly("drones", &Cargo::drones, "The ship's drones in storage")
		.def_readonly("augments", &Cargo::augments, "The ship's augments")
		.def_readonly("over_capacity", &Cargo::overCapacity, "Items that will be left behind on jump")
		;

	py::class_<Reactor>(module, "Reactor", "A ship's reactor")
		.def_readonly("total", &Reactor::total, "Total power left in the reactor")
		.def_readonly("normal", &Reactor::normal, "Normal power left in the reactor")
		.def_readonly("battery", &Reactor::battery, "Battery power left in the reactor")
		.def_readonly("cap", &Reactor::cap, "The reactor's power cap")
		;

	py::class_<Ship>(module, "Ship", "A ship")
		.def_readonly("player", &Ship::player, "If it's a player ship")
		.def_readonly("destroyed", &Ship::destroyed, "If the ship is destroyed")
		.def_readonly("automated", &Ship::automated, "If the ship is automated")
		.def_readonly("jumping", &Ship::jumping, "If the ship is doing an FTL jump")
		.def_readonly("can_jump", &Ship::canJump, "If the ship can do an FTL jump")
		.def_readonly("can_inventory", &Ship::canInventory, "If the ship can open its inventory")
		.def_readonly("jump_timer", &Ship::jumpTimer, "The time until an FTL jump is possible")
		.def_readonly("hull", &Ship::hull, "The ship's hull")
		.def_readonly("super_shields", &Ship::superShields, "The ship's super shield bubbles")
		.def_readonly("evasion", &Ship::evasion, "The ship's evasion")
		.def_readonly("total_oxygen", &Ship::totalOxygen, "The ship's total oxygen")
		.def_readonly("shields", &Ship::shields, "The ship's shield system")
		.def_readonly("engines", &Ship::engines, "The ship's engines system")
		.def_readonly("medbay", &Ship::medbay, "The ship's medbay system")
		.def_readonly("clonebay", &Ship::clonebay, "The ship's clonebay system")
		.def_readonly("oxygen", &Ship::oxygen, "The ship's oxygen system")
		.def_readonly("teleporter", &Ship::teleporter, "The ship's teleporter system")
		.def_readonly("cloaking", &Ship::cloaking, "The ship's cloaking system")
		.def_readonly("artillery", &Ship::artillery, "The list of the ship's artillery systems")
		.def_readonly("mind_control", &Ship::mindControl, "The ship's mind control system")
		.def_readonly("hacking", &Ship::hacking, "The ship's hacking system")
		.def_readonly("weapons", &Ship::weapons, "The ship's weapons system")
		.def_readonly("drones", &Ship::drones, "The ship's drones system")
		.def_readonly("piloting", &Ship::piloting, "The ship's piloting system")
		.def_readonly("sensors", &Ship::sensors, "The ship's sensors system")
		.def_readonly("door_control", &Ship::doorControl, "The ship's door system")
		.def_readonly("battery", &Ship::battery, "The ship's backup battery system")
		.def_readonly("rooms", &Ship::rooms, "The ship's rooms")
		.def_readonly("doors", &Ship::doors, "The ship's doors")
		.def_readonly("cargo", &Ship::cargo, "The ship's inventory")
		.def("has_system", &Ship::hasSystem, "Checks if the specified system is present")
		.def("get_system", &Ship::getSystem, py::return_value_policy::reference, "Gets the specified system")
		;
}

}
