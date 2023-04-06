#include "Bind.hpp"
#include "../State/State.hpp"

namespace python_bindings
{

void bindCrew(py::module_& module)
{
	py::class_<Path>(module, "Path", "A crewmember's path; the doors are the nodes.")
		.def_readonly("start", &Path::start, "The start of the path")
		.def_readonly("finish", &Path::finish, "The end of the path")
		.def_readonly("doors", &Path::doors, "The doors on the path")
		.def_readonly("distance", &Path::distance, "The length of the path")
		;

	py::class_<Crew>(module, "Crew", "A crewmember")
		.def_readonly("ui_box", &Crew::uiBox, "The crewmember's box in the UI")
		.def_readonly("selection_state", &Crew::selectionState, "If/how the crew is selected")
		.def_readonly("blueprint", &Crew::blueprint, "The crewmember's blueprint")
		.def_readonly("position", &Crew::position, "The crewmember's position")
		.def_readonly("goal", &Crew::goal, "The crewmember's destination position")
		.def_readonly("health", &Crew::health, "The crewmember's health")
		.def_readonly("speed", &Crew::speed, "The crewmember's speed")
		.def_readonly("path", &Crew::path, "The crewmember's current path")
		.def_readonly("player", &Crew::player, "If the crewmember belongs to the player")
		.def_readonly("on_player_ship", &Crew::onPlayerShip, "If the crewmember is on the player's ship")
		.def_readonly("new_path", &Crew::newPath, "If the crewmember has changed paths")
		.def_readonly("suffocating", &Crew::suffocating, "If the crewmember has insufficient O2")
		.def_readonly("repairing", &Crew::repairing, "If the crewmember is repairing")
		.def_readonly("intruder", &Crew::intruder, "If the crewmember is an intruder on the ship they're on")
		.def_readonly("fighting", &Crew::fighting, "If the crewmember is fighting")
		.def_readonly("dead", &Crew::dead, "If the crewmember is dead")
		.def_readonly("manning", &Crew::manning, "If the crewmember is manning a system")
		.def_readonly("moving", &Crew::moving, "If the crewmember is moving")
		.def_readonly("on_fire", &Crew::onFire, "If the crewmember is on fire")
		.def_readonly("room", &Crew::room, "The room the crewmember is in")
		.def_readonly("slot", &Crew::slot, "The slot the crewmember is in")
		.def_readonly("manned_system", &Crew::mannedSystem, "The system the crewmember is manning")
		.def_readonly("room_goal", &Crew::roomGoal, "The room the crewmember wants to be in")
		.def_readonly("slot_goal", &Crew::slotGoal, "The slot the crewmember wants to be in")
		.def_readonly("room_saved", &Crew::roomSaved, "The room the crewmember's station is in")
		.def_readonly("slot_saved", &Crew::slotSaved, "The slot of the crewmember's station")
		.def_readonly("clone_queue_position", &Crew::cloneQueuePosition, "The position of the crewmember in the clone queue")
		.def_readonly("clone_death_progress", &Crew::cloneDeathProgress, "The progress of the crewmember's death in the clone bay")
		.def_readonly("ready_to_clone", &Crew::readyToClone, "If the crewmember can be cloned")
		.def_readonly("mind_controlled", &Crew::mindControlled, "If the crewmember is mind controlled")
		.def_readonly("mind_control_health_boost", &Crew::mindControlHealthBoost, "The health boost from mind control")
		.def_readonly("mind_control_damage_multiplier", &Crew::mindControlDamageMultiplier, "The damage boost from mind control")
		.def_readonly("stun_time", &Crew::stunTime, "The amount of time the crewmember is stunned for")
		.def_readonly("teleport_timer", &Crew::teleportTimer, "The amount of time the crewmember is frozen thanks to teleporting")
		.def_readonly("leaving", &Crew::leaving, "If the crewmember is leaving their ship")
		.def_readonly("arriving", &Crew::arriving, "If the crewmember is arriving at their ship")
		.def_readonly("teleporting", &Crew::teleporting, "If the crewmember is teleporting")
		.def_readonly("drone", &Crew::drone, "If the crewmember is a drone")
		;
}

}
