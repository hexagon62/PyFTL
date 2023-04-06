#include "Bind.hpp"
#include "../State/State.hpp"

namespace python_bindings
{

void bindHacking(py::module_& module)
{
	py::enum_<HackLevel>(module, "HackLevel", "Used to store the state of something being hacked")
		.value("Invalid", HackLevel::Invalid, "An invalid state")
		.value("None", HackLevel::None, "Hacking drone is depowered or not attached")
		.value("Passive", HackLevel::Passive, "Hacking drone is attached, but hacking is not activated")
		.value("Active", HackLevel::Active, "Hacking drone is attached and hacking is activated")
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
