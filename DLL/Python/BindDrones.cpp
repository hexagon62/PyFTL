#include "Bind.hpp"
#include "../State/Drone.hpp"

namespace python_bindings
{

void bindDrones(py::module_& module)
{
	py::enum_<DroneType>(module, "DroneType", "The type of a drone")
		.value("Invalid", DroneType::Invalid)
		.value("Combat", DroneType::Combat)
		.value("Defense", DroneType::Defense)
		.value("AntiBoarder", DroneType::AntiBoarder)
		.value("Boarder", DroneType::Boarder)
		.value("HullRepair", DroneType::HullRepair)
		.value("SystemRepair", DroneType::SystemRepair)
		.value("Hacking", DroneType::Hacking)
		.value("Shield", DroneType::Shield)
		;

	py::class_<SpaceDroneMovementExtra>(module, "SpaceDroneMovementExtra", "Info for combat/hull repair drone movement")
		.def_readonly("destination_last", &SpaceDroneMovementExtra::destinationLast, "The last destination spot for the drone")
		.def_readonly("progress", &SpaceDroneMovementExtra::progress, "Progress towards the current destination")
		.def_readonly("heading", &SpaceDroneMovementExtra::heading, "The current heading of the drone")
		.def_readonly("heading_last", &SpaceDroneMovementExtra::headingLast, "The last heading of the drone")
		;

	py::class_<SpaceDroneInfo>(module, "SpaceDroneInfo", "Info for a drone that's in space")
		.def_readonly("player_space", &SpaceDroneInfo::playerSpace, "If the drone is in the player space")
		.def_readonly("player_space_is_destination", &SpaceDroneInfo::playerSpaceIsDestination, "If the drone is headed to the player space")
		.def_readonly("moving", &SpaceDroneInfo::moving, "If the drone is moving")
		.def_readonly("position", &SpaceDroneInfo::position, "The drone's position")
		.def_readonly("position_last", &SpaceDroneInfo::positionLast, "The drone's last position")
		.def_readonly("destination", &SpaceDroneInfo::destination, "The drone's destination position")
		.def_readonly("speed", &SpaceDroneInfo::speed, "The drone's speed")
		.def_readonly("pause", &SpaceDroneInfo::pause, "How long the drone has not been moving while active")
		.def_readonly("cooldown", &SpaceDroneInfo::cooldown, "The time the drone must wait to fire again")
		.def_readonly("angle", &SpaceDroneInfo::angle, "The drone's angle")
		.def_readonly("angle_desired", &SpaceDroneInfo::angleDesired, "The drone's desired angle")
		.def_readonly("angle_malfunction", &SpaceDroneInfo::angleMalfunction, "The angle the drone has if hacked/ioned")
		.def_readonly("ion_time", &SpaceDroneInfo::ionTime, "The time remaining the drone will be ioned for")
		.def_readonly("weapon", &SpaceDroneInfo::weapon, "The drone's weapon blueprint")
		.def_readonly("extra_movement", &SpaceDroneInfo::extraMovement, "Combat/hull repair drone movement info")
		;

	py::class_<Drone>(module, "Drone", "A drone")
		.def_readonly_static("HARDCODED_REBUILD_TIME", &Drone::HARDCODED_REBUILD_TIME, "How long you must wait to deploy a destroyed drone again")
		.def_readonly_static("HARDCODED_SAFE_MALFUNCTION_TIME", &Drone::HARDCODED_SAFE_MALFUNCTION_TIME, "How long before a drone can be destroyed by hacking/ion damage")
		.def_readonly("blueprint", &Drone::blueprint, "The drone's blueprint")
		.def_readonly("player", &Drone::player, "If the drone belongs to a player")
		.def_readonly("powered", &Drone::powered, "If the drone is powered")
		.def_readonly("dead", &Drone::dead, "If the drone is dead")
		.def_readonly("zoltan_power", &Drone::zoltanPower, "The amount of power from Zoltan crew")
		.def_readonly("hack_level", &Drone::hackLevel, "The hacking status")
		.def_readonly("hack_time", &Drone::hackTime, "The time hacked/ioned")
		.def_readonly("destroy_timer", &Drone::destroyTimer, "The amount of time left after destruction before the drone can be redeployed")
		.def_readonly("space", &Drone::space, "Info for if the drone is in space")
		;

	py::class_<HackingDrone, Drone>(module, "HackingDrone", "A hacking drone")
		.def_readonly("start", &HackingDrone::start, "The drone's starting position")
		.def_readonly("goal", &HackingDrone::goal, "The drone's starting goal")
		.def_readonly("arrived", &HackingDrone::arrived, "If the drone has arrived")
		.def_readonly("set_up", &HackingDrone::setUp, "If the drone is set up")
		.def_readonly("room", &HackingDrone::room, "The drone's targeted room")
		;
}

}
