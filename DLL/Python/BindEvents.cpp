#include "Bind.hpp"
#include "../State/State.hpp"

namespace python_bindings
{

void bindEvents(py::module_& module)
{
	py::class_<ResourceEvent>(module, "ResourceEvent", "Resources given by an event")
		.def_readonly("missiles", &ResourceEvent::missiles, "Missiles given")
		.def_readonly("fuel", &ResourceEvent::fuel, "Fuel given")
		.def_readonly("drone_parts", &ResourceEvent::droneParts, "Drone parts given")
		.def_readonly("scrap", &ResourceEvent::scrap, "Scrap given")
		.def_readonly("crew", &ResourceEvent::crew, "Crew added")
		.def_readonly("traitor", &ResourceEvent::traitor, "If there's a traitor")
		.def_readonly("cloneable", &ResourceEvent::cloneable, "If a lost crewmember is cloneable")
		.def_readonly("steal", &ResourceEvent::steal, "If event steals")
		.def_readonly("intruders", &ResourceEvent::intruders, "If event has intruders")
		.def_readonly("weapon", &ResourceEvent::weapon, "Weapon given")
		.def_readonly("drone", &ResourceEvent::augment, "Augment given")
		.def_readonly("crew_type", &ResourceEvent::crewType, "Type of crew spawned/given")
		.def_readonly("fleet_delay", &ResourceEvent::fleetDelay, "Amount fleet is delayed/accelerated")
		.def_readonly("hull_damage", &ResourceEvent::hullDamage, "Amount of hull damage/repair")
		.def_readonly("system", &ResourceEvent::system, "System targeted (?)")
		.def_readonly("upgrade_amount", &ResourceEvent::upgradeAmount, "System upgrade amount")
		.def_readonly("remove_augment", &ResourceEvent::removeAugment, "An item that is removed (afaik only augments)")
		;

	py::class_<BoardingEvent>(module, "BoardingEvent", "Boarders spawned by an event")
		.def_readonly("crew_type", &BoardingEvent::crewType, "Crew type")
		.def_readonly("min", &BoardingEvent::min, "Minimum")
		.def_readonly("max", &BoardingEvent::max, "Maximum")
		.def_readonly("amount", &BoardingEvent::amount, "Actual amount")
		.def_readonly("breach", &BoardingEvent::breach, "If a breach is spawned")
		;

	py::class_<EventDamage>(module, "EventDamage", "Damage/status effects from an event")
		.def_readonly("system", &EventDamage::system, "Target system")
		.def_readonly("amount", &EventDamage::amount, "Amount of damage")
		.def_readonly("effect", &EventDamage::effect, "Effect type")
		;

	py::class_<Choice>(module, "Choice", "A choice that can be made at an event")
		.def_property_readonly("event", [](const Choice& o) { return o.event.get(); }, "The event tied to the choice")
		.def_readonly("required_object", &Choice::requiredObject, "A required object to make this choice")
		.def_readonly("level_min", &Choice::levelMin, "Minimum level for this choice (i.e. of systems)")
		.def_readonly("level_max", &Choice::levelMax, "Maximum level for this choice (i.e. of systems)")
		.def_readonly("max_group", &Choice::maxGroup, "Maximum amount of object?")
		.def_readonly("blue", &Choice::blue, "If the choice is a blue option")
		.def_readonly("hidden_reward", &Choice::hiddenReward, "If the choice has hidden rewards")
		;

	py::class_<LocationEvent>(module, "LocationEvent", "Location data for an event")
		.def_readonly("environment", &LocationEvent::environment, "The environment of the location")
		.def_readonly("environment_targets_enemy", &LocationEvent::environmentTargetsEnemy, "If the environment targets the enemy (i.e. ABS)")
		.def_readonly("exit", &LocationEvent::exit, "If at an exit beacon")
		.def_readonly("distress", &LocationEvent::distress, "If at a distress beacon")
		.def_readonly("reveal_map", &LocationEvent::revealMap, "If the map is revealed")
		.def_readonly("repair", &LocationEvent::repair, "If at a repair beacon")
		.def_readonly("unlock_ship", &LocationEvent::unlockShip, "If event unlocks ship")
		.def_readonly("ship", &LocationEvent::ship, "The ship at the event (if applicable)")
		.def_readonly("resources", &LocationEvent::resources, "The resources given at the event")
		.def_readonly("reward", &LocationEvent::reward, "The resources given by defeating a ship at the event")
		.def_readonly("boarders", &LocationEvent::boarders, "The boarders added (if applicable)")
		.def_readonly("store", &LocationEvent::store, "The store (if applicable)")
		.def_readonly("damage", &LocationEvent::damage, "The damage given by the event")
		.def_readonly("choices", &LocationEvent::choices, "The choices for the player")
		;
}

}
