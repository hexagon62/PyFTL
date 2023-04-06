#include "Bind.hpp"
#include "../State/Data/StarMap.hpp"

namespace python_bindings
{

void bindStarMap(py::module_& module)
{
	py::class_<Location>(module, "Location", "A location on the star map")
		.def_readonly("id", &Location::id, "The location's id")
		.def_readonly("position", &Location::position, "The location's position on the map")
		.def_readonly("neighbors", &Location::neighbors, "Locations accessible from this one")
		.def_readonly("visits", &Location::visits, "The number of times the player has visited this location")
		.def_readonly("known", &Location::known, "If the player knows what's at this beacon")
		.def_readonly("exit", &Location::exit, "If this location can be used to jump sectors")
		.def_readonly("hazard", &Location::hazard, "If this location would show a hazard on the map")
		.def_readonly("nebula", &Location::nebula, "If this location is in a nebula on the map")
		.def_readonly("flagship_present", &Location::flagshipPresent, "If this location has the Rebel Flagship at it")
		.def_readonly("quest", &Location::quest, "If this location has a quest at it")
		.def_readonly("fleet_overtaking", &Location::fleetOvertaking, "If this location will be overtaken by the fleet")
		.def_readonly("event", &Location::event, "The event at this location")
		;

	py::enum_<SectorType>(module, "SectorType", "Basic sector categories")
		.value("Invalid", SectorType::Invalid)
		.value("Civilian", SectorType::Civilian)
		.value("Hostile", SectorType::Hostile)
		.value("Nebula", SectorType::Nebula)
		;

	py::class_<Sector>(module, "Sector", "A sector")
		.def_readonly("id", &Sector::id, "The sector's id")
		.def_readonly("type", &Sector::type, "The type of the sector")
		.def_readonly("name", &Sector::name, "The internal name of the sector")
		.def_readonly("visited", &Sector::visited, "If the player has visited this sector")
		.def_readonly("reachable", &Sector::reachable, "If this sector is reachable on the map")
		.def_readonly("neighbors", &Sector::neighbors, "Sectors that can be accessed from this sector")
		.def_readonly("position", &Sector::position, "Position on the map")
		.def_readonly("level", &Sector::level, "The sector number (0-7 generally)")
		.def_readonly("unique", &Sector::unique, "If this particular sector type will only (usually) be generated once")
		;

	py::class_<StarMap>(module, "StarMap", "The star map is where beacon/sector selection occurs")
		.def_readonly("locations", &StarMap::locations, "The list of locations in the current sector")
		.def_readonly("last_stand", &StarMap::lastStand, "If the current sector is The Last Stand")
		.def_readonly("flagship_jumping", &StarMap::flagshipJumping, "If the flagship is jumping")
		.def_readonly("map_revealed", &StarMap::mapRevealed, "If the map is fully revealed")
		.def_readonly("secret_sector", &StarMap::secretSector, "If the sector is secret")
		.def_readonly("flagship_path", &StarMap::flagshipPath, "The path the flagship is taking")
		.def_readonly("translation", &StarMap::translation, "The offset in position of locations from the top left of the star map UI (tab included)")
		.def_readonly("danger_zone", &StarMap::dangerZone, "The zone currently taken by the rebel fleet")
		.def_readonly("pursuit_delay", &StarMap::pursuitDelay, "The number of jumps to delay the fleet for")
		.def_readonly("turns_left", &StarMap::turnsLeft, "Turns left until the Federation base is destroyed; -1 means the flagship hasn't arrived at it yet")
		.def_readonly("sectors", &StarMap::sectors, "The list of sectors")
		.def_readonly("current_location", &StarMap::currentLocation, "The current location; null when jumping sectors")
		.def_readonly("current_sector", &StarMap::currentSector, "The current sector; nullable, but probably not going to be null")
		.def_readonly("choosing_new_sector", &StarMap::choosingNewSector, "If picking a sector from the map, rather than a new location")
		.def_readonly("infinite_mode", &StarMap::infiniteMode, "If infinite mode is on")
		.def_readonly("nebula_sector", &StarMap::nebulaSector, "If the current sector is a nebula sector")
		.def_readonly("distress_beacon", &StarMap::distressBeacon, "If the player's distress beacon is on")
		.def_readonly("sector_number", &StarMap::sectorNumber, "The number of the current sector")
		;
}

}
