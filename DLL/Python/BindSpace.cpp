#include "Bind.hpp"
#include "../State/Data/Space.hpp"

namespace python_bindings
{

void bindSpace(py::module_& module)
{
	py::class_<AsteroidInfo>(module, "AsteroidInfo", "Info for generating asteroids")
		.def_readonly("spawn_rates", &AsteroidInfo::spawnRates, "The spawn rate for each generator state")
		.def_readonly("state_lengths", &AsteroidInfo::stateLengths, "The length of each generate state")
		.def_readonly("ship_count", &AsteroidInfo::shipCount, "The number of ships at this location")
		.def_readonly("state", &AsteroidInfo::state, "The current generator state")
		.def_readonly("player_space", &AsteroidInfo::playerSpace, "If the asteroid will be in the player space")
		.def_readonly("next_direction", &AsteroidInfo::nextDirection, "Next asteroid direction")
		.def_readonly("state_timer", &AsteroidInfo::stateTimer, "Timer for next state change")
		.def_readonly("timer", &AsteroidInfo::timer, "Timer for next asteroid")
		.def_readonly("running", &AsteroidInfo::running, "If currently spawning asteroids")
		.def_readonly("shield_level", &AsteroidInfo::shieldLevel, "Shield level used to determine spawn rates")
		;

	py::enum_<EnvironmentType>(module, "EnvironmentType", "Type of environment at a location")
		.value("Invalid", EnvironmentType::Invalid)
		.value("Normal", EnvironmentType::Normal)
		.value("Asteroids", EnvironmentType::Asteroids)
		.value("CloseToSun", EnvironmentType::CloseToSun)
		.value("Nebula", EnvironmentType::Nebula)
		.value("IonStorm", EnvironmentType::IonStorm)
		.value("Pulsar", EnvironmentType::Pulsar)
		.value("ASB", EnvironmentType::ASB)
		;

	py::class_<Space>(module, "Space", "Info about what's in space")
		.def_readonly("projectiles", &Space::projectiles, "Projectile list")
		.def_readonly("environment", &Space::environment, "The environment")
		.def_readonly("asteroids", &Space::asteroids, "Asteroid generation (if in asteroid environment)")
		.def_readonly("hazard_timer", &Space::hazardTimer, "Timer until hazard activates (if applicable)")
		;
}

}
