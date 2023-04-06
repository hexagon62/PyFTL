#include "Bind.hpp"
#include "../State/State.hpp"

namespace python_bindings
{

void bindStores(py::module_& module)
{
	py::enum_<StoreBoxType>(module, "StoreBoxType", "Type of items at the store")
		.value("Invalid", StoreBoxType::Invalid)
		.value("Weapon", StoreBoxType::Weapon)
		.value("Drone", StoreBoxType::Drone)
		.value("Augment", StoreBoxType::Augment)
		.value("Crew", StoreBoxType::Crew)
		.value("System", StoreBoxType::System)
		.value("Item", StoreBoxType::Item)
		;

	py::class_<StoreBox>(module, "StoreBox", "A box with an item in it at the store")
		.def_readonly("type", &StoreBox::type, "Item type")
		.def_readonly("actual_price", &StoreBox::actualPrice, "The actual price, use this, not the blueprint")
		.def_readonly("id", &StoreBox::id, "The id of the box")
		.def_readonly("page2", &StoreBox::page2, "If the box is on page 2")
		.def_readonly("weapon", &StoreBox::weapon, "The weapon contained (if applicable)")
		.def_readonly("drone", &StoreBox::drone, "The drone contained (if applicable)")
		.def_readonly("augment", &StoreBox::augment, "The augment contained (if applicable)")
		.def_readonly("crew", &StoreBox::crew, "The crew contained (if applicable)")
		.def_readonly("system", &StoreBox::system, "The system contained (if applicable)")
		;

	py::class_<Store>(module, "Store", "A store")
		.def_readonly_static("HARDCODED_BOXES_PER_SECTION", &Store::HARDCODED_BOXES_PER_SECTION, "Store boxes per store section")
		.def_readonly("boxes", &Store::boxes, "Number of boxes")
		.def_readonly("sections", &Store::sections, "Number of sections")
		.def_readonly("fuel", &Store::fuel, "Fuel in stock")
		.def_readonly("fuel_cost", &Store::fuelCost, "Fuel price")
		.def_readonly("missiles", &Store::missiles, "Missiles in stock")
		.def_readonly("missile_cost", &Store::missileCost, "Missile price")
		.def_readonly("drone_parts", &Store::droneParts, "Drone parts in stock")
		.def_readonly("drone_part_cost", &Store::dronePartCost, "Drone part price")
		.def_readonly("repair_cost", &Store::repairCost, "Repair price")
		.def_readonly("repair_cost_full", &Store::repairCostFull, "Price to repair all damage")
		.def_readonly("page2", &Store::page2, "If player is looking at page 2")
		;
}

}
