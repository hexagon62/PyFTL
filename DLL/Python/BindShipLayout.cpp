#include "Bind.hpp"
#include "../State/Ship.hpp"

namespace python_bindings
{

void bindShipLayout(py::module_& module)
{
	py::class_<Repairable>(module, "Repairable", "A repairable object")
		.def_readonly("repair_progress", &Repairable::repairProgress, "Progress towards repairing it")
		.def_readonly("position", &Repairable::position, "The position of the object")
		.def_readonly("room", &Repairable::room, "The room it's in")
		.def_readonly("slot", &Repairable::slot, "The slot it's in")
		;

	py::class_<Fire, Repairable>(module, "Fire", "A fire")
		.def_readonly("death_timer", &Fire::deathTimer, "The time left to live for the fire")
		;

	py::class_<Breach, Repairable>(module, "Breach", "A breach")
		;

	py::class_<Slot>(module, "Slot", "A slot in a room")
		.def_readonly("id", &Slot::id, "The id of the slot")
		.def_readonly("rect", &Slot::rect, "The hitbox of the slot")
		.def_readonly("position", &Slot::position, "The position of the slot in the room")
		.def_readonly("occupiable", &Slot::occupiable, "If the slot can be occupied normally")
		.def_readonly("player", &Slot::player, "If the slot belongs to the player")
		.def_readonly("room", &Slot::room, "The room this slot belongs to")
		.def_readonly("crew", &Slot::crew, "The crewmember stationed at this slot")
		.def_readonly("intruder", &Slot::intruder, "The intruder stationed at this slot")
		.def_readonly("fire", &Slot::fire, "The fire at this slot")
		.def_readonly("breach", &Slot::breach, "The breach at this slot")
		;

	py::class_<Room>(module, "Room", "A room in the ship")
		.def_readonly_static("HARDCODED_TILE_SIZE", &Room::HARDCODED_TILE_SIZE, "The size of tiles (at 1280x720 resolution)")
		.def_readonly("system", &Room::system, "The system of the room")
		.def_readonly("id", &Room::id, "The id of the room")
		.def_readonly("primary_slot", &Room::primarySlot, "The primary slot of the room (where it'll be manned)")
		.def_readonly("primary_direction", &Room::primaryDirection, "The direction manning crewmembers will face")
		.def_readonly("rect", &Room::rect, "The hitbox of the room")
		.def_readonly("tiles", &Room::tiles, "The number of tiles in each direction")
		.def_readonly("player", &Room::player, "If the room is owned by the player")
		.def_readonly("visible", &Room::visible, "If the room can be seen inside of")
		.def_readonly("stunning", &Room::stunning, "If the room is stunning its inhabitants")
		.def_readonly("oxygen", &Room::oxygen, "The oxygen in the room")
		.def_readonly("hack_level", &Room::hackLevel, "The hacking level")
		.def_readonly("crew", &Room::crew, "Crew in this room")
		.def_readonly("intruders", &Room::intruders, "Intruders in this room")
		.def_readonly("fire_repair", &Room::fireRepair, "Progress of extinguishing fires (1 is equivalent to 1 fire at full health)")
		.def_readonly("breach_repair", &Room::breachRepair, "Progress of patching breaches (1 is equivalent to 1 breach at full health)")
		.def_readonly("slots_occupiable", &Room::slotsOccupiable, "Number of slots that can be occupied normally by crew")
		.def_readonly("slots", &Room::slots, "The number of slots the room has overall")
		.def("slot_at", py::overload_cast<const Point<int>&>(&Room::slotAt, py::const_), "Gets the slot at the specified point", py::return_value_policy::reference)
		.def("slot_id_at", &Room::slotIdAt, "Gets id of the slot at the specified point")
		.def("mind_controllable", &Room::mindControllable, py::arg("ignore_visibility") = false, "Checks if the room is a valid mind control target")
		;

	py::class_<Door>(module, "Door", "A door in the ship")
		.def_readonly_static("HARDCODED_HEALTH", &Door::HARDCODED_HEALTH, "The health the door has, indexed by difficulty, then door system level")
		.def_readonly("id", &Door::id, "The id of the door")
		.def_readonly("rooms", &Door::rooms, "The rooms the door is adjacent to")
		.def_readonly("level", &Door::level, "The level of the door")
		.def_readonly("health", &Door::health, "The health of the door")
		.def_readonly("hack_level", &Door::hackLevel, "The hacking level")
		.def_readonly("open", &Door::open, "If the door is open")
		.def_readonly("open_fake", &Door::openFake, "If the door is forced open")
		.def_readonly("ioned", &Door::ioned, "If the door is ioned")
		.def_readonly("vertical", &Door::vertical, "If the door is vertical")
		.def_readonly("airlock", &Door::airlock, "If the door connects to space")
		.def_readonly("player", &Door::player, "If the door belongs to the player")
		.def_readonly("rect", &Door::rect, "The rectangle bounding the door")
		;
}

}
