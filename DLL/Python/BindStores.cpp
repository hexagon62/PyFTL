#include "Bind.hpp"
#include "../State/Event.hpp"

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
		.def_readonly("item", &StoreBox::item, "The main item contained")
		.def_readonly("extra", &StoreBox::extra, "The extra item (if applicable; ex: free Drone with Drone system)")
		;

	py::class_<Store>(module, "Store", "A store")
		.def_readonly_static("HARDCODED_BOXES_PER_SECTION", &Store::HARDCODED_BOXES_PER_SECTION, "Store boxes per store section")
		.def_readonly_static("HARDCODED_SECTIONS_PER_PAGE", &Store::HARDCODED_SECTIONS_PER_PAGE, "Store sections per store page")
		.def_readonly("boxes", &Store::boxes, "Number of boxes")
		.def_readonly("sections", &Store::sections, "Number of sections")
		.def_readonly("confirming", &Store::confirming, "Which box the confirm dialog is asking about")
		.def_readonly("fuel", &Store::fuel, "Fuel in stock")
		.def_readonly("fuel_cost", &Store::fuelCost, "Fuel price")
		.def_readonly("missiles", &Store::missiles, "Missiles in stock")
		.def_readonly("missile_cost", &Store::missileCost, "Missile price")
		.def_readonly("drone_parts", &Store::droneParts, "Drone parts in stock")
		.def_readonly("drone_part_cost", &Store::dronePartCost, "Drone part price")
		.def_readonly("repair_cost", &Store::repairCost, "Repair price")
		.def_readonly("repair_cost_full", &Store::repairCostFull, "Price to repair all damage")
		;
}

}
